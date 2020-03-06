#ifndef REDIS_HOME__H
#define REDIS_HOME__H

#include <hiredis.h>
#include <string>
#include <vector>
#include <stdio.h>

#ifdef _WIN32
	#include <win32.h>
#endif

typedef struct redis_host
{
	redis_host()
		:port(-1)
	{}
	std::string ip;
	int port;//主端口
	std::string hash;
}RedisHost;

enum SlotState
{
	snull = 0,
	sbegin,
	sendt,
	sip1,
	sip2,
	sport1,
	sport2,
	shash1,
	shash2
};

typedef struct cluster_slots
{
	cluster_slots()
		:begin(-1),
		end(-1)
	{}
	int begin;
	int end;
	RedisHost master;
	RedisHost slave;
	void print()
	{
		printf("%d\n%d\n", begin, end);
		printf("master: %s, %d, %s\n", master.ip.c_str(), master.port, master.hash.c_str());
		printf("slave: %s, %d, %s\n", slave.ip.c_str(), slave.port, slave.hash.c_str());
	}
}ClusterSlot;

/*
	当前版本只支持一主一从的集群，不支持一主多从

*/
class RedisHome 
{
public:
	RedisHome();
	~RedisHome();

	redisContext* connect(const char* ipstr, int port, struct timeval tval);
	redisReply* command(const char *command);

	bool isError();
	const char* getErrorString();
	int getError();
	std::string getString();
	long long getInteger();

	bool next(std::string& key, std::string& val);
	bool next(ClusterSlot& slot);

private:
	void clearAll();
	bool reconnect();
	void getArrayString();
private:
	redisContext* _ctx;
	redisReply* _repl;

	char _last_cmd[512];
	std::string _last_status;
	long long _last_int;
	int _last_type;
	std::vector<std::string> _str_result;
	int _str_pos;

	int _idx;
	SlotState _sstate;

	std::string _ipstr;
	int _port;
	timeval _tval;
	
};

#endif