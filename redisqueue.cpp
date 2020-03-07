#include "redisqueue.h"
#include "rediscluster.h"

RedisQueue* const RedisQueue::_instance = new RedisQueue();

RedisQueue* RedisQueue::getInstance()
{
	return _instance;
}

RedisQueue::RedisQueue()
	:_th(std::shared_ptr<std::thread>()),
	_cluster(NULL),
	_isStop(true)
{}

void RedisQueue::init(const char* ipstr, int port, const char* auth)
{
	_cluster = new RedisCluster();
	bool flag = _cluster->connect(ipstr, port, auth);
	if (!flag)
	{
		printf("connect to cluster error\n");
	}
}

void RedisQueue::post(const char* command)
{
	std::string cmd(command);
	_t_queue.push_back(cmd);
}

RedisHome* RedisQueue::get(const char* command)
{
	_mutex.lock();
	RedisHome* redis = _cluster->command(command);
	_mutex.unlock();
	if (redis->isError())
	{
		printf("CLUSTER COMMAND ERROR:%d, %s\n", redis->getError(), redis->getErrorString());
	}
	return redis;
}

void RedisQueue::startAsync()
{
	if (_th)
	{
		return;
	}
	_isStop = false;
	_th = std::make_shared<std::thread>(&RedisQueue::_loop, this);
}

void RedisQueue::stop()
{
	_isStop = true;
	printf("wait for RedisQueue loop\n");
	if (_th)
	{
		_th->join();
	}
}

void RedisQueue::_loop()
{
	printf("start RedisQueue loop\n");
	std::string cmd;
	bool flag;
	while (!_isStop)
	{
		cmd = _t_queue.pop(flag, std::chrono::milliseconds(1000));
		if (flag)
		{
			get(cmd.c_str());
		}
	}
	printf("end RedisQueue loop\n");
}

RedisQueue::~RedisQueue()
{
	if (_cluster)
	{
		delete _cluster;
	}

}