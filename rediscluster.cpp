#include "rediscluster.h"
#include "util.h"

RedisCluster::RedisCluster()
	:_repl(NULL)
{}

bool RedisCluster::connect(const char* ipstr, int port, const char* auth)
{
	RedisHome* inst = doConnect(ipstr, port, auth);
	if (!inst)
	{
		return false;
	}
	redisReply* reply = NULL;
	reply = inst->command("CLUSTER SLOTS");
	if (!reply)
	{
		return false;
	}
	//依次连接集群中的各个服务器
	ClusterSlot single;
	RedisHome* instance = NULL;
	while (inst->next(single))
	{
		_slots.push_back(single);

		if (single.master.port == port && strcmp(single.master.ip.c_str(), ipstr) == 0)
		{
			//如果是传进来的实例
			_ctxs[single.master.hash] = inst;
		}
		else
		{
			instance = doConnect(single.master.ip.c_str(), single.master.port, auth);
			if (!instance)
			{
				return false;
			}
			_ctxs[single.master.hash] = instance;
		}
		if (single.slave.port == port && strcmp(single.slave.ip.c_str(), ipstr) == 0)
		{
			//如果是传进来的实例
			_ctxs[single.slave.hash] = inst;
		}
		else
		{
			instance = doConnect(single.slave.ip.c_str(), single.slave.port, auth);
			if (!instance)
			{
				return false;
			}
			_ctxs[single.slave.hash] = instance;
		}
	}
	
	return true;
}

RedisHome* RedisCluster::doConnect(const char* ipstr, int port, const char* auth)
{
	RedisHome* inst = new RedisHome();

	struct timeval tval;
	tval.tv_sec = 3;
	tval.tv_usec = 0;

	redisContext* ctx = inst->connect(ipstr, port, tval);
	if (!ctx)
	{
		return NULL;
	}
	
	if (auth)
	{
		std::string sauth = "AUTH ";
		sauth.append(auth);
		redisReply* reply = inst->command(sauth.c_str());
		if (!reply)
		{
			return NULL;
		}
		if (reply->type == REDIS_REPLY_ERROR)
		{
			printf("AUTH ERROR:%s\n", reply->str);
			return NULL;
		}
	}
	return inst;
}

redisReply* RedisCluster::getLastReply()
{
	return _repl;
}

RedisHome* RedisCluster::command(const char* command)
{
	//1. get key
	//2. calc hash value
	//3. select the destination host
	//4. process result
	std::string key = Util::getKey(command, strlen(command));
	unsigned int hashslot = Util::clusterManagerKeyHashSlot(key.c_str(), key.size());
	//找到数据存储所在实例
	RedisHome* inst = getContext(hashslot);
	if (!inst)
	{
		return NULL;
	}
	redisReply* reply = inst->command(command);
	if (isRedirect(reply))
	{
		//仅重定向一次
		inst = getContext(reply);
		if (inst)
		{
			reply = inst->command(command);
		}
	}
	_repl = reply;
	return inst;
}

bool RedisCluster::isRedirect(redisReply* reply)
{
	if (!reply || reply->type != REDIS_REPLY_ERROR);
	{
		return false;
	}
	if (strstr(reply->str, "MOVED") != NULL)
	{
		return true;
	}
	return false;
}

RedisHome* RedisCluster::getContext(redisReply* reply)
{
	char* temp = strstr(reply->str, "MOVED");
	if (temp == NULL)
	{
		return NULL;
	}
	temp += 6;//pointer to hash slot of the key
	const char* ip_port = strstr(temp, " ");
	const char* sport = strstr(ip_port, ":");

	size_t ip_len = sport - ip_port - 1;//1是空格
	std::string ipstr(ip_port + 1, ip_len);
	std::string port(sport + 1, strlen(ip_port) - ip_len - 1);

	return getContext(ipstr.c_str(), std::atoi(port.c_str()));
}

RedisHome* RedisCluster::getContext(unsigned int hashslot)
{
	std::string hash;
	for (int i = 0; i < _slots.size(); ++i)
	{
		if (hashslot >= _slots[i].begin && hashslot <= _slots[i].end)
		{
			hash = _slots[i].master.hash;
			break;
		}
	}
	if (_ctxs.find(hash) != _ctxs.end())
	{
		return _ctxs[hash];
	}
	return NULL;
}

RedisHome* RedisCluster::getContext(const char* ipstr, int port)
{
	std::string hash;
	for (int i = 0; i < _slots.size(); ++i)
	{
		if (strcmp(_slots[i].master.ip.c_str(), ipstr) == 0 && _slots[i].master.port == port)
		{
			hash = _slots[i].master.hash;
			break;
		}
		if (strcmp(_slots[i].slave.ip.c_str(), ipstr) == 0 && _slots[i].slave.port == port)
		{
			hash = _slots[i].slave.hash;
			break;
		}
	}
	if (_ctxs.find(hash) != _ctxs.end())
	{
		return _ctxs[hash];
	}
	return NULL;
}

RedisCluster::~RedisCluster()
{
	for (auto& unit : _ctxs)
	{
		if (unit.second)
		{
			delete unit.second;
		}
	}
}