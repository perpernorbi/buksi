#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int cgiLed(HttpdConnData *connData);
int cgiDrive(HttpdConnData *connData);
int tplIndex(HttpdConnData *connData, char *token, void **arg);
void onOffDrive(char d);
void sendDirectVelocity(int left, int right);


#endif
