#ifndef __SELECT__
#define __SELECT__

#include <string>
#include <vector>
#include <limits>
#include <chrono>
#include <hiredis/hiredis.h>

namespace swss {

static std::chrono::time_point<std::chrono::steady_clock> g_max_event_time = std::chrono::time_point<std::chrono::steady_clock>::max();

class Selectable
{
public:
    Selectable(int pri = 0) : m_priority(pri),
                              m_last_used_time(std::chrono::steady_clock::now()) {}

    virtual ~Selectable() = default;

    /* return file handler for the Selectable */
    virtual int getFd() = 0;

    /* Read all data from the fd associated with Selectable */
    virtual uint64_t readData() = 0;

    /*
       true if Selectable has data in it for immediate read
       This method is intended to be implemented in subclasses of Selectable
       which also implement the hasCachedData() method
       it's possible to have a case, when an application read all cached data after a signal,
       and we have a Selectable with no data in the m_ready queue (Select class).
       The class without hasCachedData never is going to be in m_ready state without the data
    */
    virtual bool hasData()
    {
        return true;
    }

    /* true if Selectable has data in its cache */
    virtual bool hasCachedData()
    {
        return false;
    }

    /* true if Selectable was initialized with data */
    virtual bool initializedWithData()
    {
        return false;
    }

    /* run this function after every read */
    virtual void updateAfterRead()
    {
    }

    virtual int getPri() const
    {
        return m_priority;
    }

private:

    friend class Select;

    // only Select class can access and update m_earliest_event_time


    std::chrono::time_point<std::chrono::steady_clock> getEarliestEventTime() const
    {
        return m_earliest_event_time;
    }

    void updateEarliestEventTime()
    {
        if (m_earliest_event_time != g_max_event_time)
        {
            return;
        }

        m_earliest_event_time = std::chrono::steady_clock::now();
    }

    void resetEarliestEventTime()
    {
        m_earliest_event_time = g_max_event_time;
    }

    int m_priority; // defines priority of Selectable inside Select
                    // higher value is higher priority
    std::chrono::time_point<std::chrono::steady_clock> m_earliest_event_time;
};

}

#endif
