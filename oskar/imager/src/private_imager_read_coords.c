/*
 * Copyright (c) 2017, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "imager/private_imager.h"
#include "imager/private_imager_read_coords.h"
#include "imager/oskar_imager.h"
#include "binary/oskar_binary.h"
#include "math/oskar_cmath.h"
#include "mem/oskar_binary_read_mem.h"
#include "ms/oskar_measurement_set.h"
#include "vis/oskar_vis_block.h"
#include "vis/oskar_vis_header.h"
#include "utility/oskar_timer.h"

#include <float.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_imager_read_coords_ms(oskar_Imager* h, const char* filename,
        int i_file, int num_files, int* percent_done, int* percent_next,
        int* status)
{
#ifndef OSKAR_NO_MS
    oskar_MeasurementSet* ms;
    oskar_Mem *uvw, *u, *v, *w, *weight, *time_centroid;
    int num_channels, num_stations, num_baselines, num_pols;
    int start_row, num_rows;
    double *uvw_, *u_, *v_, *w_;
    if (*status) return;

    /* Read the header. */
    ms = oskar_ms_open(filename);
    if (!ms)
    {
        *status = OSKAR_ERR_FILE_IO;
        return;
    }
    num_rows = (int) oskar_ms_num_rows(ms);
    num_stations = (int) oskar_ms_num_stations(ms);
    num_baselines = num_stations * (num_stations - 1) / 2;
    num_pols = (int) oskar_ms_num_pols(ms);
    num_channels = (int) oskar_ms_num_channels(ms);

    /* Set visibility meta-data. */
    oskar_imager_set_vis_frequency(h,
            oskar_ms_freq_start_hz(ms),
            oskar_ms_freq_inc_hz(ms), num_channels);
    oskar_imager_set_vis_phase_centre(h,
            oskar_ms_phase_centre_ra_rad(ms) * 180/M_PI,
            oskar_ms_phase_centre_dec_rad(ms) * 180/M_PI);

    /* Create arrays. */
    uvw = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, 3 * num_baselines, status);
    u = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, num_baselines, status);
    v = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, num_baselines, status);
    w = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, num_baselines, status);
    weight = oskar_mem_create(OSKAR_SINGLE, OSKAR_CPU,
            num_baselines * num_pols, status);
    time_centroid = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, num_baselines,
            status);
    uvw_ = oskar_mem_double(uvw, status);
    u_ = oskar_mem_double(u, status);
    v_ = oskar_mem_double(v, status);
    w_ = oskar_mem_double(w, status);

    /* Loop over visibility blocks. */
    for (start_row = 0; start_row < num_rows; start_row += num_baselines)
    {
        int i, block_size;
        size_t allocated, required;
        if (*status) break;

        /* Read coordinates and weights from Measurement Set. */
        oskar_timer_resume(h->tmr_read);
        block_size = num_rows - start_row;
        if (block_size > num_baselines) block_size = num_baselines;
        allocated = oskar_mem_length(uvw) *
                oskar_mem_element_size(oskar_mem_type(uvw));
        oskar_ms_read_column(ms, "UVW", start_row, block_size,
                allocated, oskar_mem_void(uvw), &required, status);
        allocated = oskar_mem_length(weight) *
                oskar_mem_element_size(oskar_mem_type(weight));
        oskar_ms_read_column(ms, "WEIGHT", start_row, block_size,
                allocated, oskar_mem_void(weight), &required, status);
        allocated = oskar_mem_length(time_centroid) *
                oskar_mem_element_size(oskar_mem_type(time_centroid));
        oskar_ms_read_column(ms, "TIME_CENTROID", start_row, block_size,
                allocated, oskar_mem_void(time_centroid), &required, status);

        /* Split up baseline coordinates. */
        for (i = 0; i < block_size; ++i)
        {
            u_[i] = uvw_[3*i + 0];
            v_[i] = uvw_[3*i + 1];
            w_[i] = uvw_[3*i + 2];
        }

        /* Update the imager with the data. */
        oskar_timer_pause(h->tmr_read);
        oskar_imager_update(h, block_size, 0, num_channels - 1, num_pols,
                u, v, w, 0, weight, time_centroid, status);
        *percent_done = (int) round(100.0 * (
                (start_row + block_size) / (double)(num_rows * num_files) +
                i_file / (double)num_files));
        if (h->log && percent_next && *percent_done >= *percent_next)
        {
            oskar_log_message(h->log, 'S', -2, "%3d%% ...", *percent_done);
            *percent_next = 10 + 10 * (*percent_done / 10);
        }
    }
    oskar_mem_free(uvw, status);
    oskar_mem_free(u, status);
    oskar_mem_free(v, status);
    oskar_mem_free(w, status);
    oskar_mem_free(weight, status);
    oskar_mem_free(time_centroid, status);
    oskar_ms_close(ms);
#else
    (void) filename;
    (void) i_file;
    (void) num_files;
    (void) percent_done;
    (void) percent_next;
    oskar_log_error(h->log, "OSKAR was compiled without Measurement Set support.");
    *status = OSKAR_ERR_FUNCTION_NOT_AVAILABLE;
#endif
}


