#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int cgiLed(HttpdConnData *connData);
int cgiLogin(HttpdConnData *connData);
int cgiDrive(HttpdConnData *connData);
int tplRobotParams(HttpdConnData *connData, char *token, void **arg);
bool isItTheLatestLoggedInClient(HttpdConnData *connData);
void onOffDrive(char d);
void sendDirectVelocity(int left, int right);


#endif
