#ifndef __DBCONNECTOR__
#define __DBCONNECTOR__

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>
#include <mutex>

#include <hiredis/hiredis.h>
#include "dbdecorator.h"
#include "rediscommand.h"
#include "redisreply.h"
#define EMPTY_NAMESPACE std::string()

namespace swss {

class DBConnector;
class PubSub;
class RedisPipeline;

typedef std::map<swss::DBDecoratorType, std::shared_ptr<DBDecorator> > DecoratorMapping;

class RedisInstInfo
{
public:
    std::string unixSocketPath;
    std::string hostname;
    int port;
};

class SonicDBInfo
{
public:
    std::string instName;
    int dbId;
    std::string separator;
};

class SonicDBConfig
{
public:
    static constexpr const char *DEFAULT_SONIC_DB_CONFIG_FILE = "/var/run/redis/sonic-db/database_config.json";
    static constexpr const char *DEFAULT_SONIC_DB_GLOBAL_CONFIG_FILE = "/var/run/redis/sonic-db/database_global.json";
    static void initialize(const std::string &file = DEFAULT_SONIC_DB_CONFIG_FILE);
#ifdef SWIG
    %pythoncode %{
        ## TODO: the python function and C++ one is not on-par
        @staticmethod
        def load_sonic_db_config(sonic_db_file_path=DEFAULT_SONIC_DB_CONFIG_FILE):
            SonicDBConfig.initialize(sonic_db_file_path)
    %}
#endif

    static void initializeGlobalConfig(const std::string &file = DEFAULT_SONIC_DB_GLOBAL_CONFIG_FILE);
#ifdef SWIG
    %pythoncode %{
        ## TODO: the python function and C++ one is not on-par
        @staticmethod
        def load_sonic_global_db_config(global_db_file_path=DEFAULT_SONIC_DB_GLOBAL_CONFIG_FILE, namespace=None):
            SonicDBConfig.initializeGlobalConfig(global_db_file_path)
    %}
#endif

