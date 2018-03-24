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


#include <mcu/types.h>
#include <mcu/core.h>
#include <mcu/bootloader.h>
#include <mcu/arch.h>

#include "link_transport.h"
#include "board_config.h"


const struct __sFILE_fake __sf_fake_stdin;
const struct __sFILE_fake __sf_fake_stdout;
const struct __sFILE_fake __sf_fake_stderr;

#include "../src/board_arch_config.h"

//MCU Board configuration -- same config as Stratify OS
STM32_NUCLEO144_DECLARE_MCU_BOARD_CONFIG(SOS_BOARD_SYSTEM_CLOCK, 0, &stm32_arch_config, SOS_BOARD_USB_RX_BUFFER_SIZE);

//Bootloader configuration (don't need Stratify OS configuration for just the bootloader)
STM32_NUCLEO144_DECLARE_BOOT_BOARD_CONFIG(&link_transport);

extern void boot_main();

//Execute the Stratify OS default bootloader
void _main(){ boot_main(); }
