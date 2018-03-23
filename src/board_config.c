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


#include <sys/lock.h>
#include <fcntl.h>
#include <errno.h>
#include <mcu/mcu.h>
#include <mcu/debug.h>
#include <mcu/periph.h>
#include <device/microchip/sst25vf.h>
#include <device/sys.h>
#include <device/uartfifo.h>
#include <device/usbfifo.h>
#include <device/fifo.h>
#include <device/cfifo.h>
#include <device/sys.h>
#include <sos/link.h>
#include <sos/fs/sysfs.h>
#include <sos/fs/appfs.h>
#include <sos/fs/devfs.h>
#include <sos/fs/sffs.h>
#include <sos/sos.h>

#include "board_trace.h"
#include "link_transport.h"

//openocd -f interface/stlink-v2-1.cfg -f target/stm32f4x_stlink.cfg

#define SOS_BOARD_SYSTEM_CLOCK 168000000
#define SOS_BOARD_PERIPH_CLOCK (SOS_BOARD_SYSTEM_CLOCK/4)
#define SOS_BOARD_SYSTEM_MEMORY_SIZE (8192*3)
#define SOS_BOARD_TASK_TOTAL 10

//--------------------------------------------MCU Configuration-------------------------------------------------

static void board_event_handler(int event, void * args);

#define USB_RX_BUFFER_SIZE 512
char usb_rx_buffer[USB_RX_BUFFER_SIZE] MCU_SYS_MEM;

const mcu_board_config_t mcu_board_config = {
    .core_osc_freq = 8000000,
    .core_cpu_freq = SOS_BOARD_SYSTEM_CLOCK,
    .core_periph_freq = SOS_BOARD_SYSTEM_CLOCK,
    .usb_max_packet_zero = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
    .debug_uart_port = 2,
    .debug_uart_attr = {
        UART_DEFINE_ATTR(
            UART_FLAG_SET_LINE_CODING_DEFAULT, 8, 115200,
            3, 8, //tx port, pin
            3, 9, //rx port, pin
            0xff, 0xff, //rts port, pin
            0xff, 0xff) //cts port, pin
    },
    .o_flags = MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH,
    .event_handler = board_event_handler,
    .led = {1, 7},
    .usb_rx_buffer = usb_rx_buffer,
    .usb_rx_buffer_size = USB_RX_BUFFER_SIZE
};

void board_event_handler(int event, void * args){
    switch(event){
    case MCU_BOARD_CONFIG_EVENT_ROOT_TASK_INIT:
        break;

    case MCU_BOARD_CONFIG_EVENT_ROOT_FATAL:
        //start the bootloader on a fatal event
        //mcu_core_invokebootloader(0, 0);
        if( args != 0 ){
            mcu_debug_printf("Fatal Error %s\n", (const char*)args);
        } else {
            mcu_debug_printf("Fatal Error unknown\n");
        }
        while(1){
            ;
        }
        break;

    case MCU_BOARD_CONFIG_EVENT_START_INIT:
        break;

    case MCU_BOARD_CONFIG_EVENT_START_LINK:
        mcu_debug_user_printf("Start LED %d\n", mcu_config.irq_middle_prio);
        sos_led_startup();
        break;

    case MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM:
        mcu_debug_user_printf("Started %ld apps\n", *((u32*)args));
        break;
    }
}

//--------------------------------------------Stratify OS Configuration-------------------------------------------------

const sos_board_config_t sos_board_config = {
    .clk_usecond_tmr = 1, //TIM2 -- 32 bit timer
    .task_total = SOS_BOARD_TASK_TOTAL,
    .stdin_dev = "/dev/stdio-in" ,
    .stdout_dev = "/dev/stdio-out",
    .stderr_dev = "/dev/stdio-out",
    .o_sys_flags = SYS_FLAG_IS_STDIO_FIFO | SYS_FLAG_IS_TRACE,
    .sys_name = "Nucleo-F429ZI",
    .sys_version = "0.5",
    .sys_id = "-L6TkvdQalXZTxgM_74-",
    .sys_memory_size = SOS_BOARD_SYSTEM_MEMORY_SIZE,
    .start = sos_default_thread,
    .start_args = &link_transport,
    .start_stack_size = SOS_DEFAULT_START_STACK_SIZE,
    .socket_api = 0,
    .request = 0,
    .trace_dev = "/dev/trace",
    .trace_event = board_trace_event
};

