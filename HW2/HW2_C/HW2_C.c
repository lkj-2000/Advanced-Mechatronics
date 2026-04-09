#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define PWMPIN 18


void set_angle(int angle);

int main()
{
    stdio_init_all();
    
    gpio_set_function(PWMPIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWMPIN);
    // goal to maximize wrap (gives more possible discrete duty cycles) and minimize div
    float div = 50; //must be 1-255
    uint16_t wrap = 60000; //must be less than 65535
    pwm_set_clkdiv(slice_num, div);

    // sets PWM frequency and resolution
    // frequency is 150MHz / div / wrap, we're aiming for 50 Hz
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(PWMPIN,0); //sets duty cycle to 50%

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while (true) {
       int i = 0;
       for (i=0; i<180; i++){
        set_angle(i);
        sleep_ms(10);
       } 
       for (i=180; i>0; i--){
        set_angle(i);
        sleep_ms(10);
       }
    //   sleep_ms(6000);
    //   printf("Enter PWM: ");
    //   int pwm;
    //   scanf("%d", &pwm);
    //   set_angle(pwm);
    } 
}


void set_angle(int angle){
    float pwm = 1250+(angle/180.0)*(6950-1250); //calculates duty cycle for given angle
    pwm_set_gpio_level(PWMPIN, pwm);
}