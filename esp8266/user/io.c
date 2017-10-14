
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
static ETSTimer resetBtntimer;

#ifdef MOTOR_SHIELD
#define PWM_A_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO4_U
#define PWM_A_OUT_IO_NUM  4
#define PWM_A_OUT_IO_FUNC FUNC_GPIO4

#define PWM_B_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO5_U
#define PWM_B_OUT_IO_NUM  5
#define PWM_B_OUT_IO_FUNC FUNC_GPIO5

#define DIR_A_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO0_U
#define DIR_A_OUT_IO_BIT  BIT0
#define DIR_A_OUT_IO_FUNC FUNC_GPIO0

#define DIR_B_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO2_U
#define DIR_B_OUT_IO_BIT  BIT2
#define DIR_B_OUT_IO_FUNC FUNC_GPIO2

#define BTN_GPIO_MUX  PERIPHS_IO_MUX_MTDI_U
#define BTN_GPIO_BIT  BIT12
#define BTN_GPIO_FUNC FUNC_GPIO12

#define LED_GPIO_MUX  PERIPHS_IO_MUX_MTMS_U
#define LED_GPIO_BIT  BIT14
#define LED_GPIO_FUNC FUNC_GPIO14

#define PWM_CHANNEL_COUNT 2
#else
#define BTN_GPIO_MUX  PERIPHS_IO_MUX_GPIO0_U
#define BTN_GPIO_BIT  BIT0
#define BTN_GPIO_FUNC FUNC_GPIO0

#define LED_GPIO_MUX  PERIPHS_IO_MUX_GPIO2_U
#define LED_GPIO_BIT  BIT2
#define LED_GPIO_FUNC FUNC_GPIO2
#endif

static uint8_t ledState = 0;

static void (*statusChangeHandler)(void) = NULL;
void ICACHE_FLASH_ATTR ioLedChangeHandler(void (*f)(void))
{
    statusChangeHandler = f;
}

void ICACHE_FLASH_ATTR ioLed(int ena) {
    //gpio_output_set is overkill. ToDo: use better mactos
    if (ena) {
        gpio_output_set(LED_GPIO_BIT, 0, 0, 0);
        ledState = 1;
    } else {
        gpio_output_set(0, LED_GPIO_BIT, 0, 0);
        ledState = 0;
    }
    if (statusChangeHandler != NULL)
        statusChangeHandler();
}

void ICACHE_FLASH_ATTR io_set_pwm(int left, int right)
{
#ifdef MOTOR_SHIELD
    if (left > 0)
        gpio_output_set(0, DIR_A_OUT_IO_BIT, 0, 0);
    else
        gpio_output_set(DIR_A_OUT_IO_BIT, 0, 0, 0);
    left = (left >= 0) ? left : -left;
    left <<= 15;
    pwm_set_duty(left, 1);

    if (right > 0)
        gpio_output_set(0, DIR_B_OUT_IO_BIT, 0, 0);
    else
        gpio_output_set(DIR_B_OUT_IO_BIT, 0, 0, 0);

    right = (right >= 0) ? right : -right;
    right <<= 15;
    pwm_set_duty(right, 0);
    pwm_start();
#endif
}

uint8_t ICACHE_FLASH_ATTR ioGetLed()
{
    return ledState;
}

void ICACHE_FLASH_ATTR ioLedToggle()
{
    ioLed((ledState)?0:1);
}

static void ICACHE_FLASH_ATTR resetBtnTimerCb(void *arg) {
    static int resetCnt=0;
    if (!(gpio_input_get() & BTN_GPIO_BIT)) {
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




void ICACHE_FLASH_ATTR pwmInit()
{
#ifdef MOTOR_SHIELD
    PIN_FUNC_SELECT(DIR_A_OUT_IO_MUX, DIR_A_OUT_IO_FUNC);
    PIN_FUNC_SELECT(DIR_B_OUT_IO_MUX, DIR_B_OUT_IO_FUNC);
    gpio_output_set(0, 0, DIR_A_OUT_IO_BIT | DIR_B_OUT_IO_BIT, 0);
    uint32 io_info[][3] = {
        {PWM_A_OUT_IO_MUX,PWM_A_OUT_IO_FUNC,PWM_A_OUT_IO_NUM},
        {PWM_B_OUT_IO_MUX,PWM_B_OUT_IO_FUNC,PWM_B_OUT_IO_NUM}
    };
    uint32 pwm_duty_init[2] = {0, 0};
    pwm_init(47186,  pwm_duty_init, 2, io_info);
    pwm_start();
#endif

}

void ICACHE_FLASH_ATTR ioInit() {
    PIN_FUNC_SELECT(LED_GPIO_MUX, LED_GPIO_FUNC);
    PIN_FUNC_SELECT(BTN_GPIO_MUX, BTN_GPIO_FUNC);
    gpio_output_set(0, 0, LED_GPIO_BIT, BTN_GPIO_BIT);
    os_timer_disarm(&resetBtntimer);
    os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
    os_timer_arm(&resetBtntimer, 100, 1);
    pwmInit();
}
