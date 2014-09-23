/*
 * Copyright (c) 2011-2014, The University of Oxford
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

#ifdef OSKAR_HAVE_CUDA
#include <cuda_runtime_api.h>
#endif

#include <private_mem.h>
#include <oskar_mem.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_mem_copy_contents(oskar_Mem* dst, const oskar_Mem* src,
        size_t offset_dst, size_t offset_src, size_t num_elements, int* status)
{
    int type_src, type_dst, location_src, location_dst;
    size_t n_elements_src, n_elements_dst, bytes, start_dst, start_src;
    void *destination, *source;

    /* Check all inputs. */
    if (!src || !dst || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Get the meta-data. */
    n_elements_src = src->num_elements;
    n_elements_dst = dst->num_elements;
    type_src = src->type;
    type_dst = dst->type;
    location_src = src->location;
    location_dst = dst->location;

    /* Check the data types. */
    if (type_src != type_dst)
    {
        *status = OSKAR_ERR_TYPE_MISMATCH;
        return;
    }

    /* Check that there are enough elements to copy. */
    if (num_elements > n_elements_src)
    {
        *status = OSKAR_ERR_OUT_OF_RANGE;
        return;
    }

    /* Check the data dimensions. */
    if (num_elements > (n_elements_dst - offset_dst))
    {
        *status = OSKAR_ERR_OUT_OF_RANGE;
        return;
    }

    /* Return immediately if there is nothing to copy. */
    if (src->data == NULL || n_elements_src == 0)
        return;

    /* Get the number of bytes to copy. */
    bytes       = oskar_mem_element_size(type_src) * num_elements;
    start_dst   = oskar_mem_element_size(type_src) * offset_dst;
    start_src   = oskar_mem_element_size(type_src) * offset_src;
    destination = (void*)((char*)(dst->data) + start_dst);
    source      = (void*)((char*)(src->data) + start_src);

    /* Host to host. */
    if (location_src == OSKAR_CPU && location_dst == OSKAR_CPU)
    {
        memcpy(destination, source, bytes);
        return;
    }

    /* Host to device. */
    else if (location_src == OSKAR_CPU && location_dst == OSKAR_GPU)
    {
#ifdef OSKAR_HAVE_CUDA
        cudaMemcpy(destination, source, bytes, cudaMemcpyHostToDevice);
        *status = cudaPeekAtLastError();
#else
        *status = OSKAR_ERR_CUDA_NOT_AVAILABLE;
#endif
        return;
    }

    /* Device to host. */
    else if (location_src == OSKAR_GPU && location_dst == OSKAR_CPU)
    {
#ifdef OSKAR_HAVE_CUDA
        cudaMemcpy(destination, source, bytes, cudaMemcpyDeviceToHost);
        *status = cudaPeekAtLastError();
#else
        *status = OSKAR_ERR_CUDA_NOT_AVAILABLE;
#endif
        return;
    }

    /* Device to device. */
    else if (location_src == OSKAR_GPU && location_dst == OSKAR_GPU)
    {
#ifdef OSKAR_HAVE_CUDA
        cudaMemcpy(destination, source, bytes, cudaMemcpyDeviceToDevice);
        *status = cudaPeekAtLastError();
#else
        *status = OSKAR_ERR_CUDA_NOT_AVAILABLE;
#endif
        return;
    }

    *status = OSKAR_ERR_BAD_LOCATION;
}

#ifdef __cplusplus
}
#endif