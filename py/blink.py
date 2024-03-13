from machine import Pin, Timer
import rp2, time
from rp2 import PIO, StateMachine #, asm_pi

@rp2.asm_pio(out_init=(PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW), sideset_init=(PIO.OUT_LOW, PIO.OUT_LOW))
def z80pico():
    NONE = 0
    CLK1 = 1
    CLK0 = 0
    ADDR_L = 1
    ADDR_H = 2
    CPUCLK = 17
    
    wrap_target()
#inicio:
    mov(osr,invert(null))   .side(NONE) # INVERT!!!!!!
    out(pindirs,8)          .side(NONE) # set data bus as output
    pull(block)             .side(NONE) # espero operacion
    wait(CLK0, gpio, CPUCLK).side(NONE) # necesario para estabilizar la sincronizacion
    wait(CLK1, gpio, CPUCLK).side(NONE) # sincronizo T1
    irq(0)                  .side(NONE) # disparo secuencia
#T1:
#; CODIGO CON 74LS573
    out(pins,8)             .side(ADDR_L) .delay(2) # LE durante 2 ciclos
    nop()                   .side(NONE)   .delay(1) # termino LE y espero un ciclo mas
    out(pins,8)             .side(ADDR_H) .delay(2) # LE durante 2 ciclos
    wait(CLK0, gpio, CPUCLK).side(NONE)   .delay(1) # termino LE y espero T1'
#; CODIGO CON 74LS574
#; out pins,8            side NONE   ; presento addr_low  
#; nop                   side ADDR_L ; CLK 
#; out pins,8            side NONE   ; presento addr_high
#; wait CLK0 gpio CPUCLK side ADDR_H ; CLK 
#T1_:
    out(pins,8)             .side(NONE) # data
    out(pindirs, 8)         .side(NONE) # set data bus as input/output

    wait( 1, irq, 1)        .side(NONE) # waits ending of sequence
#T3:
    in_(pins,32)            .side(NONE) # TODO: is it possible to set threshold for input in order to avoid PUSH?
    wait(CLK0, gpio, CPUCLK).side(NONE) # espero T3'
#T3_:
    push()                  .side(NONE)
    wrap()

@rp2.asm_pio(out_init=(PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW, PIO.OUT_LOW))
def z80sequencer():
    CPUCLK= 17
    CLK1= 1
    CLK0=0
    wrap_target()
    mov(osr,invert(null))
    out(pins,5)                 # set all signals to 1
    pull(block)
    wait(1, irq, 0)             #[SHIFT] ; estoy en T1
    wait(CLK0,gpio,CPUCLK)      #[SHIFT] ; espero T1'
    out(pins,5)                 .delay(15)
    out(pins,5)                 .delay(15)
    out(pins,5)                 .delay(15)
    nop()                       .delay(31) # wait state
    out(pins,5)                 # doy un tiempo antes de hacer el IN en sm0
    irq(1)                      .delay(16)
    out(pins,5)                 .delay(16)
    wrap()

@rp2.asm_pio()
def test():
    wrap_target()
    pull(block)
    out(x,32)
    jmp(x_dec,"sigue")
    label("sigue")
    mov(isr,invert(null))
    push()
    wrap()
    

def OutZ80(port, data):
    z = (0xff << 24) | (data << 16) | port;
    #pio_sm_put(pio0, sm_z80sequencer, outSeq);
    _sm1.put(0b1111101010010100101011111) #OUT
    #pio_sm_put(pio0, sm_z80pico, z);
    _sm.put(z)
    #pio_sm_get_blocking(pio0, sm_z80pico); //descarto resultado
    _sm.get()
    time.sleep(0.000001)  # Pausa de 1 microsegundo


def Test_PSG_1():
    OutZ80(0xa0, 7)
    OutZ80(0xa1, 0xB8) #//b1011 1000 ; apaga ruido y deja bit 7 en 1 que es el default para MSX
    for xx in range(0, 0x480):
        OutZ80(0xa0, 0)
        OutZ80(0xa1, xx & 0xff)
        OutZ80(0xa0, 1)
        OutZ80(0xa1, xx >> 8)
        OutZ80(0xa0, 8)
        OutZ80(0xa1, 8)
        time.sleep(0.001)  # Pausa de 1 milisegundo
    OutZ80(0xa0, 8)
    OutZ80(0xa1, 0)

def VDP_set(reg, value):
    reg = reg & 0x7          # solo registros de 0 a 7, TMS9918A
    OutZ80(0x99, value)      # paso el valor 
    OutZ80(0x99, 0x80 | reg) # a continuacion el registro destino
    #_vdp_reg[reg] = value    # guardo el valor para el get

def screen1(): #SCREEN 1
    # InZ80(0x99);
    VDP_set(0, 0x00)
    VDP_set(1, 0xe0)
    VDP_set(2, 0x06)
    VDP_set(3, 0x80)
    VDP_set(4, 0x00)
    VDP_set(5, 0x36)
    VDP_set(6, 0x07)
    VDP_set(7, 0x17) 
    #CharSet(0);

_sm = StateMachine(0, z80pico, in_base=Pin(2), out_base=Pin(2), sideset_base=Pin(10))
_sm.active(1)

_sm1 = StateMachine(1, z80sequencer, out_base=Pin(12))
_sm1.active(1)

screen1()
Test_PSG_1()
print("2")
print(0b0101 & 3)

"""
_sm = StateMachine(0, test)
_sm.active(1)
_sm.put(0)
print(_sm.get())
"""