#include "common/selectable.h"
#include "common/logger.h"
#include "common/select.h"
#include <algorithm>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>


using namespace std;

namespace swss {

Select::Select()
{
    m_epoll_fd = ::epoll_create1(0);
    if (m_epoll_fd == -1)
    {
        std::string error = std::string("Select::constructor:epoll_create1: error=("
                          + std::to_string(errno) + "}:"
                          + strerror(errno));
        throw std::runtime_error(error);
    }
}

Select::~Select()
{
    (void)::close(m_epoll_fd);
}

void Select::addSelectable(Selectable *selectable)
{
    const int fd = selectable->getFd();

    if(m_objects.find(fd) != m_objects.end())
    {
        SWSS_LOG_WARN("Selectable is already added to the list, ignoring.");
        return;
    }

    m_objects[fd] = selectable;

    if (selectable->initializedWithData())
    {
        SWSS_LOG_ERROR("[TEST] Select::addSelectable has data: %zu,  m_ready: %zu", (size_t)selectable, m_ready.size());
        selectable->updateEarliestEventTime();
        m_ready.insert(selectable);
        SWSS_LOG_ERROR("[TEST] Select::addSelectable after insert m_ready: %zu", m_ready.size());
    }

    struct epoll_event ev = {
        .events = EPOLLIN,
        .data = { .fd = fd, },
    };

    int res = ::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (res == -1)
    {
        std::string error = std::string("Select::add_fd:epoll_ctl: error=("
                          + std::to_string(errno) + "}:"
                          + strerror(errno));
        throw std::runtime_error(error);
    }

    SWSS_LOG_ERROR("[TEST]  Select::addSelectable m_ready: %zu", m_ready.size());
}

void Select::removeSelectable(Selectable *selectable)
{
    const int fd = selectable->getFd();

    m_objects.erase(fd);
    SWSS_LOG_ERROR("[TEST] Select::removeSelectable: %zu, m_ready: %zu", (size_t)selectable, m_ready.size());
    selectable->resetEarliestEventTime();
    m_ready.erase(selectable);

    int res = ::epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    if (res == -1)
    {
        std::string error = std::string("Select::del_fd:epoll_ctl: error=("
                          + std::to_string(errno) + "}:"
                          + strerror(errno));
        throw std::runtime_error(error);
    }
}

void Select::addSelectables(vector<Selectable *> selectables)
{
    for(auto it : selectables)
    {
        addSelectable(it);
    }
}

int Select::poll_descriptors(Selectable **c, unsigned int timeout, bool interrupt_on_signal = false)
{
    int sz_selectables = static_cast<int>(m_objects.size());
    std::vector<struct epoll_event> events(sz_selectables);
    int ret;

    while(true)
    {
        ret = ::epoll_wait(m_epoll_fd, events.data(), sz_selectables, timeout);
        // on signal interrupt check if we need to return
        if (ret == -1 && errno == EINTR)
        {
            if (interrupt_on_signal)
            {
                return Select::SIGNALINT;
            }
        }
        // on all other errors break the loop
        else
        {
            break;
        }
    }

    if (ret < 0)
    {
        return Select::ERROR;
    }

    std::string tables = "";
    for (int i = 0; i < ret; ++i)
    {
        int fd = events[i].data.fd;
        Selectable* sel = m_objects[fd];
        try
        {
            sel->readData();
        }
        catch (const std::runtime_error& ex)
        {
            SWSS_LOG_ERROR("readData error: %s", ex.what());
            return Select::ERROR;
        }

        tables += std::to_string((size_t)sel);
        tables += ";";
        sel->updateEarliestEventTime();
        m_ready.insert(sel);
    }
    SWSS_LOG_ERROR("[TEST] Select::poll sel: %s , m_ready: %zu, epool ret: %d", tables.c_str(), m_ready.size(), ret);

    while (!m_ready.empty())
    {
        auto sel = *m_ready.begin();

        SWSS_LOG_ERROR("[TEST] Select::poll erase: %zu", (size_t)sel);
        m_ready.erase(sel);

        if (!sel->hasData())
        {
            SWSS_LOG_ERROR("[TEST] Select::poll remove empty: %zu", (size_t)sel);
            sel->resetEarliestEventTime();
            continue;
        }

        *c = sel;

        if (sel->hasCachedData())
        {
            // reinsert Selectable back to the m_ready set, when there're more messages in the cache
            SWSS_LOG_ERROR("[TEST] Select::poll re-insert: %zu", (size_t)sel);
            m_ready.insert(sel);
        }
        else
        {
            sel->resetEarliestEventTime();
        }

        sel->updateAfterRead();

        SWSS_LOG_ERROR("[TEST] Select::poll return: %zu , m_ready: %zu", (size_t)sel, m_ready.size());
        return Select::OBJECT;
    }

    SWSS_LOG_ERROR("[TEST] Select::poll timeout, m_ready: %zu", m_ready.size());
    return Select::TIMEOUT;
}

int Select::select(Selectable **c, int timeout, bool interrupt_on_signal)
{
    SWSS_LOG_ENTER();

    int ret;

    *c = NULL;

    /* check if we have some data */
    ret = poll_descriptors(c, 0);

    /* return if we have data, we have an error or desired timeout was 0 */
    if (ret != Select::TIMEOUT || timeout == 0)
        return ret;

    /* wait for data */
    ret = poll_descriptors(c, timeout, interrupt_on_signal);

    return ret;

}

bool Select::isQueueEmpty()
{
    return m_ready.empty();
}

std::string Select::resultToString(int result)
{
    SWSS_LOG_ENTER();

    switch (result)
    {
        case swss::Select::OBJECT:
            return "OBJECT";

        case swss::Select::ERROR:
            return "ERROR";

        case swss::Select::TIMEOUT:
            return "TIMEOUT";

        case swss::Select::SIGNALINT:
            return "SIGNALINT";

        default:
            SWSS_LOG_WARN("unknown select result: %d", result);
            return "UNKNOWN";
    }
}

};
