#ifndef REDIS_CLUSTER__H
#define REDIS_CLUSTER__H

#include <stdio.h>
#include "redishome.h"
#include <vector>
#include <string>
#include <map>

class RedisCluster
{
public:
	RedisCluster();
	~RedisCluster();
	bool connect(const char* ipstr, int port, const char* auth = NULL);

	RedisHome* command(const char* command);
	redisReply* getLastReply();

private:
	RedisHome* getContext(unsigned int hashslot);
	RedisHome* getContext(const char* ipstr, int port);
	RedisHome* getContext(redisReply*);
	bool isRedirect(redisReply*);

	RedisHome* doConnect(const char* ipstr, int port, const char* auth);

private:
	std::map<std::string, RedisHome*> _ctxs;
	std::vector<ClusterSlot> _slots;
	redisReply* _repl;
};


#endif