/*
 * Copyright (c) 2011, The University of Oxford
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

#include "utility/oskar_BinaryHeader.h"
#include "utility/oskar_BinaryTag.h"
#include "utility/oskar_binary_stream_read_header.h"
#include "utility/oskar_binary_tag_index_create.h"
#include "utility/oskar_endian.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

int oskar_binary_tag_index_create(oskar_BinaryTagIndex** index, FILE* stream)
{
    oskar_BinaryHeader header;
    oskar_BinaryTagIndex* idx;
    int error, i;

    /* Allocate index. */
    idx = (oskar_BinaryTagIndex*) malloc(sizeof(oskar_BinaryTagIndex));
    *index = idx;

    /* Read header. */
    error = oskar_binary_stream_read_header(stream, &header);
    if (error) return error;

    /* Initialise tag index. */
    idx->num_tags = 0;
    idx->tag = 0;
    idx->user_index = 0;
    idx->block_size_bytes = 0;
    idx->data_size_bytes = 0;
    idx->data_offset_bytes = 0;
    idx->name_group = 0;
    idx->name_tag = 0;

    /* Read all tags in the stream. */
    for (i = 0; OSKAR_TRUE; ++i)
    {
        size_t memcpy_size = 0;

        /* Check if we need to allocate more storage for the tag. */
        if (i % 10 == 0)
        {
            int m;

            /* New size of arrays. */
            m = i + 10;
            idx->tag = (oskar_BinaryTag*) realloc(idx->tag,
                    m * sizeof(oskar_BinaryTag));
            idx->user_index = (int*) realloc(idx->user_index, m * sizeof(int));
            idx->block_size_bytes = (size_t*) realloc(idx->block_size_bytes,
                    m * sizeof(size_t));
            idx->data_size_bytes = (size_t*) realloc(idx->data_size_bytes,
                    m * sizeof(size_t));
            idx->data_offset_bytes = (long*) realloc(idx->data_offset_bytes,
                    m * sizeof(long));
            idx->name_group = (char**) realloc(idx->name_group,
                    m * sizeof(char*));
            idx->name_tag = (char**) realloc(idx->name_tag, m * sizeof(char*));
        }

        /* Initialise the tag index data. */
        idx->user_index[i] = 0;
        idx->block_size_bytes[i] = 0;
        idx->data_size_bytes[i] = 0;
        idx->data_offset_bytes[i] = 0;
        idx->name_group[i] = 0;
        idx->name_tag[i] = 0;

        /* Try to read a tag, and end the loop if unsuccessful. */
        if (fread(&(idx->tag[i]), sizeof(oskar_BinaryTag), 1, stream) != 1)
            break;

        /* If the bytes read are not a tag, then return an error. */
        if (idx->tag[i].magic[0] != 'T' ||
                idx->tag[i].magic[1] != 'A' ||
                idx->tag[i].magic[2] != 'G' ||
                idx->tag[i].magic[3] != 0)
            return OSKAR_ERR_BAD_BINARY_FORMAT;

        /* Copy out the index. */
        memcpy_size = MIN(sizeof(int), sizeof(idx->tag[i].user_index));
        memcpy(&idx->user_index[i], idx->tag[i].user_index, memcpy_size);

        /* Get the index in native byte order. */
        if (oskar_endian() != OSKAR_LITTLE_ENDIAN)
        {
            if (sizeof(int) == 2)
                oskar_endian_swap_2((char*)(&idx->user_index[i]));
            else if (sizeof(int) == 4)
                oskar_endian_swap_4((char*)(&idx->user_index[i]));
            else if (sizeof(int) == 8)
                oskar_endian_swap_8((char*)(&idx->user_index[i]));
        }

        /* Copy out the number of bytes in the block. */
        memcpy_size = MIN(sizeof(size_t), sizeof(idx->tag[i].size_bytes));
        memcpy(&idx->block_size_bytes[i], idx->tag[i].size_bytes, memcpy_size);

        /* Get the number of bytes in the block in native byte order. */
        if (oskar_endian() != OSKAR_LITTLE_ENDIAN)
        {
            if (sizeof(size_t) == 2)
                oskar_endian_swap_2((char*)(&idx->block_size_bytes[i]));
            else if (sizeof(size_t) == 4)
                oskar_endian_swap_4((char*)(&idx->block_size_bytes[i]));
            else if (sizeof(size_t) == 8)
                oskar_endian_swap_8((char*)(&idx->block_size_bytes[i]));
        }

        /* Store the data size. */
        idx->data_size_bytes[i] = idx->block_size_bytes[i];

        /* Check if the tag is extended. */
        if (idx->tag[i].flags & (1 << 7))
        {
            size_t lgroup, ltag;

            /* Get the lengths of the strings. */
            lgroup = idx->tag[i].group.bytes;
            ltag = idx->tag[i].tag.bytes;

            /* Modify data size. */
            idx->data_size_bytes[i] -= (lgroup + ltag);

            /* Allocate memory for the tag names. */
            idx->name_group[i] = (char*) malloc(lgroup);
            idx->name_tag[i]   = (char*) malloc(ltag);

            /* Copy the tag names into the index. */
            if (fread(idx->name_group[i], 1, lgroup, stream) != lgroup)
                return OSKAR_ERR_BAD_BINARY_FORMAT;
            if (fread(idx->name_tag[i], 1, ltag, stream) != ltag)
                return OSKAR_ERR_BAD_BINARY_FORMAT;
        }

        /* Store the current stream pointer as the data offset. */
        idx->data_offset_bytes[i] = ftell(stream);

        /* Increment stream pointer by data size. */
        fseek(stream, idx->data_size_bytes[i], SEEK_CUR);

        /* Save the number of tags read from the stream. */
        idx->num_tags = i + 1;
    }

    return OSKAR_SUCCESS;
}

#ifdef __cplusplus
}
#endif