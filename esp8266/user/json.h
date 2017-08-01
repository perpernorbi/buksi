#ifndef JSON_H
#define JSON_H

char* ICACHE_FLASH_ATTR JSONBeginObject(char* buffer);
char* ICACHE_FLASH_ATTR JSONEndObject(char* buffer);
char* ICACHE_FLASH_ATTR JSONAddKey(char* buffer, const char* key);
char* ICACHE_FLASH_ATTR JSONAddKeyValuePairStr(char* buffer, const char* key, const char* value);
char* ICACHE_FLASH_ATTR JSONAddKeyValuePairIpAddr(char* buffer, const char* key, struct ip_addr ip);


#endif // JSON_H
