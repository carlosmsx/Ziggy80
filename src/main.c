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
#include "marat/Z80.h"
#include "z80pico.h"

// create a CPU core object
Z80 cpu;

volatile bool vdp_int=false;

bool vdp_int_callback(struct repeating_timer *t) 
{
    vdp_int = true;
    return true;
}

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
  
    InitPPI();
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
  
    //Reset the CPU to 0x00 and zero the regs/flags
    ResetZ80(&cpu);
    struct repeating_timer timer;
    add_repeating_timer_ms(-25, vdp_int_callback, NULL, &timer);

    for (;;)
    {
        // execute single opcode from memory at the current PC
        StepZ80(&cpu);
        if (vdp_int)
        {
            IntZ80(&cpu, INT_RST38);
            vdp_int = false;
        }
    }
}

