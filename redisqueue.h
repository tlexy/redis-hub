#ifndef REDIS_QUEUE__H
#define REDIS_QUEUE__H

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "threadqueue.hpp"
#include <mutex>

class RedisHome;
class RedisCluster;


class RedisQueue
{
public:
	RedisQueue();
	~RedisQueue();
	static RedisQueue* const _instance;

	static RedisQueue* getInstance();

	void init(const char* ipstr, int port, const char* auth = NULL);
	void startAsync();

	void post(const char* command); //异步处理，可能会失败
	RedisHome* get(const char* command); //同步处理

	void stop();
	void _loop();

private:
	//std::vector<std::string> _c_queue;//待处理的指令队列
	ThreadQueue<std::string> _t_queue;
	RedisCluster* _cluster;
	std::shared_ptr<std::thread> _th;
	bool _isStop;
	std::mutex _mutex;
	//std::unique_lock<std::mutex> lock;

};

#endif