/*********************************************
 * Autor: Carlos Escobar
 * Jul-2023
 * Ref:
 *********************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
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
    // const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);

    sleep_ms(500);
    InitRAM();
    SetupPIO();
    // gpio_put(LED_PIN, 1);
   
    InitPPI();
    Ti99Splash();
    sleep_ms(2000);
  
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

