#include <iostream>
#include <stdio.h>
#include <hiredis.h>
#include <win32.h>
#include <stdarg.h>
#include "redishome.h"
#include "util.h"
#include "rediscluster.h"
#include "threadqueue.hpp"
#include "redisqueue.h"

#pragma comment(lib, "ws2_32.lib")

//name main:version:random:server_id
//name ms:v20200108:stuxHUYB:s1:roomrtmp
//name ms:v20200108:stuxHUYB:s1:usertoken

void test_getKey()
{
	char* command1 = "GET too";
	char* command2 = "SET abdfd hoo";
	char* command3 = " GET fdfd ";
	char* command4 = "HGET ada ms:s1";
	char* command5 = "HSET rrr ms:s1 https://ttta1";
	char* command6 = "HGET tyu ms:s1 ";
	std::string key1 = Util::getKey(command1, strlen(command1));
	std::string key2 = Util::getKey(command2, strlen(command2));
	std::string key3 = Util::getKey(command3, strlen(command3));
	std::string key4 = Util::getKey(command4, strlen(command4));
	std::string key5 = Util::getKey(command5, strlen(command5));
	std::string key6 = Util::getKey(command6, strlen(command6));
}

void test_getContext(const char* str)
{
	char* temp = strstr((char*)str, "MOVED");
	if (temp == NULL)
	{
		return;
	}
	temp += 6;//pointer to hash slot of the key
	const char* ip_port = strstr(temp, " ");
	const char* sport = strstr(ip_port, ":");

	size_t ip_len = sport - ip_port - 1;//1ÊÇ¿Õ¸ñ
	std::string ipstr(ip_port + 1, ip_len);
	std::string port(sport + 1, strlen(ip_port) - ip_len - 1);
	int a = 1;
}

#define redis_svr (*RedisQueue::getInstance())

int main()
{
	RedisQueue::getInstance()->init("192.168.239.130", 7000);
	RedisQueue::getInstance()->startAsync();

	/*redis_svr.post("hset ms:s2 room1 http://abcd1");
	redis_svr.post("hset ms:s2 room2 http://abcd2");
	redis_svr.post("hset ms:s2 room3 http://abcd3");
	redis_svr.post("hset ms:s2 room4 http://abcd4");
	redis_svr.post("hset ms:s2 room5 http://abcd5");

	redis_svr.post("hset ms:s3 room1 http://abcd100");
	redis_svr.post("hset ms:s3 room2 http://abcd200");
	redis_svr.post("hset ms:s4 room3 http://abcd300");
	redis_svr.post("hset ms:s4 room4 http://abcd400");
	redis_svr.post("hset ms:s5 room5 http://abcd500");

	redis_svr.get("hset ms:s2 room11 http://alibaba_room11");
	redis_svr.get("hset ms:s2 room12 http://alibaba_room12");
	redis_svr.get("hset ms:s3 room13 http://alibaba_room13");*/

	RedisHome* redis = redis_svr.get("hget ms:s2 room1");
	std::cout << redis->getString() << std::endl;

	redis = redis_svr.get("hget ms:s2 room1");
	std::cout << redis->getString() << std::endl;

	redis = redis_svr.get("hget ms:s2 room2");
	std::cout << redis->getString() << std::endl;

	redis = redis_svr.get("hget ms:s3 room4");
	//This will get nil.
	std::cout << redis->getString() << std::endl;

	redis = redis_svr.get("hget ms:s2 room1");
	std::cout << redis->getString() << std::endl;

	redis = redis_svr.get("hget ms:s2 room11");
	std::cout << redis->getString() << std::endl;

	std::cin.get();

	return 0;
}

int main3()
{
	RedisCluster cluster;
	cluster.connect("192.168.239.130", 7000);

	RedisHome* redis = cluster.command("GET hello");

	std::string hel = redis->getString();

	cluster.command("GET tok");
	hel = redis->getString();

	redis = cluster.command("HGETALL ms:s1");
	std::string key;
	std::string val;
	while (redis && redis->next(key, val))
	{
		printf("key:%s, val:%s\n", key.c_str(), val.c_str());
	}

	int a = 2;

	std::cin.get();
	return 0;
}

int main2()
{
	//test_getKey();
	//test_getContext("MOVED 11071 192.168.239.130:7002");
	struct timeval tval;
	tval.tv_sec = 3;
	tval.tv_usec = 0;

	RedisHome* cli1 = new RedisHome;
	RedisHome* cli2 = new RedisHome;
	RedisHome* cli3 = new RedisHome;

	cli1->connect("192.168.239.130", 7000, tval);
	//cli2->connect("192.168.239.130", 7004, tval);
	//cli3->connect("192.168.239.130", 7005, tval);

	cli1->command("CLUSTER SLOTS");
	ClusterSlot slot;
	while (cli1->next(slot))
	{
		slot.print();
	}

	delete cli1;
	delete cli2;
	delete cli3;

	std::cin.get();
	return 0;
}

int main1()
{
	struct timeval tval;
	tval.tv_sec = 3;
	tval.tv_usec = 0;
	//redisContext* ctx = redisConnectWithTimeout("127.0.0.1", 6379, tval);
	redisContext* ctx = redisConnectWithTimeout("192.168.239.130", 7000, tval);
	if (ctx == NULL || ctx->err)
	{
		if (ctx)
		{
			printf("Error:%d, %s\n", ctx->err, ctx->errstr);
		}
		else
		{
			printf("connect redis server error\n");
		}
		std::cin.get();
		return 0;
	}
	printf("connect redis server successfully!\n");

	redisReply* reply;
	///1. STRING
	
	reply = (redisReply*)redisCommand(ctx, "get hel");
	if (reply)
    {
		printf("reply: %d: %s\n", reply->type, reply->str);
	}
	else
	{
		printf("reply is null.\n");
	}

	///2. HASH
	/*reply = (redisReply*)redisCommand(ctx, "HGETALL ms:v20200108:stuxHUYB:s1:roomrtmp");
	
	if (reply)
	{
		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (int i = 0; i < reply->elements; ++i)
			{
				printf("result: %d, %s\n", reply->element[i]->type, reply->element[i]->str);
			}
		}
		printf("reply: %d: %s\n", reply->type, reply->str);
	}
	else
	{
		printf("reply is null.\n");
	}*/

	freeReplyObject(reply);
	redisFree(ctx);
	std::cin.get();
	return 0;
}