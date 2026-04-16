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

// center of OLED display
#define OLED_CENTER_X 64
#define OLED_CENTER_Y 16

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
 void i2c_read_multi(unsigned char address, unsigned char reg, uint8_t *data_store){
    int length = sizeof(data_store);
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, address, data_store, length, false);  // false - finished with bus
}

void IMU_read_shift(){
    uint8_t data[14];
    i2c_read_multi(MPU_ADDR, ACCEL_XOUT_H, data);
    
    // read accelerations and convert to units of g
    uint16_t accel_x = (data[0] << 8) | data[1];
    uint16_t accel_y = (data[2] << 8) | data[3];
    uint16_t accel_z = (data[4] << 8) | data[5];
    float accel_x_g = accel_x * 0.000061;
    float accel_y_g = accel_y * 0.000061;

    // read temperature and convert to degrees C
    uint16_t temp = (data[6] << 8) | data[7];
    float temp_c = 36.53 + temp/340.0;

    // read gyroscope and convert to degrees per second
    uint16_t gyro_x = (data[8] << 8) | data[9];
    uint16_t gyro_y = (data[10] << 8) | data[11];
    uint16_t gyro_z = (data[12] << 8) | data[13];
    float gyro_x_deg = gyro_x * 0.007630;
    float gyro_y_deg = gyro_y * 0.007630;
    float gyro_z_deg = gyro_z * 0.007630;
}

void drawLine(float a_x, float a_y){
    int x1 = OLED_CENTER_X + a_x;
    int y1 = OLED_CENTER_Y + a_y;
    
}

int main()
{
    stdio_init_all();
    gpio_initialization();
    MPU_init();

    uint8_t who_am_i =i2c_read(MPU_ADDR, WHO_AM_I); // check if i2c bus is working if returns 0x68
    if (who_am_i != 0x68 && who_am_i != 0x96){
        while(true){
            gpio_put(25, 1); // turn on LED to alert for power reset
        }
    }

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
