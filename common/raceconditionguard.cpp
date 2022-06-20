#include <iostream>
#include <sstream>
#include <system_error>

#include "common/logger.h"
#include "common/raceconditionguard.h"

using namespace std;
using namespace swss;

RaceConditionGuard::RaceConditionGuard(atomic<bool> &running, const string &info)
:m_running(running)
{
    auto currentlyRunning = m_running.exchange(true);
    if (currentlyRunning)
    {
        string errmsg = "Current thread '" + info + "' conflict with other thread.";
        SWSS_LOG_ERROR("%s", errmsg.c_str());
        throw system_error(make_error_code(errc::operation_in_progress), errmsg.c_str());
    }
}

RaceConditionGuard::~RaceConditionGuard()
{
    m_running = false;
}