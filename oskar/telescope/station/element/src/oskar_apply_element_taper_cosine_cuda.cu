/*
 * Copyright (c) 2012-2018, The University of Oxford
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

#include "telescope/station/element/oskar_apply_element_taper_cosine_cuda.h"
#include <cuda_runtime.h>

/* Kernels. ================================================================ */

/* Single precision. */
__global__
void oskar_apply_element_taper_cosine_scalar_cudak_f(const int num_sources,
        const float cos_power, const float* theta, float2* jones)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_sources) return;
    const float f = powf(cosf(theta[i]), cos_power);
    jones[i].x *= f;
    jones[i].y *= f;
}

__global__
void oskar_apply_element_taper_cosine_matrix_cudak_f(const int num_sources,
        const float cos_power, const float* theta, float4c* jones)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_sources) return;
    const float f = powf(cosf(theta[i]), cos_power);
    jones[i].a.x *= f;
    jones[i].a.y *= f;
    jones[i].b.x *= f;
    jones[i].b.y *= f;
    jones[i].c.x *= f;
    jones[i].c.y *= f;
    jones[i].d.x *= f;
    jones[i].d.y *= f;
}

/* Double precision. */
__global__
void oskar_apply_element_taper_cosine_scalar_cudak_d(const int num_sources,
        const double cos_power, const double* theta, double2* jones)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_sources) return;
    const double f = pow(cos(theta[i]), cos_power);
    jones[i].x *= f;
    jones[i].y *= f;
}

__global__
void oskar_apply_element_taper_cosine_matrix_cudak_d(const int num_sources,
        const double cos_power, const double* theta, double4c* jones)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_sources) return;
    const double f = pow(cos(theta[i]), cos_power);
    jones[i].a.x *= f;
    jones[i].a.y *= f;
    jones[i].b.x *= f;
    jones[i].b.y *= f;
    jones[i].c.x *= f;
    jones[i].c.y *= f;
    jones[i].d.x *= f;
    jones[i].d.y *= f;
}

/* Kernel wrappers. ======================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void oskar_apply_element_taper_cosine_scalar_cuda_f(int num_sources,
        float cos_power, const float* d_theta, float2* d_jones)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num_sources + num_threads - 1) / num_threads;
    oskar_apply_element_taper_cosine_scalar_cudak_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (
            num_sources, cos_power, d_theta, d_jones);
}

void oskar_apply_element_taper_cosine_matrix_cuda_f(int num_sources,
        float cos_power, const float* d_theta, float4c* d_jones)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num_sources + num_threads - 1) / num_threads;
    oskar_apply_element_taper_cosine_matrix_cudak_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (
            num_sources, cos_power, d_theta, d_jones);
}

void oskar_apply_element_taper_cosine_scalar_cuda_d(int num_sources,
        double cos_power, const double* d_theta, double2* d_jones)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num_sources + num_threads - 1) / num_threads;
    oskar_apply_element_taper_cosine_scalar_cudak_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (
            num_sources, cos_power, d_theta, d_jones);
}

void oskar_apply_element_taper_cosine_matrix_cuda_d(int num_sources,
        double cos_power, const double* d_theta, double4c* d_jones)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num_sources + num_threads - 1) / num_threads;
    oskar_apply_element_taper_cosine_matrix_cudak_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (
            num_sources, cos_power, d_theta, d_jones);
}

#ifdef __cplusplus
}
#endif
