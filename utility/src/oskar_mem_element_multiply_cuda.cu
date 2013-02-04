/*
 * Copyright (c) 2013, The University of Oxford
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

#include "utility/oskar_cuda_check_error.h"
#include "utility/oskar_mem_copy.h"
#include "utility/oskar_mem_element_multiply_cuda.h"
#include "utility/oskar_mem_free.h"
#include "utility/oskar_mem_init.h"
#include "utility/oskar_vector_types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Single precision. */
__host__ __device__ __forceinline__
void oskar_complex_multiply_f(float2* c, const float2* a, const float2* b)
{
    c->x = a->x * b->x - a->y * b->y; /* RE*RE - IM*IM */
    c->y = a->y * b->x + a->x * b->y; /* IM*RE + RE*IM */
}

__global__
void oskar_element_multiply_cudak_rr_r_f(const int n, const float* a,
        const float* b, float* c)
{
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
        c[i] = a[i] * b[i];
}

__global__
void oskar_element_multiply_cudak_cc_c_f(const int n, const float2* a,
        const float2* b, float2* c)
{
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        float2 ac, bc, cc;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_f(&cc, &ac, &bc);
        c[i] = cc;
    }
}

__global__
void oskar_element_multiply_cudak_cc_m_f(const int n, const float2* a,
        const float2* b, float4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        float2 ac, bc, cc;
        float4c m;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_f(&cc, &ac, &bc);

        /* Store result in a matrix. */
        m.a = cc;
        m.b.x = 0.0f;
        m.b.y = 0.0f;
        m.c.x = 0.0f;
        m.c.y = 0.0f;
        m.d = cc;
        c[i] = m;
    }
}

__global__
void oskar_element_multiply_cudak_cm_m_f(const int n, const float2* a,
        const float4c* b, float4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        float2 ac;
        float4c bc, m;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_f(&m.a, &ac, &bc.a);
        oskar_complex_multiply_f(&m.b, &ac, &bc.b);
        oskar_complex_multiply_f(&m.c, &ac, &bc.c);
        oskar_complex_multiply_f(&m.d, &ac, &bc.d);
        c[i] = m;
    }
}

__global__
void oskar_element_multiply_cudak_mm_m_f(const int n, const float4c* a,
        const float4c* b, float4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        float4c ac, bc, m;
        float2 t;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_f(&m.a, &ac.a, &bc.a);
        oskar_complex_multiply_f(&t,   &ac.b, &bc.c);
        m.a.x += t.x;
        m.a.y += t.y;
        oskar_complex_multiply_f(&m.b, &ac.a, &bc.b);
        oskar_complex_multiply_f(&t,   &ac.b, &bc.d);
        m.b.x += t.x;
        m.b.y += t.y;
        oskar_complex_multiply_f(&m.c, &ac.c, &bc.a);
        oskar_complex_multiply_f(&t,   &ac.d, &bc.c);
        m.c.x += t.x;
        m.c.y += t.y;
        oskar_complex_multiply_f(&m.d, &ac.c, &bc.b);
        oskar_complex_multiply_f(&t,   &ac.d, &bc.d);
        m.d.x += t.x;
        m.d.y += t.y;
        c[i] = m;
    }
}

