/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

/*
This is example code for the esphttpd library. It's a small-ish demo showing off
the server, including WiFi connection management capabilities, some IO and
some pictures of cats.
*/

#include <esp8266.h>
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "cgiflash.h"
#include "stdout.h"
#include "auth.h"
#include "espfs.h"
#include "captdns.h"
#include "webpages-espfs.h"
#include "cgiwebsocket.h"
#include "cgi-test.h"
#include "json.h"
#include "jsonparse/jsonparse.h"
#include <user_interface.h>
#include "robot.h"

//The example can print out the heap use every 3 seconds. You can use this to catch memory leaks.
//#define SHOW_HEAP_USE

//Function that tells the authentication system what users/passwords live on the system.
//This is disabled in the default build; if you want to try it, enable the authBasic line in
//the builtInUrls below.
int myPassFn(HttpdConnData *connData, int no, char *user, int userLen, char *pass, int passLen) {
    if (no==0) {
        os_strcpy(user, "admin");
        os_strcpy(pass, "s3cr3t");
        return 1;
//Add more users this way. Check against incrementing no for each user added.
//	} else if (no==1) {
//		os_strcpy(user, "user1");
//		os_strcpy(pass, "something");
//		return 1;
    }
    return 0;
}

int ICACHE_FLASH_ATTR cgiWiFiStatus (HttpdConnData *connData)
{
    os_printf("TurnLedOn\x81\x40\x80");

    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/json");
    httpdEndHeaders(connData);

    char buff[1024];
    buff[0] = 0;
    JSONBeginObject(buff);
    switch (wifi_get_opmode()) {
    case 1:
        JSONAddKeyValuePairStr(buff, "WiFiMode", "Client");
        break;
    case 2:
        JSONAddKeyValuePairStr(buff, "WiFiMode", "SoftAP");
        break;
    case 3:
        JSONAddKeyValuePairStr(buff, "WiFiMode", "STA+AP");
        break;
    }

    struct station_config stconf;
    wifi_station_get_config(&stconf);

    JSONAddKeyValuePairStr(buff, "currSsid", (const char*)stconf.ssid);

    struct ip_info info;
    wifi_get_ip_info(0, &info);
    JSONAddKey(buff, "network");
    JSONBeginObject(buff);
    JSONAddKeyValuePairIpAddr(buff, "ip", info.ip);
    JSONAddKeyValuePairIpAddr(buff, "gw", info.gw);
    JSONAddKeyValuePairIpAddr(buff, "netmask", info.netmask);
    JSONEndObject(buff);

    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR jsonparse_assert_next(struct jsonparse_state *state, int expected_json_type)
{
    int type;
    type = jsonparse_next(state);

    if (type != expected_json_type) {
        os_printf("Expected json type %c 0x%x",
                  expected_json_type, expected_json_type);
        os_printf(", got %c 0x%x\n", type, type);
        state->error = JSON_ERROR_SYNTAX;
    }
    return type;
}


/*
{
    "drive_mode": "direct",
    "velocities": [10, 10]
}
*/
void ICACHE_FLASH_ATTR wsDriveRecv(Websock *ws, char *data, int len, int flags) {
    struct jsonparse_state json_state;
    int retval = 0;
    int direct_drive = 0;
    int left, right;
    if (strncmp(data, "toggle_lights", strlen("toggle_lights")) == 0) {
        os_printf("toggle\n");
        ioLedToggle();
        os_printf("toggle\n");
        return;
    }
    jsonparse_setup(&json_state, data, len);
    while ((retval = jsonparse_next(&json_state)) != JSON_TYPE_ERROR) {
        if (retval == JSON_TYPE_PAIR_NAME) {
            if (jsonparse_strcmp_value(&json_state, "drive_mode") == 0) {
                jsonparse_assert_next(&json_state, JSON_TYPE_STRING);
                direct_drive = ! jsonparse_strcmp_value(&json_state, "direct");
            } else if (jsonparse_strcmp_value(&json_state, "velocities") == 0) {
                jsonparse_assert_next(&json_state, JSON_TYPE_ARRAY);
                jsonparse_assert_next(&json_state, JSON_TYPE_NUMBER);
                left = jsonparse_get_value_as_int(&json_state);
                jsonparse_assert_next(&json_state, ',');
                jsonparse_assert_next(&json_state, JSON_TYPE_NUMBER);
                right = jsonparse_get_value_as_int(&json_state);
                jsonparse_assert_next(&json_state, ']');
            }
        }
    }

    if (direct_drive) {
        os_printf("drive %d, %d", left, right);
        sendDirectVelocity(left, right);
    }

    //onOffDrive(data[0]);
    //cgiWebsocketSend(ws, buff, strlen(buff), WEBSOCK_FLAG_NONE);
}

void ICACHE_FLASH_ATTR wsDriveConnect(Websock *ws) {
    ws->recvCb=wsDriveRecv;
//    sendLedStatus();
}

#ifdef ESPFS_POS
CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_ESPFS,
    .fw1Pos=ESPFS_POS,
    .fw2Pos=0,
    .fwSize=ESPFS_SIZE,
};
#define INCLUDE_FLASH_FNS
#endif
#ifdef OTA_FLASH_SIZE_K
CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_FW,
    .fw1Pos=0x1000,
    .fw2Pos=((OTA_FLASH_SIZE_K*1024)/2)+0x1000,
    .fwSize=((OTA_FLASH_SIZE_K*1024)/2)-0x1000,
    .tagName=OTA_TAGNAME
};
#define INCLUDE_FLASH_FNS
#endif

