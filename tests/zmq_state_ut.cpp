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
#include "common/producerstatetable.h"
#include "common/consumerstatetable.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

using namespace std;
using namespace swss;

#define TEST_DB           "APPL_DB" // Need to test against a DB which uses a colon table name separator due to hardcoding in consumer_table_pops.lua
#define NUMBER_OF_THREADS    (1) // Spawning more than 256 threads causes libc++ to except
#define NUMBER_OF_OPS        (10000)
#define MAX_FIELDS       10 // Testing up to 30 fields objects
#define PRINT_SKIP           (10) // Print + for Producer and - for Consumer for every 100 ops
#define MAX_KEYS             (10)       // Testing up to 30 keys objects

static int running_thread_count = 0;
std::chrono::time_point<std::chrono::system_clock>  g_startTime;
std::chrono::time_point<std::chrono::system_clock>  g_endTime;

static inline string field(int i)
{
    return string("field ") + to_string(i);
}

static inline string value(int i)
{
    if (i == 0) return string(); // empty
    return string("value ") + to_string(i);
}

static inline bool IsDigit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

static inline int readNumberAtEOL(const string& str)
{
    if (str.empty()) return 0;
    auto pos = find_if(str.begin(), str.end(), IsDigit);
    istringstream is(str.substr(pos - str.begin()));
    int ret;
    is >> ret;
    EXPECT_TRUE((bool)is);
    return ret;
}

static void producerWorker(string tableName, string endpoint)
{
    DBConnector db(TEST_DB, 0, true);
    ProducerStateTable p(&db, tableName);
    cout << "Producer thread started: " << tableName << endl;

    g_startTime = std::chrono::system_clock::now();
    //for (int i = 0; i < NUMBER_OF_OPS; i++)
    for (int i = 0; i < NUMBER_OF_OPS;)
    {
        /*
            vector<FieldValueTuple> fields;
            for (int j = 0; j < MAX_FIELDS; j++)
            {
                FieldValueTuple t(field(j), value(j));
                fields.push_back(t);
            }
            p.set("set_key_" + to_string(i), fields);
        */
        std::vector<KeyOpFieldsValuesTuple> values;

        for (int k=0; k< 2; k++)
        {
            values.emplace_back();
            auto &current_value = values.back();

            kfvKey(current_value) = "set_key_" + to_string(i);
            for (int j = 0; j < MAX_FIELDS; j++)
            {
                kfvFieldsValues(current_value).emplace_back(field(j), value(j));
            }

            i++;
        }

        p.set(values);
        //cout << "Producer thread send: " << i << endl;
    }

    cout << "Producer thread ended: " << tableName << endl;
    
    running_thread_count--;
}

static void consumerWorker(string tableName, string endpoint)
{
    cout << "Consumer thread started: " << tableName << endl;
    
    DBConnector db(TEST_DB, 0, true);
    ConsumerStateTable c(&db, tableName);
    Select cs;
    cs.addSelectable(&c);

    //validate received data
    Selectable *selectcs;
    std::deque<KeyOpFieldsValuesTuple> vkco;
    int ret = 0;
    int received = 0;
    while (received < NUMBER_OF_OPS)
    {
        if ((ret = cs.select(&selectcs)) == Select::OBJECT)
        {
            c.pops(vkco);
            if (!vkco.empty())
            {
                g_endTime = std::chrono::system_clock::now();
            }

            while (!vkco.empty())
            {
                received++;
                vkco.pop_front();
            }
            //cout << "Consumer thread receive: " << received << endl;
        }
    }

    cout << "Consumer thread ended: " << tableName << "received: " << received << endl;
}

static inline void clearDB()
{
    DBConnector db(TEST_DB, 0, true);
    RedisReply r(&db, "FLUSHALL", REDIS_REPLY_STATUS);
    r.checkStatusOK();
}

TEST(ZmqConsumerStateTable, perf_test)
{
    clearDB();
    std::string testTableName = "ZMQ_PROD_CONS_UT";
    std::string pushEndpoint = "tcp://localhost:1234";
    std::string pullEndpoint = "tcp://*:1234";
    thread *producerThreads[NUMBER_OF_THREADS];

    // start consumer first, SHM can only have 1 consumer per table.
    thread *consumerThread = new thread(consumerWorker, testTableName, pullEndpoint);

    cout << "Starting " << NUMBER_OF_THREADS << " producers" << endl;
    /* Starting the producer before the producer */
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        running_thread_count++;
        producerThreads[i] = new thread(producerWorker, testTableName, pushEndpoint);
    }

    cout << "Done. Waiting for all job to finish " << NUMBER_OF_OPS << " jobs." << endl;
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        producerThreads[i]->join();
        delete producerThreads[i];
    }
    
    consumerThread->join();
    delete consumerThread;

    std::cout
      << "Transfer data took "
      << std::chrono::duration_cast<std::chrono::microseconds>(g_endTime - g_startTime).count() << "µs ≈ " << std::endl;
      
    cout << endl << "Done." << endl;
}