void oskar_mem_element_multiply_cuda_rr_r_f(int num, float* d_c,
        const float* d_a, const float* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_rr_r_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cc_c_f(int num, float2* d_c,
        const float2* d_a, const float2* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cc_c_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cc_m_f(int num, float4c* d_c,
        const float2* d_a, const float2* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cc_m_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cm_m_f(int num, float4c* d_c,
        const float2* d_a, const float4c* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cm_m_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_mm_m_f(int num, float4c* d_c,
        const float4c* d_a, const float4c* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_mm_m_f
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}



/* Double precision. */
__host__ __device__ __forceinline__
void oskar_complex_multiply_d(double2* c, const double2* a,
        const double2* b)
{
    c->x = a->x * b->x - a->y * b->y; /* RE*RE - IM*IM */
    c->y = a->y * b->x + a->x * b->y; /* IM*RE + RE*IM */
}

__global__
void oskar_element_multiply_cudak_rr_r_d(const int n, const double* a,
        const double* b, double* c)
{
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
        c[i] = a[i] * b[i];
}

__global__
void oskar_element_multiply_cudak_cc_c_d(const int n, const double2* a,
        const double2* b, double2* c)
{
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        double2 ac, bc, cc;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_d(&cc, &ac, &bc);
        c[i] = cc;
    }
}

__global__
void oskar_element_multiply_cudak_cc_m_d(const int n, const double2* a,
        const double2* b, double4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        double2 ac, bc, cc;
        double4c m;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_d(&cc, &ac, &bc);

        /* Store result in a matrix. */
        m.a = cc;
        m.b.x = 0.0;
        m.b.y = 0.0;
        m.c.x = 0.0;
        m.c.y = 0.0;
        m.d = cc;
        c[i] = m;
    }
}

__global__
void oskar_element_multiply_cudak_cm_m_d(const int n, const double2* a,
        const double4c* b, double4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        double2 ac;
        double4c bc, m;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_d(&m.a, &ac, &bc.a);
        oskar_complex_multiply_d(&m.b, &ac, &bc.b);
        oskar_complex_multiply_d(&m.c, &ac, &bc.c);
        oskar_complex_multiply_d(&m.d, &ac, &bc.d);
        c[i] = m;
    }
}

__global__
void oskar_element_multiply_cudak_mm_m_d(const int n, const double4c* a,
        const double4c* b, double4c* c)
{
    /* Get the array index ID that this thread is working on. */
    const int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        double4c ac, bc, m;
        double2 t;
        ac = a[i];
        bc = b[i];
        oskar_complex_multiply_d(&m.a, &ac.a, &bc.a);
        oskar_complex_multiply_d(&t,   &ac.b, &bc.c);
        m.a.x += t.x;
        m.a.y += t.y;
        oskar_complex_multiply_d(&m.b, &ac.a, &bc.b);
        oskar_complex_multiply_d(&t,   &ac.b, &bc.d);
        m.b.x += t.x;
        m.b.y += t.y;
        oskar_complex_multiply_d(&m.c, &ac.c, &bc.a);
        oskar_complex_multiply_d(&t,   &ac.d, &bc.c);
        m.c.x += t.x;
        m.c.y += t.y;
        oskar_complex_multiply_d(&m.d, &ac.c, &bc.b);
        oskar_complex_multiply_d(&t,   &ac.d, &bc.d);
        m.d.x += t.x;
        m.d.y += t.y;
        c[i] = m;
    }
}

