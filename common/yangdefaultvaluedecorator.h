#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <utility>
#include <mutex>

#include <hiredis/hiredis.h>
#include "dbdecorator.h"
#include "rediscommand.h"
#include "redisreply.h"

#include "dbconnector.h"
#include "table.h"

namespace swss {

class PubSub;
class RedisPipeline;

class YangDefaultValueDecorator : public DBConnector
{
public:
    YangDefaultValueDecorator(DBConnector* connector);
    virtual ~YangDefaultValueDecorator();
    YangDefaultValueDecorator& operator=(const YangDefaultValueDecorator&) = delete;

    virtual int getDbId() const;
    virtual std::string getDbName() const;
    virtual std::string getNamespace() const;

    virtual PubSub *pubsub();

    virtual int64_t del(const std::string &key);

    virtual bool exists(const std::string &key);

    virtual int64_t hdel(const std::string &key, const std::string &field);

    virtual int64_t hdel(const std::string &key, const std::vector<std::string> &fields);

    virtual void del(const std::vector<std::string>& keys);

    virtual std::map<std::string, std::string> hgetall(const std::string& key);

    virtual std::unordered_map<std::string, std::string> hgetall_unordered(const std::string& key);

#ifndef SWIG
    virtual void hgetall(const std::string& key, std::insert_iterator<std::map<std::string, std::string> > result);
#endif

    virtual std::vector<std::string> keys(const std::string &key);

    virtual std::pair<int, std::vector<std::string>> scan(int cursor = 0, const char *match = "", uint32_t count = 10);

    virtual bool set(const std::string &key, const std::string &value);
    virtual bool set(const std::string &key, int value);

    virtual void hset(const std::string &key, const std::string &field, const std::string &value);

    virtual void hmset(const std::string &key, std::vector<swss::FieldValueTuple>::iterator start, std::vector<swss::FieldValueTuple>::iterator stop);

    virtual void hmset(const std::string &key, std::map<std::string, std::string>::const_iterator start, std::map<std::string, std::string>::const_iterator stop);

    virtual void hmset(const std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& multiHash);

    virtual std::shared_ptr<std::string> get(const std::string &key);

    virtual std::shared_ptr<std::string> hget(const std::string &key, const std::string &field);

    virtual bool hexists(const std::string &key, const std::string &field);

    virtual int64_t incr(const std::string &key);

    virtual int64_t decr(const std::string &key);

    virtual int64_t rpush(const std::string &list, const std::string &item);

    virtual std::shared_ptr<std::string> blpop(const std::string &list, int timeout);

    virtual void subscribe(const std::string &pattern);

    virtual void psubscribe(const std::string &pattern);

    virtual void punsubscribe(const std::string &pattern);

    virtual int64_t publish(const std::string &channel, const std::string &message);

    virtual void config_set(const std::string &key, const std::string &value);

    virtual bool flushdb();

    virtual bool get(RedisPipeline *pipe, const std::string &key, std::vector<swss::FieldValueTuple> &values);

private:
    std::shared_ptr<DBConnector> m_connector;
};

}
