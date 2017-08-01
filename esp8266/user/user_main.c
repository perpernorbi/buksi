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


void wsDriveRecv(Websock *ws, char *data, int len, int flags) {
    char buff[128];
    len = (len < 128) ? len : 128;
    size_t i;
    for (i=0; (i < len) && (i < 128); ++i) buff[i] = data[i];
    buff[i] = '\0';
    os_printf("Received in websocket %s\n", buff);
    if (len > 0)
    onOffDrive(data[0]);
    //cgiWebsocketSend(ws, buff, strlen(buff), WEBSOCK_FLAG_NONE);
}

void wsDriveConnect(Websock *ws) {
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
    {"/", cgiRedirect, "/index.tpl"},
    {"/index.tpl", cgiEspFsTemplate, tplIndex},
    {"/led.cgi", cgiLed, NULL},
    {"/drive.cgi", cgiDrive, NULL},
#ifdef INCLUDE_FLASH_FNS
    {"/flash/next", cgiGetFirmwareNext, &uploadParams},
    {"/flash/upload", cgiUploadFirmware, &uploadParams},
#endif
    {"/flash/reboot", cgiRebootFirmware, NULL},

    //Routines to make the /wifi URL and everything beneath it work.

//Enable the line below to protect the WiFi configuration with an username/password combo.
//	{"/wifi/*", authBasic, myPassFn},

    {"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
    {"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
    {"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
    {"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
    {"/wifi/connect.cgi", cgiWiFiConnect, NULL},
    {"/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
    {"/wifi/wifistatus.cgi", cgiWiFiStatus, NULL},
    {"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

    {"/drive-ws.cgi", cgiWebsocket, wsDriveConnect},

    {"/test", cgiRedirect, "/test/index.html"},
    {"/test/", cgiRedirect, "/test/index.html"},
    {"/test/test.cgi", cgiTestbed, NULL},

    {"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
    {NULL, NULL, NULL}
};


#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;

static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
    os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
}
#endif

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {
    stdoutInit();
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
