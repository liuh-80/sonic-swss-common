#pragma once
#include "common/dbconnector.h"


namespace swss {

class RaceConditionGuard
{
public:
    RaceConditionGuard(std::atomic<bool> &running, const std::string &info);
    ~RaceConditionGuard();

private:
    std::atomic<bool> &m_running;
};

}