    static void validateNamespace(const std::string &netns);
    static std::string getDbInst(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static int getDbId(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static std::string getSeparator(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static std::string getSeparator(int dbId, const std::string &netns = EMPTY_NAMESPACE);
    static std::string getSeparator(const DBConnector* db);
    static std::string getDbSock(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static std::string getDbHostname(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static int getDbPort(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static std::vector<std::string> getNamespaces();
#ifdef SWIG
    %pythoncode %{
        ## TODO: the python function and C++ one is not on-par
        @staticmethod
        def get_ns_list():
            return SonicDBConfig.getNamespaces()
    %}
#endif

    static std::vector<std::string> getDbList(const std::string &netns = EMPTY_NAMESPACE);
    static bool isInit() { return m_init; };
    static bool isGlobalInit() { return m_global_init; };

private:
    static std::recursive_mutex m_db_info_mutex;
    // { namespace { instName, { unix_socket_path, hostname, port } } }
    static std::unordered_map<std::string, std::unordered_map<std::string, RedisInstInfo>> m_inst_info;
    // { namespace, { dbName, {instName, dbId, separator} } }
    static std::unordered_map<std::string, std::unordered_map<std::string, SonicDBInfo>> m_db_info;
    // { namespace, { dbId, separator } }
    static std::unordered_map<std::string, std::unordered_map<int, std::string>> m_db_separator;
    static bool m_init;
    static bool m_global_init;
    static void parseDatabaseConfig(const std::string &file,
                                    std::unordered_map<std::string, RedisInstInfo> &inst_entry,
                                    std::unordered_map<std::string, SonicDBInfo> &db_entry,
                                    std::unordered_map<int, std::string> &separator_entry);
    static SonicDBInfo& getDbInfo(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
    static RedisInstInfo& getRedisInfo(const std::string &dbName, const std::string &netns = EMPTY_NAMESPACE);
};

class RedisContext
{
public:
    static constexpr const char *DEFAULT_UNIXSOCKET = "/var/run/redis/redis.sock";

    /*
     * Connect to Redis DB wither with a hostname:port or unix socket
     * Select the database index provided by "db"
     *
     * Timeout - The time in milisecond until exception is been thrown. For
     *           infinite wait, set this value to 0
     */
    RedisContext(const RedisContext &other);
    RedisContext& operator=(const RedisContext&) = delete;

    ~RedisContext();

    redisContext *getContext() const;

    /*
     * Assign a name to the Redis client used for this connection
     * This is helpful when debugging Redis clients using `redis-cli client list`
     */
    void setClientName(const std::string& clientName);

    std::string getClientName();

protected:
    RedisContext();
    void initContext(const char *host, int port, const timeval *tv);
    void initContext(const char *path, const timeval *tv);
    void setContext(redisContext *ctx);

private:
    redisContext *m_conn;
};

class DBConnector : public RedisContext
{
public:
    static constexpr const char *DEFAULT_UNIXSOCKET = "/var/run/redis/redis.sock";

    /*
     * Connect to Redis DB wither with a hostname:port or unix socket
     * Select the database index provided by "db"
     *
     * Timeout - The time in milisecond until exception is been thrown. For
     *           infinite wait, set this value to 0
     */
    explicit DBConnector(const DBConnector &other);
    DBConnector(int dbId, const RedisContext &ctx);
    DBConnector(int dbId, const std::string &hostname, int port, unsigned int timeout);
    DBConnector(int dbId, const std::string &unixPath, unsigned int timeout);
    DBConnector(const std::string &dbName, unsigned int timeout, bool isTcpConn = false);
    DBConnector(const std::string &dbName, unsigned int timeout, bool isTcpConn, const std::string &netns);
    virtual ~DBConnector();
    DBConnector& operator=(const DBConnector&) = delete;

    virtual int getDbId() const;
    virtual std::string getDbName() const;
    virtual std::string getNamespace() const;

#ifdef SWIG
    %pythoncode %{
        namespace = property(getNamespace)
    %}
#endif

    static void select(DBConnector *db);

    /* Create new context to DB */
    DBConnector *newConnector(unsigned int timeout) const;

    virtual PubSub *pubsub();

    virtual int64_t del(const std::string &key);

#ifdef SWIG
    // SWIG interface file (.i) globally rename map C++ `del` to python `delete`,
    // but applications already followed the old behavior of auto renamed `_del`.
    // So we implemented old behavior for backward compatibility
    // TODO: remove this function after applications use the function name `delete`
    %pythoncode %{
        def _del(self, *args, **kwargs):
            return self.delete(*args, **kwargs)
    %}
#endif

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
protected:
    DBConnector();

private:
    void setNamespace(const std::string &netns);

    template <typename ReturnType=std::unordered_map<std::string, std::string>>
    ReturnType hgetall_internal(const std::string &key);

    template <typename OutputIterator>
    void hgetall_internal(const std::string &key, OutputIterator result);

    template<typename InputIterator>
    void hmset_internal(const std::string &key, InputIterator start, InputIterator stop);

    int m_dbId;
    std::string m_dbName;
    std::string m_namespace;

    std::string m_shaRedisMulti;
};

/*
// TODO: need more discussion about the API design, use may both want get/not get default vaule when running time
class CfgDBConnector : public DBConnector
{
    explicit CfgDBConnector(const DBConnector &other, bool getDefaultValue);
    CfgDBConnector(int dbId, const RedisContext &ctx, bool getDefaultValue);
    CfgDBConnector(int dbId, const std::string &hostname, int port, unsigned int timeout, bool getDefaultValue);
    CfgDBConnector(int dbId, const std::string &unixPath, unsigned int timeout, bool getDefaultValue);
    CfgDBConnector(const std::string &dbName, unsigned int timeout, bool getDefaultValue, bool isTcpConn = false);
    CfgDBConnector(const std::string &dbName, unsigned int timeout, bool isTcpConn, const std::string &netns, bool getDefaultValue);
    CfgDBConnector& operator=(const DBConnector&) = delete;
    CfgDBConnector& operator=(const CfgDBConnector&) = delete;
    
    ~CfgDBConnector();

    std::shared_ptr<DBDecorator> getDBDecortor() const override;

private:
    bool m_get_default_value;
    std::shared_ptr<DBDecorator> m_db_decorator;
};
*/

}
#endif
