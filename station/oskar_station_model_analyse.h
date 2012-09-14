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

#ifndef OSKAR_STATION_MODEL_ANALYSE_H_
#define OSKAR_STATION_MODEL_ANALYSE_H_

/**
 * @file oskar_station_model_analyse.h
 */

#include "oskar_global.h"
#include "station/oskar_StationModel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * Analyses a station model to determine its properties.
 *
 * @details
 * This function analyses a station model to determine whether element errors
 * and/or weights should be applied.
 * The relevant flags within the structure are updated.
 *
 * @param[in,out] station Station model structure to analyse.
 * @param[out] finished_identical_station_check Flag set if stations cannot be identical.
 * @param[in,out]  status   Status return code.
 */
OSKAR_EXPORT
void oskar_station_model_analyse(oskar_StationModel* station,
        int* finished_identical_station_check, int* status);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_STATION_MODEL_ANALYSE_H_ */
