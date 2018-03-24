/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/




#include <mcu/arch.h>

#include "board_arch_config.h"

const stm32_arch_config_t stm32_arch_config = {
    .o_flags = 0,
    .clock_pllm = 4,
    .clock_plln = 168,
    .clock_pllp = 2,
    .clock_pllq = 7,
    .clock_ahb_clock_divider = 1,
    .clock_apb1_clock_divider = 2,
    .clock_apb2_clock_divider = 1,
    .clock_48_clock_selection = 0, //NA
    .clock_voltage_scale = 1,
    .clock_flash_latency = 5
};
