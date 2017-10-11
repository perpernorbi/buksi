#ifndef IO_H
#define IO_H

void ICACHE_FLASH_ATTR ioLed(int ena);
void ICACHE_FLASH_ATTR ioLedToggle();
uint8_t ICACHE_FLASH_ATTR ioGetLed();
void ICACHE_FLASH_ATTR ioInit(void);
void ICACHE_FLASH_ATTR ioLedChangeHandler(void (*f)(void));
void ICACHE_FLASH_ATTR io_set_pwm(int left, int right);

#endif
