/*
Some random cgi routines. Used in the LED example and the page that returns the entire
flash as a binary. Also handles the hit counter on the main page.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */


#include <esp8266.h>
#include "cgi.h"
#include "io.h"
#include "json.h"
#define MOTOR_SHIELD

static const char speed = 0x09;

void ICACHE_FLASH_ATTR sendVelocity(char left, char right)
{
    char startByte = 0x80;
    char crc = left + right + startByte;
    os_printf("%c%c%c%c", startByte, left, right, crc);
}

void ICACHE_FLASH_ATTR sendDirectVelocity(int left, int right)
{
#ifdef MOTOR_SHIELD
    os_printf("\nleft %d", left);
    if (left > 10) {
        gpio_left_forward();
        os_printf("\nleft forward\n");
    }
    else if (left > -10) {
        gpio_left_stop();
        os_printf("\nleft stop\n");
    }
    else {
        gpio_left_backward();
        os_printf("\nleft backward\n");
    }

    if (right > 10)
        gpio_right_forward();
    else if (right > -10)
        gpio_right_stop();
    else
        gpio_right_backward();

#else
    char startByte = 0x82;
    char crc = left + right + startByte;
    os_printf("%c%c%c%c", startByte, (char)left, (char)right, crc);
#endif
}

void ICACHE_FLASH_ATTR onOffDrive(char d)
{
    switch (d) {
    case 'f':
        sendVelocity(+speed, +speed);
        break;
    case 'b':
        sendVelocity(-speed, -speed);
        break;
    case 'l':
        sendVelocity(+speed, -speed);
        break;
    case 'r':
        sendVelocity(-speed, +speed);
        break;
    case 's':
        sendVelocity(0, 0);
        break;
    }
}

int ICACHE_FLASH_ATTR cgiDrive(HttpdConnData *connData) {
    int len;
    char buff[1024];

    if (connData->conn==NULL) {
        //Connection aborted. Clean up.
        return HTTPD_CGI_DONE;
    }

    if (connData->requestType == HTTPD_METHOD_POST) {
        len=httpdFindArg(connData->post->buff, "led", buff, sizeof(buff));
        if (len!=0)
            onOffDrive(buff[0]);
    }

    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "application/json; charset=utf-8");
    httpdEndHeaders(connData);

    buff[0] = '{';
    buff[1] = '}';
    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;

}

//Cgi that turns the LED on or off according to the 'led' param in the POST data
int ICACHE_FLASH_ATTR cgiLed(HttpdConnData *connData) {
    int len;
    char buff[1024];

    if (connData->conn==NULL) {
        //Connection aborted. Clean up.
        return HTTPD_CGI_DONE;
    }

    if (connData->requestType == HTTPD_METHOD_POST) {
        len=httpdFindArg(connData->post->buff, "led", buff, sizeof(buff));
        if (len!=0) {
            switch (buff[0]) {
            case 't':
                ioLedToggle();
                break;
            case '0':
                ioLed(0);
                break;
            case '1':
                ioLed(1);
                break;
            }
        }
    }

    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "application/json; charset=utf-8");
    httpdEndHeaders(connData);

    buff[0] = '\0';
    JSONBeginObject(buff);
    if (ioGetLed())
        JSONAddKeyValuePairStr(buff,"led","on");
    else
        JSONAddKeyValuePairStr(buff,"led","off");
    JSONEndObject(buff);
    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;
}



//Template code for the led page.
int ICACHE_FLASH_ATTR tplIndex(HttpdConnData *connData, char *token, void **arg) {
    char buff[128];
    if (token==NULL) return HTTPD_CGI_DONE;

    os_strcpy(buff, "Unknown");

    if (os_strcmp(token, "devicename") == 0) {
        os_strcpy(buff, "Buksi");
    }
    if (os_strcmp(token, "ledstate")==0) {
        if (ioGetLed()) {
            os_strcpy(buff, "on");
        } else {
            os_strcpy(buff, "off");
        }
    }
    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;
}
