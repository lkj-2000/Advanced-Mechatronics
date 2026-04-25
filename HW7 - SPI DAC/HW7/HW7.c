#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define SPI_PORT
#define PIN_MISO
#define PIN_CS_DAC
#define PIN_SCK
#define PIN_MOSI


int main()
{
    stdio_init_all();

    // SPI initialization
    spi_init(SPI_PORT, 1000 * 1); // the baud, or bits per second
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);

    while (true) {
        // call writeDAC
        float t = 0;
        t = t+0.1; // adjusts how choppy output is
        float voltage = (sine(2*pi*f*t)+1)/2*3.3;

        writeDAC(channel, voltage)
        sleep_ms(10);
    }
}

void writeDAC(int channel, float v){
    uint8_t data[2];

    data[0] = 0b01110000;

    data[0] = data[0] | ((channel&0b1)<<7) // put the channel bit in

    uint16_t myV = v/3.3*1023

    data[0] = data[0] | (myV>>6)&0b00001111

    data[1] = (myV<<2)&0xFF // ignores last two bits of 10 bit number

    data[1] = 0b11111100;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}