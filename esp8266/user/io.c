
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */


#include <esp8266.h>
#include <pwm.h>

#define MOTOR_SHIELD
#define LEDGPIO FUNC_GPIO2
#define BTNGPIO FUNC_GPIO0

#ifdef MOTOR_SHIELD
#define PWM_A_OUT_IO_MUX PERIPHS_IO_MUX_GPIO5_U
#define PWM_A_OUT_IO_NUM 5
#define PWM_A_OUT_IO_FUNC  FUNC_GPIO5

#define PWM_B_OUT_IO_MUX PERIPHS_IO_MUX_GPIO4_U
#define PWM_B_OUT_IO_NUM 4
#define PWM_B_OUT_IO_FUNC  FUNC_GPIO4

#define PWM_CHANNEL_COUNT 2
#else
static ETSTimer resetBtntimer;
#endif

static uint8_t ledState = 0;

static void (*statusChangeHandler)(void);
void ICACHE_FLASH_ATTR ioLedChangeHandler(void (*f)(void))
{
    statusChangeHandler = f;
}

void ICACHE_FLASH_ATTR ioLed(int ena) {
#ifndef MOTOR_SHIELD
    //gpio_output_set is overkill. ToDo: use better mactos
    if (ena) {
        gpio_output_set((1<<LEDGPIO), 0, (1<<LEDGPIO), 0);
        ledState = 1;
    } else {
        gpio_output_set(0, (1<<LEDGPIO), (1<<LEDGPIO), 0);
        ledState = 0;
    }
    statusChangeHandler();
#endif
}

void ICACHE_FLASH_ATTR io_set_pwm(int left, int right)
{
    if (left > 0)
        gpio_output_set(0, 1<<0, 0, 0);
    else
        gpio_output_set(1<<0, 0, 0, 0);
    left = (left >= 0) ? left : -left;
    left <<= 15;
    pwm_set_duty(left, 1);

    if (right > 0)
        gpio_output_set(0, 1<<2, 0, 0);
    else
        gpio_output_set(1<<2, 0, 0, 0);

    right = (right >= 0) ? right : -right;
    right <<= 15;
    pwm_set_duty(right, 0);
    pwm_start();

}

uint8_t ICACHE_FLASH_ATTR ioGetLed()
{
    return ledState;
}

void ICACHE_FLASH_ATTR ioLedToggle()
{
    ioLed((ledState)?0:1);
}

#ifndef MOTOR_SHIELD
static void ICACHE_FLASH_ATTR resetBtnTimerCb(void *arg) {
    static int resetCnt=0;
    if (!GPIO_INPUT_GET(BTNGPIO)) {
        resetCnt++;
    } else {
        if (resetCnt>=30) { //3 sec pressed
            wifi_station_disconnect();
            wifi_set_opmode(0x3); //reset to AP+STA mode
            os_printf("Reset to AP mode. Restarting system...\n");
            system_restart();
        }
        resetCnt=0;
    }
    if (resetCnt == 2) {
        ioLedToggle();
    }
}
#endif

#ifdef MOTOR_SHIELD



void ICACHE_FLASH_ATTR pwmInit()
{
    uint32 io_info[][3] = {
        {PWM_A_OUT_IO_MUX,PWM_A_OUT_IO_FUNC,PWM_A_OUT_IO_NUM},
        {PWM_B_OUT_IO_MUX,PWM_B_OUT_IO_FUNC,PWM_B_OUT_IO_NUM}
    };

    uint32 pwm_duty_init[2] = {0, 0};
    pwm_init(47186,  pwm_duty_init, 2, io_info);

    pwm_start();
}

void ICACHE_FLASH_ATTR ioInit() {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    gpio_output_set(0, 0, (1<<0) | (1<<2), 0);
    //os_timer_disarm(&resetBtntimer);
    //os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
    //os_timer_arm(&resetBtntimer, 100, 1);
    pwmInit();
}


#else
void ICACHE_FLASH_ATTR ioInit() {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    gpio_output_set(0, 0, (1<<LEDGPIO), (1<<BTNGPIO));
    os_timer_disarm(&resetBtntimer);
    os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
    os_timer_arm(&resetBtntimer, 100, 1);
}
#endif
