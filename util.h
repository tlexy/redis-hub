#ifndef REDIS_UTIL__H
#define REDIS_UTIL__H

#include <stdio.h>
#include <stdint.h>
#include <string>

class Util
{
public:
	static uint16_t crc16(const char *buf, int len);
	static unsigned int clusterManagerKeyHashSlot(const char *key, int keylen);
	static std::string getKey(const char* command, size_t len);
};


#endif