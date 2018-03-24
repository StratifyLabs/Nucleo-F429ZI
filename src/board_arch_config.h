/*

Copyright 2011-2018 Tyler Gilbert

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

#ifndef BOARD_ARCH_CONFIG_H_
#define BOARD_ARCH_CONFIG_H_

#include <mcu/arch.h>

#define SOS_BOARD_SYSTEM_CLOCK 168000000
#define SOS_BOARD_PERIPH_CLOCK (SOS_BOARD_SYSTEM_CLOCK/4)
#define SOS_BOARD_SYSTEM_MEMORY_SIZE (8192*3)
#define SOS_BOARD_ID "-L6TkvdQalXZTxgM_74-"
#define SOS_BOARD_VERSION "0.5"
#define SOS_BOARD_NAME "Nucleo-F429ZI"

#define SOS_BOARD_USB_RX_BUFFER_SIZE 512

#define SOS_BOARD_STDIO_BUFFER_SIZE 512

//Total number of tasks (threads) for the entire system
#define SOS_BOARD_TASK_TOTAL 10

extern const stm32_arch_config_t stm32_arch_config;



#endif /* BOARD_ARCH_CONFIG_H_ */
