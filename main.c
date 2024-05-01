/*
 * SPDX-FileCopyrightText: Copyright 2010-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arm_nnfunctions.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASIC_OUT_CH 1
#define BASIC_IN_CH 1
#define BASIC_INPUT_W 5
#define BASIC_INPUT_H 8
#define BASIC_DST_SIZE 20
#define BASIC_INPUT_SIZE 40
#define BASIC_OUT_ACTIVATION_MIN -128
#define BASIC_OUT_ACTIVATION_MAX 127
#define BASIC_INPUT_BATCHES 1
#define BASIC_FILTER_X 2
#define BASIC_FILTER_Y 4
#define BASIC_STRIDE_X 1
#define BASIC_STRIDE_Y 1
#define BASIC_PAD_X 0
#define BASIC_PAD_Y 0
#define BASIC_OUTPUT_W 4
#define BASIC_OUTPUT_H 5
#define BASIC_INPUT_OFFSET 128
#define BASIC_OUTPUT_OFFSET 127
#define BASIC_DILATION_X 1
#define BASIC_DILATION_Y 1

const int8_t basic_weights[8] = {-72, 32, -107, -50, 81, -114, -7, -127};
const int32_t basic_biases[1] = {6388};

const int32_t basic_output_mult[1] = {1625013239};
const int32_t basic_output_shift[1] = {-8};

const int8_t basic_input[40] = {73, -88, -95, 57,  106, 13, 34,  -103, 86, 12,  107,  37,  -4,  -22,
                                16, -87, 4,   -11, -21, 52, 41,  -122, 90, 124, -62,  -23, 103, 66,
                                68, 94,  -93, 89,  -4,  68, -89, -66,  3,  4,   -108, 63};

const int8_t basic_output_ref[20] = {-11, 37, 68,  -53, -8,  -47, -1,  -6, 29,  -86,
                                     -34, 27, -40, 34,  -71, 4,   -72, 21, -14, -35};

int validate(int8_t *act, const int8_t *ref, int size)
{
    int test_passed = true;
    int count = 0;
    int total = 0;

    for (int i = 0; i < size; ++i)
    {
        total++;
        if (act[i] != ref[i])
        {
            count++;
            printf("ERROR at pos %d: Act: %d Ref: %d\r\n", i, act[i], ref[i]);
            test_passed = false;
        }
    }

    if (!test_passed)
    {
        printf("%d of %d failed\r\n", count, total);
    }

    return test_passed;
}

void basic_arm_convolve_s8(void)
{
    const arm_cmsis_nn_status expected = ARM_CMSIS_NN_SUCCESS;
    int8_t output[BASIC_DST_SIZE] = {0};

    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims;
    cmsis_nn_dims output_dims;

    const int32_t *bias_data = basic_biases;
    const int8_t *kernel_data = basic_weights;
    const int8_t *input_data = basic_input;
    const int8_t *output_ref = basic_output_ref;
    const int32_t output_ref_size = BASIC_DST_SIZE;

    input_dims.n = BASIC_INPUT_BATCHES;
    input_dims.w = BASIC_INPUT_W;
    input_dims.h = BASIC_INPUT_H;
    input_dims.c = BASIC_IN_CH;
    filter_dims.w = BASIC_FILTER_X;
    filter_dims.h = BASIC_FILTER_Y;
    filter_dims.c = BASIC_IN_CH;
    output_dims.w = BASIC_OUTPUT_W;
    output_dims.h = BASIC_OUTPUT_H;
    output_dims.c = BASIC_OUT_CH;

    conv_params.padding.w = BASIC_PAD_X;
    conv_params.padding.h = BASIC_PAD_Y;
    conv_params.stride.w = BASIC_STRIDE_X;
    conv_params.stride.h = BASIC_STRIDE_Y;
    conv_params.dilation.w = BASIC_DILATION_X;
    conv_params.dilation.h = BASIC_DILATION_Y;

    conv_params.input_offset = BASIC_INPUT_OFFSET;
    conv_params.output_offset = BASIC_OUTPUT_OFFSET;
    conv_params.activation.min = BASIC_OUT_ACTIVATION_MIN;
    conv_params.activation.max = BASIC_OUT_ACTIVATION_MAX;
    quant_params.multiplier = (int32_t *)basic_output_mult;
    quant_params.shift = (int32_t *)basic_output_shift;

    int32_t buf_size = arm_convolve_s8_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;

    arm_cmsis_nn_status result = arm_convolve_s8(&ctx,
                                                 &conv_params,
                                                 &quant_params,
                                                 &input_dims,
                                                 input_data,
                                                 &filter_dims,
                                                 kernel_data,
                                                 &bias_dims,
                                                 bias_data,
                                                 &output_dims,
                                                 output);

    if (expected != result) { 
      printf("The result is different from the expected value\n");
    }
    if (!validate(output, output_ref, output_ref_size)) {
      printf("Failed to validate the output\n");
    }
}

int main(){
  basic_arm_convolve_s8();
}