SOS_DECLARE_TASK_TABLE(SOS_BOARD_TASK_TOTAL);

//--------------------------------------------Device Filesystem-------------------------------------------------

UARTFIFO_DECLARE_CONFIG_STATE(uart0_fifo, 1024,
                              UART_FLAG_SET_LINE_CODING_DEFAULT, 8, 115200,
                              0, 2,
                              0, 3,
                              0xff, 0xff,
                              0xff, 0xff);

FIFO_DECLARE_CONFIG_STATE(stdio_in, 512);
FIFO_DECLARE_CONFIG_STATE(stdio_out, 512);
CFIFO_DECLARE_CONFIG_STATE_4(board_fifo, 256);

/* This is the list of devices that will show up in the /dev folder
 * automatically.  By default, the peripheral devices for the MCU are available
 * plus some devices on the board.
 */
const devfs_device_t devfs_list[] = {
    //System devices
    DEVFS_DEVICE("trace", ffifo, 0, &board_trace_config, &board_trace_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("fifo", cfifo, 0, &board_fifo_config, &board_fifo_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("stdio-out", fifo, 0, &stdio_out_config, &stdio_out_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("stdio-in", fifo, 0, &stdio_in_config, &stdio_in_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("link-phy-usb", usbfifo, 0, &sos_link_transport_usb_fifo_cfg, &sos_link_transport_usb_fifo_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("sys", sys, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

    //MCU peripherals
    DEVFS_DEVICE("core", mcu_core, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("core0", mcu_core, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

    DEVFS_DEVICE("i2c0", mcu_i2c, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("i2c1", mcu_i2c, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("i2c2", mcu_i2c, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("i2c3", mcu_i2c, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

    DEVFS_DEVICE("pio0", mcu_pio, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOA
    DEVFS_DEVICE("pio1", mcu_pio, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOB
    DEVFS_DEVICE("pio2", mcu_pio, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOC
    DEVFS_DEVICE("pio3", mcu_pio, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOD
    DEVFS_DEVICE("pio4", mcu_pio, 4, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOE
    DEVFS_DEVICE("pio5", mcu_pio, 5, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOF
    DEVFS_DEVICE("pio6", mcu_pio, 6, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOG
    DEVFS_DEVICE("pio7", mcu_pio, 7, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //GPIOH

    DEVFS_DEVICE("spi0", mcu_spi, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("spi1", mcu_spi, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("spi2", mcu_spi, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("spi3", mcu_spi, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

    DEVFS_DEVICE("tmr0", mcu_tmr, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //TIM1
    DEVFS_DEVICE("tmr1", mcu_tmr, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //TIM2
    DEVFS_DEVICE("tmr2", mcu_tmr, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr3", mcu_tmr, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr4", mcu_tmr, 4, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr5", mcu_tmr, 5, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr6", mcu_tmr, 6, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr7", mcu_tmr, 7, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR), //TIM8
    //Does this chip have more timers?

    DEVFS_DEVICE("uart0", uartfifo, 0, &uart0_fifo_config, &uart0_fifo_state, 0666, SOS_USER_ROOT, S_IFCHR),
    DEVFS_TERMINATOR
};


//--------------------------------------------Root Filesystem-------------------------------------------------

const devfs_device_t mem0 = DEVFS_DEVICE("mem0", mcu_mem, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFBLK);
const sysfs_t sysfs_list[] = {
    APPFS_MOUNT("/app", &mem0, SYSFS_ALL_ACCESS), //the folder for ram/flash applications
    DEVFS_MOUNT("/dev", devfs_list, SYSFS_READONLY_ACCESS), //the list of devices
    //SFFS_MOUNT("/home", &sffs_cfg, SYSFS_ALL_ACCESS), //the stratify file system on external RAM
    //FATFS("/home", &fatfs_cfg, SYSFS_ALL_ACCESS), //fat filesystem with external SD card
    SYSFS_MOUNT("/", sysfs_list, SYSFS_READONLY_ACCESS), //the root filesystem (must be last)
    SYSFS_TERMINATOR
};


