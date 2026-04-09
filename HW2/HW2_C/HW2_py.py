import time
import board
import pwmio # get access to PWM

servo = pwmio.PWMOut(board.GP18, variable_frequency=True)
servo.frequency = 50
servo.duty_cycle = 0 # initially off, at 16bit number so max on is 65535

max = (int)(65535*(6300/50000))
min = (int)(65535*(1200/50000))

while True:
    # start duty cycle at 0, every loop increase by 100 
    # until getting to the max of 65535
    for i in range(min, max, 4):
        servo.duty_cycle = i
        time.sleep(0.001)
    for i in range(max, min, -4):
        servo.duty_cycle = i
        time.sleep(0.001)