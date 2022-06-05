#include <iostream>
#include <signal.h>
#include "common/logger.h"
#include "signalhandlerhelper.h"

using namespace swss;

std::map<int, bool> SignalHandlerHelper::m_signalStatusMapping;
std::map<int, SigActionPair> SignalHandlerHelper::m_sigActionMapping;
std::map<int, std::shared_ptr<SignalCallbackBase>> SignalHandlerHelper::m_sigCallbackMapping;

void SignalHandlerHelper::registerSignalHandler(int signalNumber)
{
    auto result = m_sigActionMapping.find(signalNumber);
    if (result != m_sigActionMapping.end())
    {
        // signal action already registered
        SWSS_LOG_WARN("sigaction for %d already registered.", signalNumber);
        return;
    }

    m_signalStatusMapping[signalNumber] = false;

    SigActionPair sig_action_pair;
    auto *new_action = &sig_action_pair.first;
    auto *old_action = &sig_action_pair.second;

    new_action->sa_handler = SignalHandlerHelper::onSignal;
    sigemptyset(&new_action->sa_mask);
    new_action->sa_flags = 0;

    // always replace old action even old action is ignore signal
    sigaction(signalNumber, new_action, old_action);

    m_sigActionMapping[signalNumber] = sig_action_pair;
}

void SignalHandlerHelper::registerSignalHandler(int signalNumber, std::shared_ptr<SignalCallbackBase> callback)
{
    // Call any method on callback here will crash, because GLI been released by swsscommon exception handler.
    m_sigCallbackMapping[signalNumber] = callback;
    SignalHandlerHelper::registerSignalHandler(signalNumber);
}

void SignalHandlerHelper::restoreSignalHandler(int signalNumber)
{
    auto result = m_sigActionMapping.find(signalNumber);
    if (result == m_sigActionMapping.end())
    {
        // signal action does not registered
        SWSS_LOG_WARN("sigaction for %d does not registered.",signalNumber);
        return;
    }

    auto *old_action = &result->second.second;

    sigaction(signalNumber, old_action, NULL);
}

void SignalHandlerHelper::onSignal(int signalNumber)
{
    m_signalStatusMapping[signalNumber] = true;
    auto result = m_sigCallbackMapping.find(signalNumber);
    if (result != m_sigCallbackMapping.end())
    {
        SWSS_LOG_DEBUG("call python signal handler for signal: %d.",signalNumber);
        result->second->onSignal(signalNumber);
    }
}

bool SignalHandlerHelper::checkSignal(int signalNumber)
{
    auto result = m_signalStatusMapping.find(signalNumber);
    if (result != m_signalStatusMapping.end())
    {
        return result->second;
    }

    return false;
}

void SignalHandlerHelper::resetSignal(int signalNumber)
{
    m_signalStatusMapping[signalNumber] = false;
}