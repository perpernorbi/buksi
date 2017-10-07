#ifndef IO_H
#define IO_H

void ICACHE_FLASH_ATTR ioLed(int ena);
void ICACHE_FLASH_ATTR ioLedToggle();
uint8_t ICACHE_FLASH_ATTR ioGetLed();
void ioInit(void);
void ICACHE_FLASH_ATTR ioLedChangeHandler(void (*f)(void));

void gpio_left_forward();
void gpio_left_stop();
void gpio_left_backward();

void gpio_right_forward();
void gpio_right_stop();
void gpio_right_backward();
#endif
