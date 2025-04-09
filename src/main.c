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

#define TECLA_PIN 18
#define VDP_INT_PIN 19

// create a CPU core object
Z80 cpu;

int main() 
{
    stdio_init_all();
    set_sys_clock_khz(250000, false);


    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

    InitRAM();
    SetupPIO();
    gpio_init(TECLA_PIN);
    gpio_set_dir(TECLA_PIN, GPIO_IN); //lee tecla
    gpio_pull_up(TECLA_PIN);
    gpio_init(VDP_INT_PIN);
    gpio_set_dir(VDP_INT_PIN, GPIO_IN);

    InitPPI();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(200);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
  
    //Reset the CPU to 0x00 and zero the regs/flags
    ResetZ80(&cpu);
    // static struct repeating_timer timer;
    // add_repeating_timer_ms(-30, vdp_int_callback, NULL, &timer); //-20=50hs, -16=60hz
    for (;;)
    {
        // execute single opcode from memory at the current PC
        StepZ80(&cpu);
        if (gpio_get(VDP_INT_PIN) == 0)
        {
            IntZ80(&cpu, INT_RST38);
        }
    }
}

