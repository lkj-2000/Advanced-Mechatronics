#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

void drawLetter(int x, int y, char letter){
    int ascii_ind = letter - 20; // ascii offset
    if (ascii_ind < 0 || ascii_ind > 95) {
        return; // character not in font
    }
    for (int i = 0; i < 5; i++) { // width of each character
        char col = ASCII[ascii_ind][i];
        for (int j = 0; j < 8; j++) { // height of each character
           // ((col >> j) & 1) {
            ssd1306_drawPixel(x + i, y + j, 1);
           //
        }
    }
}
void printArray(int x, int y, char* str){

}


int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    ssd1306_setup();
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // adc initialisation
    adc_init();
    adc_gpio_init(26); // GP26 is ADC0
    adc_select_input(0); // select ADC input 0

    gpio_init(16);
    gpio_set_dir(16, GPIO_OUT);
    gpio_put(16, 1); // turn on onboard LED to indicate program is running

    ssd1306_clear();
    ssd1306_update();

    while (true) {
     //printArray(0, 0, "Hello World!");
       ssd1306_drawPixel(12,12,1);
       ssd1306_update();
    }
}
