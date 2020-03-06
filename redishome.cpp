#include "redishome.h"
#include <stdio.h>

RedisHome::RedisHome()
{}

RedisHome::~RedisHome()
{
	if (_repl)
	{
		freeReplyObject(_repl);
	}
}

redisContext* RedisHome::connect(const char* ipstr, int port, struct timeval tval)
{
	_ipstr.append(ipstr);
	_port = port;
	_tval = tval;

	printf("connect to server: %s:%d\n", ipstr, port);
	_ctx = redisConnectWithTimeout(ipstr, port, tval);
	if (_ctx == NULL || _ctx->err)
	{
		if (_ctx)
		{
			printf("Error:%d, %s\n", _ctx->err, _ctx->errstr);
		}
		else
		{
			printf("connect redis server error\n");
		}
	}
	else
	{
		printf("connect to server successfully!\n");
	}
	return _ctx;
}

const char* RedisHome::getErrorString()
{
	return _last_status.c_str();
}

int RedisHome::getError()
{
	return _last_type;
}

redisReply* RedisHome::command(const char *command)
{
	clearAll();
	snprintf(_last_cmd, sizeof(_last_cmd), command);
	_repl = (redisReply*)redisCommand(_ctx, command);
	if (!_repl)
	{
		//很有可能是断线了，需要重连
		if (_ctx)
		{
			_last_type = _ctx->err;
			_last_status.append(_ctx->errstr == NULL ? "" : _ctx->errstr);
		}
		return NULL;
	}
	_last_type = _repl->type;
	if (_repl->type == REDIS_REPLY_STATUS)
	{
		_last_status.append(_repl->str);
	}
	else if (_repl->type == REDIS_REPLY_INTEGER)
	{
		_last_int = _repl->integer;
	}
	else if (_repl->type == REDIS_REPLY_STRING)
	{
		_last_status.append(_repl->str);
	}
	else if (_repl->type == REDIS_REPLY_ERROR)
	{
		_last_status.append(_repl->str == NULL ? "" : _repl->str);
	}
	return _repl;
}

bool RedisHome::next(ClusterSlot& slot)
{
	if (!_repl || _repl->type != REDIS_REPLY_ARRAY)
	{
		return false;
	}
	if (_idx >= _repl->elements)
	{
		return false;
	}
	if (_repl->element[_idx]->type != REDIS_REPLY_ARRAY)
	{
		return false;
	}
	_sstate = sbegin;
	//数组中的数组
	struct redisReply **eles = _repl->element[_idx]->element;
	for (int j = 0; j < _repl->element[_idx]->elements; ++j)
	{
		if (eles[j]->type == REDIS_REPLY_INTEGER)
		{
			if (_sstate == sbegin)
			{
				slot.begin = eles[j]->integer;
				_sstate = sendt;
			}
			else if (_sstate == sendt)
			{
				slot.end = eles[j]->integer;
				_sstate = sip1;
			}
		}
		else if (eles[j]->type == REDIS_REPLY_ARRAY)
		{
			struct redisReply **elements = eles[j]->element;
			for (int k = 0; k < eles[j]->elements; ++k)
			{
				if (elements[k]->type == REDIS_REPLY_INTEGER)
				{
					if (_sstate == sport1)
					{
						slot.master.port = elements[k]->integer;
						_sstate = shash1;
					}
					else if (_sstate == sport2)
					{
						slot.slave.port = elements[k]->integer;
						_sstate = shash2;
					}
				}
				else if (elements[k]->type == REDIS_REPLY_STRING)
				{
					if (_sstate == sip1)
					{
						slot.master.ip = elements[k]->str;
						_sstate = sport1;
					}
					else if (_sstate == shash1)
					{
						slot.master.hash = elements[k]->str;
						_sstate = sip2;
					}
					else if (_sstate == sip2)
					{
						slot.slave.ip = elements[k]->str;
						_sstate = sport2;
					}
					else if (_sstate == shash2)
					{
						slot.slave.hash = elements[k]->str;
					}
				}
			}
		}
	}
	++_idx;
	return _sstate == shash2;
}

bool RedisHome::next(std::string& key, std::string& val)
{
	if (_str_pos < 0)
	{
		getArrayString();
		_str_pos = 0;
	}
	if (_str_pos + 1 >= _str_result.size())
	{
		return false;
	}
	key = _str_result[_str_pos];
	val = _str_result[_str_pos + 1];
	++_str_pos;
	return true;
}

std::string RedisHome::getString()
{
	return _last_status;
}

bool RedisHome::isError()
{
	return _ctx == NULL || _ctx->err;
}

long long RedisHome::getInteger()
{
	return _last_int;
}

void RedisHome::clearAll()
{
	if (_repl)
	{
		freeReplyObject(_repl);
	}
	_sstate = snull;
	_str_pos = -1;
	_last_type = -1;
	_idx = 0;
	_last_status = "";
	_str_result.clear();
}

void RedisHome::getArrayString()
{
	if (_repl->type == REDIS_REPLY_ARRAY)
	{
		for (int i = 0; i < _repl->elements; ++i)
		{
			if (_repl->element[i]->type == REDIS_REPLY_STRING)
			{
				_str_result.push_back(_repl->element[i]->str);
			}
		}
	}
}

bool RedisHome::reconnect()
{
	_ctx = redisConnectWithTimeout(_ipstr.c_str(), _port, _tval);
	if (_ctx == NULL || _ctx->err)
	{
		return false;
	}
	return true;
}