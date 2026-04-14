#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define ADDR 0b0100000 // 0x20
#define OLAT_ADDR 0b00001010 // 0Ah
#define GPIO_ADDR 0b00001001 // 09h
#define IODIR_ADDR 0b00000000 // 00h

void setPin(unsigned char reg, unsigned char value){
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

void chip_initialization(){
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    //gpio_pull_up(I2C_SDA);
    //gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    setPin(IODIR_ADDR, 0b01111111); // set GP7 as output and GP0 as input (write an unsigned char to IODIR to set direction of each pin)
}

unsigned char readPin(unsigned char reg){
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);  // true to keep host control of bus
    uint8_t value;
    i2c_read_blocking(I2C_PORT, ADDR, &value, 1, false);  // false - finished with bus
    return value;
}

void led_flash(){
    setPin(GPIO_ADDR, 0b10000000);  // set bit 0 to 1 (turn pin on)
    sleep_ms(1000);
    setPin(GPIO_ADDR, 0b00000000);
    sleep_ms(1000);
}

int main()  
{
    chip_initialization();

    while (true) {
        // led_flash();

        uint8_t gpio_current = readPin(GPIO_ADDR);  // read from GPIO to know if button is pushed
        // if button pushed (low), need to set high
        if ((gpio_current & 0b00000001) == 0){
            setPin(GPIO_ADDR, gpio_current | 0b10000000);  // set bit 0 to 1 (turn pin on)
        }
        else{
            setPin(GPIO_ADDR, gpio_current & 0b01111111);
        }
    }
}
