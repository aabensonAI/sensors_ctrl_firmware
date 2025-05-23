#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void blink(int pin);
void motor_task(void);
void init_gpio_and_pwm();

#ifdef __cplusplus
}
#endif
