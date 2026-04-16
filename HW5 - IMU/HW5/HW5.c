#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA_IMU 8
#define I2C_SCL_IMU 9
#define I2C_SDA_OLED 20
#define I2C_SCL_OLED 21

#define MPU_ADDR 0x68
// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

void MPU_init(){
    i2c_write(MPU_ADDR, PWR_MGMT_1, 0x00);// write 0x00 to PWR_MGMT_1 to turn chip on
    i2c_write(MPU_ADDR, ACCEL_CONFIG, 0x00); // write to ACCEL_CONFIG to enable accelerometer, sensitivity to +/- 2g
    i2c_write(MPU_ADDR, GYRO_CONFIG, 0x00); // write to GYRO_CONFIG to enable gyropscope, sensitivity to +/- 2000 dps
}

void i2c_write(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

void gpio_initialization(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA_IMU, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_IMU, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_IMU);
    gpio_pull_up(I2C_SCL_IMU);

    gpio_set_function(I2C_SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_OLED);
    gpio_pull_up(I2C_SCL_OLED);
}

unsigned char i2c_read(unsigned char address, unsigned char reg){
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    uint8_t value;
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);  // false - finished with bus
    return value;
}

int main()
{
    stdio_init_all();
    gpio_initialization();
    MPU_init();
    ssd1306_setup();
    ssd1306_clear();
    
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
