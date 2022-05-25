
#include "common/yangdefaultvaluedecorator.h"

#include "common/dbconnector.h"
#include "common/dbdecorator.h"
#include "common/redisreply.h"
#include "common/redisapi.h"
#include "common/pubsub.h"

using namespace std;
using namespace swss;

YangDefaultValueDecorator::YangDefaultValueDecorator(DBConnector* connector)
{
    m_connector = std::shared_ptr<DBConnector>(connector);
}

YangDefaultValueDecorator::~YangDefaultValueDecorator()
{
}

int YangDefaultValueDecorator::getDbId() const
{
    return m_connector->getDbId();
}

string YangDefaultValueDecorator::getDbName() const
{
    return m_connector->getDbName();
}

string YangDefaultValueDecorator::getNamespace() const
{
    return m_connector->getNamespace();
}

PubSub* YangDefaultValueDecorator::pubsub()
{
    return m_connector->pubsub();
}

int64_t YangDefaultValueDecorator::del(const std::string &key)
{
    return m_connector->del(key);
}

bool YangDefaultValueDecorator::exists(const std::string &key)
{
    return m_connector->exists(key);
}

int64_t YangDefaultValueDecorator::hdel(const std::string &key, const std::string &field)
{
    return m_connector->hdel(key, field);
}

int64_t YangDefaultValueDecorator::hdel(const std::string &key, const std::vector<std::string> &fields)
{
    return m_connector->hdel(key, fields);
}

void YangDefaultValueDecorator::del(const std::vector<std::string>& keys)
{
    m_connector->del(keys);
}

map<string, string> YangDefaultValueDecorator::hgetall(const std::string& key)
{
    return m_connector->hgetall(key);
}

std::unordered_map<std::string, std::string> YangDefaultValueDecorator::hgetall_unordered(const std::string& key)
{
    return m_connector->hgetall_unordered(key);
}

#ifndef SWIG
void YangDefaultValueDecorator::hgetall(const std::string& key, std::insert_iterator<std::map<std::string, std::string> > result)
{
    m_connector->hgetall(key, result);

    /*
    auto dbdecortor = this->getDBDecorator(ReadDecorator);
    if (dbdecortor)
    {
        dbdecortor->decorate(key, ctx, result);
    }
    */
}
#endif

std::vector<std::string> YangDefaultValueDecorator::keys(const std::string &key)
{
    return m_connector->keys(key);
}

std::pair<int, std::vector<std::string>> YangDefaultValueDecorator::scan(int cursor, const char *match, uint32_t count)
{
    return m_connector->scan(cursor, match, count);
}

bool YangDefaultValueDecorator::set(const std::string &key, const std::string &value)
{
    return m_connector->set(key, value);
}

bool YangDefaultValueDecorator::set(const std::string &key, int value)
{
    return m_connector->set(key, value);
}

void YangDefaultValueDecorator::hset(const std::string &key, const std::string &field, const std::string &value)
{
    m_connector->hset(key, field, value);
}

void YangDefaultValueDecorator::hmset(const std::string &key, std::vector<FieldValueTuple>::iterator start, std::vector<FieldValueTuple>::iterator stop)
{
    m_connector->hmset(key, start, stop);
}

void YangDefaultValueDecorator::hmset(const std::string &key, std::map<std::string, std::string>::const_iterator start, std::map<std::string, std::string>::const_iterator stop)
{
    m_connector->hmset(key, start, stop);
}

void YangDefaultValueDecorator::hmset(const std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& multiHash)
{
    m_connector->hmset(multiHash);
}

std::shared_ptr<std::string> YangDefaultValueDecorator::get(const std::string &key)
{
    return m_connector->get(key);
}

std::shared_ptr<std::string> YangDefaultValueDecorator::hget(const std::string &key, const std::string &field)
{
    return m_connector->hget(key, field);

    /*
        auto dbdecortor = this->getDBDecorator(ReadDecorator);
        if (dbdecortor)
        {
            return dbdecortor->decorate(key, field);
        }
        else
        {
            return shared_ptr<string>(NULL);
        }
        */
}

bool YangDefaultValueDecorator::hexists(const std::string &key, const std::string &field)
{
    return m_connector->hexists(key, field);
}

int64_t YangDefaultValueDecorator::incr(const std::string &key)
{
    return m_connector->incr(key);
}

int64_t YangDefaultValueDecorator::decr(const std::string &key)
{
    return m_connector->decr(key);
}

int64_t YangDefaultValueDecorator::rpush(const std::string &list, const std::string &item)
{
    return m_connector->rpush(list, item);
}

std::shared_ptr<std::string> YangDefaultValueDecorator::blpop(const std::string &list, int timeout)
{
    return m_connector->blpop(list, timeout);
}

void YangDefaultValueDecorator::subscribe(const std::string &pattern)
{
    m_connector->subscribe(pattern);
}

void YangDefaultValueDecorator::psubscribe(const std::string &pattern)
{
    m_connector->psubscribe(pattern);
}

void YangDefaultValueDecorator::punsubscribe(const std::string &pattern)
{
    m_connector->punsubscribe(pattern);
}

int64_t YangDefaultValueDecorator::publish(const std::string &channel, const std::string &message)
{
    return m_connector->publish(channel, message);
}

void YangDefaultValueDecorator::config_set(const std::string &key, const std::string &value)
{
    m_connector->config_set(key, value);
}

bool YangDefaultValueDecorator::flushdb()
{
    return m_connector->flushdb();
}

bool YangDefaultValueDecorator::get(RedisPipeline *pipe, const string &key, vector<FieldValueTuple> &values)
{
    return m_connector->get(pipe, key, values);
    /*
    // Decorate result because result is not from DBConnection, so it's not decorated
    auto& db_decorator = getDBDecorator(ReadDecorator);
    if (db_decorator != nullptr)
    {
        db_decorator->decorate(key, values);
    }
    */
}