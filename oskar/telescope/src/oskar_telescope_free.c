/*
 * Copyright (c) 2011-2015, The University of Oxford
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

#include "telescope/private_telescope.h"
#include "telescope/oskar_telescope.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_telescope_free(oskar_Telescope* telescope, int* status)
{
    int i = 0;
    if (!telescope) return;

    /* Free the arrays. */
    oskar_mem_free(telescope->station_true_x_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_true_y_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_true_z_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_true_x_enu_metres, status);
    oskar_mem_free(telescope->station_true_y_enu_metres, status);
    oskar_mem_free(telescope->station_true_z_enu_metres, status);
    oskar_mem_free(telescope->station_measured_x_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_measured_y_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_measured_z_offset_ecef_metres, status);
    oskar_mem_free(telescope->station_measured_x_enu_metres, status);
    oskar_mem_free(telescope->station_measured_y_enu_metres, status);
    oskar_mem_free(telescope->station_measured_z_enu_metres, status);

    /* Free each station. */
    for (i = 0; i < telescope->num_stations; ++i)
    {
        oskar_station_free(oskar_telescope_station(telescope, i), status);
    }

    /* Free the station array. */
    free(telescope->station);

    /* Free the structure itself. */
    free(telescope);
}

#ifdef __cplusplus
}
#endif
