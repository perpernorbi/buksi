#include <esp8266.h>


char* ICACHE_FLASH_ATTR JSONBeginObject(char* buffer)
{
    return strcat(buffer, "{");
}

char* ICACHE_FLASH_ATTR JSONEndObject(char* buffer)
{
    return strcat(buffer, "}");
}

char* ICACHE_FLASH_ATTR JSONAddKey(char* buffer, const char* key)
{
    size_t length = strlen(buffer);
    if ((length > 0) && (buffer[length-1] == '"'))
        length += sprintf(buffer + length, ",");
    sprintf(buffer + length, "\"%s\":", key);
    return buffer;
}

char* ICACHE_FLASH_ATTR JSONAddKeyValuePairStr(char* buffer, const char* key, const char* value)
{
    JSONAddKey(buffer, key);
    size_t length = strlen(buffer);
    sprintf(buffer + length, "\"%s\"", value);
    return buffer;
}

char* ICACHE_FLASH_ATTR JSONAddKeyValuePairIpAddr(char* buffer, const char* key, struct ip_addr ip)
{
    union ipaddr {
        uint32_t addr;
        uint8_t a[4];
    } ipu;
    ipu.addr=ip.addr;
    JSONAddKey(buffer, key);
    size_t length = strlen(buffer);
    sprintf(buffer + length, "\"%u.%u.%u.%u\"", ipu.a[0], ipu.a[1], ipu.a[2], ipu.a[3]);
    return buffer;
}
