# PWM_Motor_Controller

This repo showcases some of my early work with embedded programming. It is an implementation of a PWM motor controller which takes in input from a 16-key keypad to select the motor's speed. The header file shows some of the functinality of the software such as the avility to select the PWM fequency which does impact how well we can controll each of the speeds of the motor as show in the graph below:

![PWM freq vs Tachometer reading](https://github.com/vergi1iu5/PWM_Motor_Controller/blob/main/docs/Images/Duty%20Cycle.png)

This graph shows how each selected setting has a diffrent effect on the motor's speed (coming from a tachometer reading.) Finally, here are some pictures of what a PWM signal is incase the reader needs it. It is just a comfirmation that a pressed key selects the correct pwm duty cycle:


![60% duty cycle](https://github.com/vergi1iu5/PWM_Motor_Controller/blob/main/docs/Images/20p.PNG)

And a graph showing what duty cycle is expected and what I got after pressing each key.

![60% duty cycle](https://github.com/vergi1iu5/PWM_Motor_Controller/blob/main/docs/Images/PulseWidth.png)

Reading the reference manual and header file will give you more insisgt into the eact setup.
