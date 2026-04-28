#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19

#define PI 3.14159265358979323846264338795028841971

void writeDAC(int channel, float v);
static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

int main()
{
    stdio_init_all();

    // SPI initialization
    spi_init(SPI_PORT, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float t = 0;
    float dt = 0.01;

    while (true) {
        // 2Hz sin wave using (2*pi*f*t)
        // sinf is between -1 and 1, so need to add 1 and scale our Vref by multiplying the amplitude by 1/2
        float sin_voltage = (sinf(2*PI*t) + 1) * (3.3/2);
        writeDAC(0, sin_voltage); // channel A

        static bool up = true;
        static float tri_volt = 0;

        if (up){ // triangle slope up
            tri_volt += 3.3*dt;
            if (tri_volt >= 3.3){
                tri_volt = 3.3;
                up = false;
            }
        }
        else{ // triangle slope down
            tri_volt -= 3.3*dt; 
            if (tri_volt <= 0){
                tri_volt = 0;
                up = true;
            }
        }
        writeDAC(1, tri_volt); // channel B

        // float voltage = 2.0f;
        // writeDAC(0, voltage);

        sleep_ms(10);
        printf("voltage: %f\n",tri_volt);
        t += dt; // adjusts how choppy output is
    }
}

void writeDAC(int channel, float v){
    //convert voltage to 12 bit DAC code from 0 to 1023
    uint16_t DAC = (uint16_t)(v/3.3*1023);

    // bits 12-15: SHDN, gain, BUF, DACA/B
    uint16_t bit16 = 0;

    bit16 |= (channel & 0x1) << 15;
    bit16 |= (1 << 14);    
    bit16 |= (1 << 13);   
    bit16 |= (1 << 12);
    bit16 |= (DAC & 0x0FFF); 

    // split into two 8 bytes
    uint8_t data[2];
    data[0] = (bit16 >> 8) & 0xFF;
    data[1] = bit16 & 0xFF;

    // data[0] = 0b01110000;
    // data[0] |= ((channel&0b1) << 15); // put the channel bit in, A=0 or B=1 based on sine or triangle
    // data[0] |= (DAC >> 12) & 0xFF; // fills in last 4 bits of 8
    // data[1] = (DAC << 2) & 0xFF; // ignores last two bits of 10 bit number

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}