void oskar_mem_element_multiply_cuda_rr_r_d(int num, double* d_c,
        const double* d_a, const double* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_rr_r_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cc_c_d(int num, double2* d_c,
        const double2* d_a, const double2* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cc_c_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cc_m_d(int num, double4c* d_c,
        const double2* d_a, const double2* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cc_m_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_cm_m_d(int num, double4c* d_c,
        const double2* d_a, const double4c* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_cm_m_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}

void oskar_mem_element_multiply_cuda_mm_m_d(int num, double4c* d_c,
        const double4c* d_a, const double4c* d_b)
{
    int num_blocks, num_threads = 256;
    num_blocks = (num + num_threads - 1) / num_threads;
    oskar_element_multiply_cudak_mm_m_d
    OSKAR_CUDAK_CONF(num_blocks, num_threads) (num, d_a, d_b, d_c);
}


static void oskar_mem_element_multiply_select_cuda(oskar_Mem* c,
        const oskar_Mem* a, const oskar_Mem* b, int num, int* status)
{
    int error = OSKAR_ERR_TYPE_MISMATCH; /* Set to type mismatch by default. */

    /* Check if safe to proceed. */
    if (*status) return;

    /* Multiply the elements. */
    if (a->type == OSKAR_DOUBLE)
    {
        if (b->type == OSKAR_DOUBLE)
        {
            if (c->type == OSKAR_DOUBLE)
            {
                /* Real, real to real. */
                error = 0;
                oskar_mem_element_multiply_cuda_rr_r_d(num, (double*)c->data,
                        (const double*)a->data, (const double*)b->data);
            }
        }
    }
    else if (a->type == OSKAR_DOUBLE_COMPLEX)
    {
        if (b->type == OSKAR_DOUBLE_COMPLEX)
        {
            if (c->type == OSKAR_DOUBLE_COMPLEX)
            {
                /* Complex scalar, complex scalar to complex scalar. */
                error = 0;
                oskar_mem_element_multiply_cuda_cc_c_d(num, (double2*)c->data,
                        (const double2*)a->data, (const double2*)b->data);
            }
            else if (c->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
            {
                /* Complex scalar, complex scalar to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cc_m_d(num, (double4c*)c->data,
                        (const double2*)a->data, (const double2*)b->data);
            }
        }
        else if (b->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
        {
            if (c->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
            {
                /* Complex scalar, complex matrix to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cm_m_d(num, (double4c*)c->data,
                        (const double2*)a->data, (const double4c*)b->data);
            }
        }
    }
    else if (a->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
    {
        if (b->type == OSKAR_DOUBLE_COMPLEX)
        {
            if (c->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
            {
                /* Complex matrix, complex scalar to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cm_m_d(num, (double4c*)c->data,
                        (const double2*)b->data, (const double4c*)a->data);
            }
        }
        else if (b->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
        {
            if (c->type == OSKAR_DOUBLE_COMPLEX_MATRIX)
            {
                /* Complex matrix, complex matrix to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_mm_m_d(num, (double4c*)c->data,
                        (const double4c*)a->data, (const double4c*)b->data);
            }
        }
    }
    else if (a->type == OSKAR_SINGLE)
    {
        if (b->type == OSKAR_SINGLE)
        {
            if (c->type == OSKAR_SINGLE)
            {
                /* Real, real to real. */
                error = 0;
                oskar_mem_element_multiply_cuda_rr_r_f(num, (float*)c->data,
                        (const float*)a->data, (const float*)b->data);
            }
        }
    }
    else if (a->type == OSKAR_SINGLE_COMPLEX)
    {
        if (b->type == OSKAR_SINGLE_COMPLEX)
        {
            if (c->type == OSKAR_SINGLE_COMPLEX)
            {
                /* Complex scalar, complex scalar to complex scalar. */
                error = 0;
                oskar_mem_element_multiply_cuda_cc_c_f(num, (float2*)c->data,
                        (const float2*)a->data, (const float2*)b->data);
            }
            else if (c->type == OSKAR_SINGLE_COMPLEX_MATRIX)
            {
                /* Complex scalar, complex scalar to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cc_m_f(num, (float4c*)c->data,
                        (const float2*)a->data, (const float2*)b->data);
            }
        }
        else if (b->type == OSKAR_SINGLE_COMPLEX_MATRIX)
        {
            if (c->type == OSKAR_SINGLE_COMPLEX_MATRIX)
            {
                /* Complex scalar, complex matrix to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cm_m_f(num, (float4c*)c->data,
                        (const float2*)a->data, (const float4c*)b->data);
            }
        }
    }
    else if (a->type == OSKAR_SINGLE_COMPLEX_MATRIX)
    {
        if (b->type == OSKAR_SINGLE_COMPLEX)
        {
            if (c->type == OSKAR_SINGLE_COMPLEX_MATRIX)
            {
                /* Complex matrix, complex scalar to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_cm_m_f(num, (float4c*)c->data,
                        (const float2*)b->data, (const float4c*)a->data);
            }
        }
        else if (b->type == OSKAR_SINGLE_COMPLEX_MATRIX)
        {
            if (c->type == OSKAR_SINGLE_COMPLEX_MATRIX)
            {
                /* Complex matrix, complex matrix to complex matrix. */
                error = 0;
                oskar_mem_element_multiply_cuda_mm_m_f(num, (float4c*)c->data,
                        (const float4c*)a->data, (const float4c*)b->data);
            }
        }
    }

    /* Check for type mismatch and CUDA error. */
    if (error) *status = error;
    oskar_cuda_check_error(status);
}


void oskar_mem_element_multiply_cuda(oskar_Mem* C, const oskar_Mem* A,
        const oskar_Mem* B, int num, int* status)
{
    oskar_Mem At, Bt;
    const oskar_Mem *Ap, *Bp;

    /* Check all inputs. */
    if (!C || !A || !B || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Set default pointer values. */
    Ap = A;
    Bp = B;

    /* Check that the output array is in GPU memory. */
    if (C->location != OSKAR_LOCATION_GPU)
    {
        *status = OSKAR_ERR_BAD_LOCATION;
        return;
    }

    /* Check memory is allocated. */
    if (!A->data || !B->data || !C->data)
    {
        *status = OSKAR_ERR_MEMORY_NOT_ALLOCATED;
        return;
    }

    /* Set the number of elements to multiply. */
    if (num <= 0) num = A->num_elements;

    /* Check that there are enough elements. */
    if (B->num_elements < num || C->num_elements < num)
    {
        *status = OSKAR_ERR_DIMENSION_MISMATCH;
        return;
    }

    /* Copy input data to temporary GPU memory if required. */
    oskar_mem_init(&At, A->type, OSKAR_LOCATION_GPU, 0, 1, status);
    oskar_mem_init(&Bt, B->type, OSKAR_LOCATION_GPU, 0, 1, status);
    if (A->location != OSKAR_LOCATION_GPU)
    {
        oskar_mem_copy(&At, A, status);
        Ap = &At;
    }
    if (B->location != OSKAR_LOCATION_GPU)
    {
        oskar_mem_copy(&Bt, B, status);
        Bp = &Bt;
    }

    /* Do the multiplication using CUDA. */
    oskar_mem_element_multiply_select_cuda(C, Ap, Bp, num, status);

    /* Free temporaries. */
    oskar_mem_free(&At, status);
    oskar_mem_free(&Bt, status);
}

#ifdef __cplusplus
}
#endif