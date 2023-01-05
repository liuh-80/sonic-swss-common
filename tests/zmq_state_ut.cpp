#include <iostream>
#include <memory>
#include <thread>
#include <algorithm>
#include <deque>
#include "gtest/gtest.h"
#include "common/dbconnector.h"
#include "common/notificationconsumer.h"
#include "common/notificationproducer.h"
#include "common/select.h"
#include "common/selectableevent.h"
#include "common/table.h"
#include "common/zmqproducerstatetable.h"
#include "common/zmqconsumerstatetable.h"

using namespace std;
using namespace swss;

#define TEST_DB           "APPL_DB" // Need to test against a DB which uses a colon table name separator due to hardcoding in consumer_table_pops.lua
#define NUMBER_OF_THREADS    (10) // Spawning more than 256 threads causes libc++ to except
#define NUMBER_OF_OPS        (100)
#define MAX_FIELDS       10 // Testing up to 30 fields objects
#define PRINT_SKIP           (10) // Print + for Producer and - for Consumer for every 100 ops
#define MAX_KEYS             (10)       // Testing up to 30 keys objects

std::shared_ptr<swss::DBConnector>          g_db;
std::shared_ptr<swss::RedisPipeline>        g_redisPipeline;
bool g_destoried = false;
class TestStr : public string
{
public:
    TestStr(char* val)
    : string(val)
    {
        cout << "Ctor: " << this->c_str()  << endl;
    }
    
    ~TestStr()
    {
        cout << "Dtor: " << this->c_str() << endl;
        g_destoried = true;
    }
};

static void exitHandler()
{
    sleep(2);
    exit(0);
    //_exit(0);
}


static void routeCreater()
{
    while (true)
    {
        //cout << "Start redis command" << endl;
        RedisCommand command;
        command.format("HLEN %s", "TEST_KEY");
        g_redisPipeline->push(command, REDIS_REPLY_INTEGER);
        //cout << "End redis command" << endl;
    }
}

static void checkStaticString()
{
    static TestStr test("1234567890");
    while (true)
    {
        cout << "Static string: " << test.c_str() << endl;
        if (g_destoried)
        {
            break;
        }
    }
}

TEST(RedisPipeline, test)
{
    g_db                 = std::make_shared<swss::DBConnector>(ASIC_DB, swss::DBConnector::DEFAULT_UNIXSOCKET, 0);
    g_redisPipeline      = std::make_shared<swss::RedisPipeline>(g_db.get());

    thread *exitThread = new thread(exitHandler);
    thread *routeCreaterThread = new thread(routeCreater);
    thread *checkStaticStringThread = new thread(checkStaticString);

    sleep(5);

    exitThread->join();
    routeCreaterThread->join();
    checkStaticStringThread->join();
    delete exitThread;
    delete routeCreaterThread;
    delete checkStaticStringThread;
}