/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
    {"*", cgiRedirectApClientToHostname, "esp8266.nonet"},
    {"/", cgiRedirect, "/index.html"},
    {"/index.html", cgiEspFsTemplate, tplRobotParams},
    {"/led.cgi", cgiLed, NULL},
    {"/drive.cgi", cgiDrive, NULL},
#ifdef INCLUDE_FLASH_FNS
    {"/flash/next", cgiGetFirmwareNext, &uploadParams},
    {"/flash/upload", cgiUploadFirmware, &uploadParams},
#endif
//    {"/flash/reboot", cgiRebootFirmware, NULL},

    //Routines to make the /wifi URL and everything beneath it work.

//Enable the line below to protect the WiFi configuration with an username/password combo.
//	{"/wifi/*", authBasic, myPassFn},

//    {"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
//    {"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
//    {"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
//    {"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
//    {"/wifi/connect.cgi", cgiWiFiConnect, NULL},
//    {"/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
//    {"/wifi/wifistatus.cgi", cgiWiFiStatus, NULL},
//    {"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

    {"/drive-ws.cgi", cgiWebsocket, wsDriveConnect},

//    {"/test", cgiRedirect, "/test/index.html"},
//    {"/test/", cgiRedirect, "/test/index.html"},
//    {"/test/test.cgi", cgiTestbed, NULL},

    {"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
    {NULL, NULL, NULL}
};


#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;

static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
    os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
}
#endif

void ICACHE_FLASH_ATTR setAccessPoint()
{
    if (wifi_get_opmode() != SOFTAP_MODE) {
        wifi_set_opmode(SOFTAP_MODE);
        wifi_set_opmode_current(SOFTAP_MODE);
        //system_restart();
    }
    struct softap_config c;
    if (wifi_softap_get_config(&c)) {
        os_memset(c.ssid, 0, 32);
        os_memset(c.password, 0, 64);
        os_memcpy(c.ssid, ROBOTNAME, 7);
        os_memcpy(c.password, ROBOTWPA2PSK, 8);
        c.ssid_len = strlen(ROBOTNAME);
        c.authmode = AUTH_WPA2_PSK;
        wifi_softap_set_config(&c);
    }
}

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {
    stdoutInit();

    os_printf("I have been here\n");

    setAccessPoint();
    ioInit();
    captdnsInit();

    // 0x40200000 is the base address for spi flash memory mapping, ESPFS_POS is the position
    // where image is written in flash that is defined in Makefile.
#ifdef ESPFS_POS
    espFsInit((void*)(0x40200000 + ESPFS_POS));
#else
    espFsInit((void*)(webpages_espfs_start));
#endif
    httpdInit(builtInUrls, 80);
#ifdef SHOW_HEAP_USE
    //Norbi
    //os_timer_disarm(&prHeapTimer);
    //os_timer_setfn(&prHeapTimer, prHeapTimerCb, NULL);
    //os_timer_arm(&prHeapTimer, 3000, 1);
#endif
    //Norbi
    //os_timer_disarm(&websockTimer);
    //os_timer_setfn(&websockTimer, websockTimerCb, NULL);
    //os_timer_arm(&websockTimer, 1000, 1);
    os_printf("\nReady\n");
    os_printf("\x81\x40\x80\x41");
}

void user_rf_pre_init() {
    //Not needed, but some SDK versions want this defined.
}
