#include "redissingle.h"
#include "redishome.h"

RedisSingle* const RedisSingle::_instance = new RedisSingle();

RedisSingle* RedisSingle::getInstance()
{
	return _instance;
}

RedisSingle::RedisSingle()
	:_th(std::shared_ptr<std::thread>()),
	_single(NULL),
	_isStop(true)
{}

bool RedisSingle::init(const char* ipstr, int port)
{
	_single = new RedisHome();
	struct timeval tval;
	tval.tv_sec = 15;
	tval.tv_usec = 0;
	redisContext* ctx = _single->connect(ipstr, port, tval);
	if (!ctx || ctx->err)
	{
		printf("connect to redis error\n");
		return false;
	}
	return true;
}

void RedisSingle::post(const char* command)
{
	std::string cmd(command);
	_t_queue.push_back(cmd);
}

RedisHome* RedisSingle::get(const char* command)
{
	_mutex.lock();
	_single->command(command);
	_mutex.unlock();
	if (_single->isError())
	{
		printf("CLUSTER COMMAND ERROR:%d, %s\n", _single->getError(), _single->getErrorString());
	}
	return _single;
}

void RedisSingle::startAsync()
{
	if (_th)
	{
		return;
	}
	_isStop = false;
	_th = std::make_shared<std::thread>(&RedisSingle::_loop, this);
}

void RedisSingle::stop()
{
	_isStop = true;
	printf("wait for RedisQueue loop\n");
	if (_th)
	{
		_th->join();
	}
}

void RedisSingle::_loop()
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

RedisSingle::~RedisSingle()
{
	if (_single)
	{
		delete _single;
	}

}