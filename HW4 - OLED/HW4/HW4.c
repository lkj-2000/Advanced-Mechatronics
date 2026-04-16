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
    int ascii_ind = letter - 32; // ascii offset
    if (ascii_ind < 0 || ascii_ind > 95) {
        return; // character not in font
    }
    for (int i = 0; i < 5; i++) { // width of each character
        char col = ASCII[ascii_ind][i]; // each index of ASCII matrix is binary number of the column of the character
        for (int j = 0; j < 8; j++) { // height of each character
           if ((col >> j) & 1) { // picking out correct number in binary number to see if pixel should be on
            ssd1306_drawPixel(x + i, y + j, 1);
           }
        }
    }
}
void printArray(int x, int y, char* str){
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) { // loop through characters in string until null terminator
        if (x + j*6 > 128) { // check if character needs to wrap (128 pixels wide)
            y += 8;
            j = 0;
        } else {
            j++;
        }
        drawLetter(x + j*6, y, str[i]); // draw each character, with 1 pixel space in between
    }
}

void ledBlink(){
    gpio_put(16, 1); // turn on LED
    sleep_ms(500);
    gpio_put(16, 0); // turn off LED
    sleep_ms(500);
}


int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();

    // adc initialisation
    adc_init();
    adc_gpio_init(26); // GP26 is ADC0
    adc_select_input(0); // select ADC input 0

    gpio_init(16);
    gpio_set_dir(16, GPIO_OUT);
    gpio_put(16, 1); // turn on onboard LED to indicate program is running

    ssd1306_clear();
    ssd1306_update();

    unsigned int t_start;
    unsigned int t_end;
    float FPS;

    while (true) {
        ssd1306_clear();
        // ledBlink(); // blink LED to indicate program is running
        t_start = to_us_since_boot(get_absolute_time()); 

        uint16_t adc_value = adc_read(); // read value from ADC
        float voltage = adc_value * 3.3 / 4095.0; // convert ADC value to voltage (12-bit ADC, so 2^12 possible values)
        
        printArray(0, 0, "i am anisha"); // print string on display
        char volt_str[15];
        sprintf(volt_str, "Voltage: %.2fV", voltage); // convert voltage to string with
        printArray(1, 10, volt_str);
        ssd1306_update();

        t_end = to_us_since_boot(get_absolute_time()); 
        FPS = 1000000.0 / (t_end - t_start);
        char FPS_str[15];
        sprintf(FPS_str, "FPS: %.2f", FPS);
        printArray(1,20,FPS_str);
        ssd1306_update();
    }
}