void oskar_imager_read_coords_vis(oskar_Imager* h, const char* filename,
        int i_file, int num_files, int* percent_done, int* percent_next,
        int* status)
{
    oskar_Binary* vis_file;
    oskar_VisHeader* header;
    oskar_Mem *uu, *vv, *ww, *weight, *time_centroid, *time_slice;
    int coord_prec, max_times_per_block, tags_per_block, i_block, num_blocks;
    int num_times_total, num_stations, num_baselines, num_pols;
    double time_start_mjd, time_inc_sec;
    if (*status) return;

    /* Read the header. */
    vis_file = oskar_binary_create(filename, 'r', status);
    header = oskar_vis_header_read(vis_file, status);
    if (*status)
    {
        oskar_vis_header_free(header, status);
        oskar_binary_free(vis_file);
        return;
    }
    coord_prec = oskar_vis_header_coord_precision(header);
    max_times_per_block = oskar_vis_header_max_times_per_block(header);
    tags_per_block = oskar_vis_header_num_tags_per_block(header);
    num_times_total = oskar_vis_header_num_times_total(header);
    num_stations = oskar_vis_header_num_stations(header);
    num_baselines = num_stations * (num_stations - 1) / 2;
    num_pols = oskar_type_is_matrix(oskar_vis_header_amp_type(header)) ? 4 : 1;
    num_blocks = (num_times_total + max_times_per_block - 1) /
            max_times_per_block;
    time_start_mjd = oskar_vis_header_time_start_mjd_utc(header) * 86400.0;
    time_inc_sec = oskar_vis_header_time_inc_sec(header);

    /* Set visibility meta-data. */
    oskar_imager_set_vis_frequency(h,
            oskar_vis_header_freq_start_hz(header),
            oskar_vis_header_freq_inc_hz(header),
            oskar_vis_header_num_channels_total(header));
    oskar_imager_set_vis_phase_centre(h,
            oskar_vis_header_phase_centre_ra_deg(header),
            oskar_vis_header_phase_centre_dec_deg(header));

    /* Create scratch arrays. Weights are all 1. */
    uu = oskar_mem_create(coord_prec, OSKAR_CPU, 0, status);
    vv = oskar_mem_create(coord_prec, OSKAR_CPU, 0, status);
    ww = oskar_mem_create(coord_prec, OSKAR_CPU, 0, status);
    time_centroid = oskar_mem_create(OSKAR_DOUBLE,
            OSKAR_CPU, num_baselines * max_times_per_block, status);
    time_slice = oskar_mem_create_alias(0, 0, 0, status);
    weight = oskar_mem_create(h->imager_prec,
            OSKAR_CPU, num_baselines * num_pols * max_times_per_block, status);
    oskar_mem_set_value_real(weight, 1.0, 0, 0, status);

    /* Loop over visibility blocks. */
    for (i_block = 0; i_block < num_blocks; ++i_block)
    {
        int t, num_times, num_channels, start_time, start_chan, end_chan;
        int dim_start_and_size[6];
        size_t num_rows;
        if (*status) break;

        /* Read block metadata. */
        oskar_timer_resume(h->tmr_read);
        oskar_binary_set_query_search_start(vis_file,
                i_block * tags_per_block, status);
        oskar_binary_read(vis_file, OSKAR_INT,
                OSKAR_TAG_GROUP_VIS_BLOCK,
                OSKAR_VIS_BLOCK_TAG_DIM_START_AND_SIZE, i_block,
                sizeof(dim_start_and_size), dim_start_and_size, status);
        start_time   = dim_start_and_size[0];
        start_chan   = dim_start_and_size[1];
        num_times    = dim_start_and_size[2];
        num_channels = dim_start_and_size[3];
        num_rows     = num_times * num_baselines;
        end_chan     = start_chan + num_channels - 1;

        /* Fill in the time centroid values. */
        for (t = 0; t < num_times; ++t)
        {
            oskar_mem_set_alias(time_slice, time_centroid,
                    t * num_baselines, num_baselines, status);
            oskar_mem_set_value_real(time_slice,
                    time_start_mjd + (start_time + t + 0.5) * time_inc_sec,
                    0, num_baselines, status);
        }

        /* Read the baseline coordinates. */
        oskar_binary_read_mem(vis_file, uu, OSKAR_TAG_GROUP_VIS_BLOCK,
                OSKAR_VIS_BLOCK_TAG_BASELINE_UU, i_block, status);
        oskar_binary_read_mem(vis_file, vv, OSKAR_TAG_GROUP_VIS_BLOCK,
                OSKAR_VIS_BLOCK_TAG_BASELINE_VV, i_block, status);
        oskar_binary_read_mem(vis_file, ww, OSKAR_TAG_GROUP_VIS_BLOCK,
                OSKAR_VIS_BLOCK_TAG_BASELINE_WW, i_block, status);

        /* Update the imager with the data. */
        oskar_timer_pause(h->tmr_read);
        oskar_imager_update(h, num_rows, start_chan, end_chan, num_pols,
                uu, vv, ww, 0, weight, time_centroid, status);
        *percent_done = (int) round(100.0 * (
                (i_block + 1) / (double)(num_blocks * num_files) +
                i_file / (double)num_files));
        if (h->log && percent_next && *percent_done >= *percent_next)
        {
            oskar_log_message(h->log, 'S', -2, "%3d%% ...", *percent_done);
            *percent_next = 10 + 10 * (*percent_done / 10);
        }
    }
    oskar_mem_free(uu, status);
    oskar_mem_free(vv, status);
    oskar_mem_free(ww, status);
    oskar_mem_free(weight, status);
    oskar_mem_free(time_centroid, status);
    oskar_mem_free(time_slice, status);
    oskar_vis_header_free(header, status);
    oskar_binary_free(vis_file);
}


#ifdef __cplusplus
}
#endif
