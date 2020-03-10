#ifndef REDIS_SINGLE__H
#define REDIS_SINGLE__H

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "threadqueue.hpp"
#include <mutex>

class RedisHome;

class RedisSingle
{
public:
	RedisSingle();
	~RedisSingle();
	static RedisSingle* const _instance;

	static RedisSingle* getInstance();

	bool init(const char* ipstr, int port);
	void startAsync();

	void post(const char* command); //异步处理，可能会失败
	RedisHome* get(const char* command); //同步处理

	void stop();
	void _loop();

private:
	ThreadQueue<std::string> _t_queue;
	RedisHome* _single;
	std::shared_ptr<std::thread> _th;
	bool _isStop;
	std::mutex _mutex;

};

#endif