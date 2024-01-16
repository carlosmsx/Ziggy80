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

bool cpuclk_callback(struct repeating_timer *t) 
{
    static int x=0;
    gpio_put(0, x&1);
    x++;
    return true;
}

int main()
{
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);

    SetupPIOTest();

    struct repeating_timer timer;
    add_repeating_timer_ms(-20, cpuclk_callback, NULL, &timer);

    uint8_t bit=1;
    for (;;)
    {
        PIO_Test(0,bit);
        sleep_ms(500);
        bit = bit<<1;
        if (!bit) bit=1;
    }
     
}
