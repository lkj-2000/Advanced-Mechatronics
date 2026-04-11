#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define ADDR 0b1000000
#define OLAT_ADDR 0b00001010 // 0Ah
#define IODIR_ADDR 0b00000000 // 00h
uint8_t buf[2];

void setPin(unsigned char address, unsigned char reg, unsigned char value){
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
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

    setPin(ADDR, IODIR_ADDR, 0b01111111); // set GP7 as output and GP0 as input (write an unsigned char to IODIR to set direction of each pin)
}

unsigned char readPin(unsigned char address, unsigned char reg){
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, address, &buf[1], 1, false);  // false - finished with bus
    return buf[1];
}

int main()
{
    chip_initialization();

    // read from GPIO to know if button is pushed
    while (true) {
        setPin(ADDR, OLAT_ADDR, 0b10000000);  // set bit 0 to 1 (turn pin on)
        sleep_ms(1000);
        setPin(ADDR, OLAT_ADDR, 0b00000000);   // set bits to OLAT to turn pins on or off
        sleep_ms(1000);
        // if readPin(ADDR, ){
        //     setPin(ADDR, OLAT_ADDR, 0b00000001);  // set bit 0 to 1 (turn pin on)
        //  }
        // else{
        //     setPin(ADDR, OLAT_ADDR, 0b00000000);   // set bits to OLAT to turn pins on or off           
        // }
    }
}
