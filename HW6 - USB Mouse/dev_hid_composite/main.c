/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "math.h"

#include "usb_descriptors.h"

#define I2C_PORT i2c0
#define I2C_SDA_IMU 8
#define I2C_SCL_IMU 9

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
#define WHO_AM_I     0x75

#define TOGGLE_BUTTON 19
#define LED_PIN 16

bool circle_mode = false;
bool last_button = true;

// function prototypes
void MPU_init();
void gpio_initialization();
void i2c_write(unsigned char address, unsigned char reg, unsigned char value);
unsigned char i2c_read(unsigned char address, unsigned char reg);
void i2c_read_multi(unsigned char address, unsigned char reg, uint8_t *data_store);
void drawLine(float a_x, float a_y);
void check_mode_toggle();


void check_mode_toggle() {
    static uint32_t last_press_time = 0;

    bool current = gpio_get(TOGGLE_BUTTON);
    uint32_t now = board_millis();

    // detect falling edge + debounce
    if (last_button == 1 && current == 0) {
        if (now - last_press_time > 200) {  // 200 ms debounce
            circle_mode = !circle_mode;
            last_press_time = now;
        }
    }

    last_button = current;
}
void MPU_init(){
    i2c_write(MPU_ADDR, PWR_MGMT_1, 0x00);// write 0x00 to PWR_MGMT_1 to turn chip on
    i2c_write(MPU_ADDR, ACCEL_CONFIG, 0x00); // write to ACCEL_CONFIG to enable accelerometer, sensitivity to +/- 2g
    i2c_write(MPU_ADDR, GYRO_CONFIG, 0x00); // write to GYRO_CONFIG to enable gyropscope, sensitivity to +/- 2000 dps
}

void i2c_write(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

unsigned char i2c_read(unsigned char address, unsigned char reg){
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    uint8_t value;
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);  // false - finished with bus
    return value;
}

 void i2c_read_multi(unsigned char address, unsigned char reg, uint8_t *data_store){
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, address, data_store, 14, false);  // false - finished with bus
}


void gpio_initialization(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA_IMU, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_IMU, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_IMU);
    gpio_pull_up(I2C_SCL_IMU);

    gpio_init(TOGGLE_BUTTON);
    gpio_set_dir(TOGGLE_BUTTON, GPIO_IN);
    gpio_pull_up(TOGGLE_BUTTON);

    // LED (GP16)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void drawLine(float a_x, float a_y){
    int x0 = 0;
    int y0 = 0;

    int x1 = x0 + a_x;
    int y1 = y0 + a_y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1) {
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  gpio_initialization();
  stdio_init_all();
  MPU_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
    check_mode_toggle();
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t deltax = 0;
      int8_t deltay = 0;

      // circle mode
      static float angle = 0.0;
      static int8_t x = 0, y = 0;
      if (circle_mode){
        int radius = 7;
        deltax = (int8_t)(radius * cos(angle));
        deltay = (int8_t)(radius * sin(angle));
        angle += 0.05;

      } else {    // IMU controlled
        
        uint8_t data[14];
        i2c_read_multi(MPU_ADDR, ACCEL_XOUT_H, data);
    
        // read accelerations and convert to units of g
        int16_t accel_x = (data[0] << 8) | data[1];
        int16_t accel_y = (data[2] << 8) | data[3];
        // float accel_x_g = accel_x * 0.000061;
        // float accel_y_g = accel_y * 0.000061;
        
        if (accel_x > 12000) {
            deltax = 8;        // fast
        } else if (accel_x > 6000) {
            deltax = 4;        // medium
        } else if (accel_x > 2000) {
            deltax = 2;        // slow
        } else if (accel_x < -12000) {
            deltax = -8;
        } else if (accel_x < -6000) {
            deltax = -4;
        } else if (accel_x < -2000) {
            deltax = -2;
        }
        
        if (accel_y > 12000) {
            deltay = 8;
        } else if (accel_y > 6000) {
            deltay = 4;
        } else if (accel_y > 2000) {
            deltay = 2;
        } else if (accel_y < -12000) {
            deltay = -8;
        } else if (accel_y < -6000) {
            deltay = -4;
        } else if (accel_y < -2000) {
            deltay = -2;
        }
      }

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, deltax, deltay, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  board_led_write(circle_mode);  // ON = circle mode, OFF = IMU mode
  gpio_put(LED_PIN, circle_mode); // external LED (GP16)
  
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}