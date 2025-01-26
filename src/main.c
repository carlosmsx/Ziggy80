/*********************************************
 * Autor: Carlos Escobar
 * Jul-2023
 * Ref:
 *********************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "msx.h"
#include "z80pico.h"

#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/ay38910.h"
#define _CPC_FREQUENCY (4000000)

//// create a CPU core object
z80_t cpu;
uint64_t pins;

// volatile bool vdp_int=false;

// bool vdp_int_callback(struct repeating_timer *t) 
// {
//     vdp_int = true;
//     return true;
// }

int main() 
{
    // stdio_init_all();
    set_sys_clock_khz(250000, false);


    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

    InitRAM();
    SetupPIO();
    gpio_init(18);
    gpio_set_dir(18, false); //lee tecla
    gpio_pull_up(18);
    // InitPPI();
    sleep_ms(200);

    // Ti99Splash();
    // Test_PSG_1();
    // sleep_ms(1000);
    // while (true) {
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //     sleep_ms(250);
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    //     sleep_ms(250);
    // }
  
    // initialize Z80 emu and execute some clock cycles
    pins = z80_init(&cpu);
    struct repeating_timer timer;
    // add_repeating_timer_ms(-25, vdp_int_callback, NULL, &timer); //60hz

    for (;;)
    {
        // execute single opcode from memory at the current PC
        pins = z80_tick(&cpu, pins);
        // if (cpu.step > 2) {
        //     if ((pins & Z80_INT)) {
        //         pins &= ~Z80_INT;
        //     }
        // }

        // handle memory read or write access
        if (pins & Z80_MREQ) {
            if (pins & Z80_RD) {
                Z80_SET_DATA(pins, RdZ80(Z80_GET_ADDR(pins)));
            } else if (pins & Z80_WR) {
                WrZ80(Z80_GET_ADDR(pins), Z80_GET_DATA(pins));
            }
        }
        else if ((pins & Z80_IORQ) & ~(pins & Z80_M1)) {
            if (pins & Z80_RD) {
                Z80_SET_DATA(pins, InZ80(Z80_GET_ADDR(pins)));
            } else if (pins & Z80_WR) {
                OutZ80(Z80_GET_ADDR(pins), Z80_GET_DATA(pins));
                // if ((Z80_GET_ADDR(pins)&0xff) == 0x98)
                //     return 1;
            }
        }
    }
}

