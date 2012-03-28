/*
 * Copyright (c) 2012, The University of Oxford
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

#ifndef OSKAR_CUDAF_DIERCKX_FPBSPL_BICUBIC_H_
#define OSKAR_CUDAF_DIERCKX_FPBSPL_BICUBIC_H_

/**
 * @file oskar_cudaf_dierckx_fpbspl_bicubic.h
 */

#include "oskar_global.h"

/**
 * @brief
 * CUDA device function for fpbspl from DIERCKX library (single precision).
 *
 * @details
 * CUDA device function to replace the fpbspl function from the DIERCKX
 * fitting library.
 *
 * This routine evaluates the (k+1) non-zero bicubic b-splines
 * at t(l) <= x < t(l+1) using the stable recurrence relation of
 * de Boor and Cox.
 */
__device__
void oskar_cudaf_dierckx_fpbspl_bicubic_f(const float *t,
        const float x, const int l, float *h)
{
    float f, hh[3];
    int i, j, li, lj;

    h[0] = 1.0f;
    for (j = 1; j <= 3; ++j)
    {
        for (i = 0; i < j; ++i)
        {
            hh[i] = h[i];
        }
        h[0] = 0.0f;
        for (i = 0; i < j; ++i)
        {
            li = l + i;
            lj = li - j;
            f = hh[i] / (t[li] - t[lj]);
            h[i] += f * (t[li] - x);
            h[i + 1] = f * (x - t[lj]);
        }
    }
}

/**
 * @brief
 * CUDA device function for fpbspl from DIERCKX library (double precision).
 *
 * @details
 * CUDA device function to replace the fpbspl function from the DIERCKX
 * fitting library.
 *
 * This routine evaluates the (k+1) non-zero bicubic b-splines
 * at t(l) <= x < t(l+1) using the stable recurrence relation of
 * de Boor and Cox.
 */
__device__
void oskar_cudaf_dierckx_fpbspl_bicubic_d(const double *t,
        const double x, const int l, double *h)
{
    double f, hh[3];
    int i, j, li, lj;

    h[0] = 1.0;
    for (j = 1; j <= 3; ++j)
    {
        for (i = 0; i < j; ++i)
        {
            hh[i] = h[i];
        }
        h[0] = 0.0;
        for (i = 0; i < j; ++i)
        {
            li = l + i;
            lj = li - j;
            f = hh[i] / (t[li] - t[lj]);
            h[i] += f * (t[li] - x);
            h[i + 1] = f * (x - t[lj]);
        }
    }
}

#endif // OSKAR_CUDAF_DIERCKX_FPBSPL_BICUBIC_H_