#include <cassert>
#include <cstring>
#include <signalhandler.h>

using namespace swss;

bool SignalHandler::m_initialized = false;

int SignalHandler::m_happened[RTSIG_MAX];

void SignalHandler::OnSignal(int signalNumber)
{
    assert(signalNumber < RTSIG_MAX);
    TryInitialize();
    m_happened[signalNumber] = true;
}

void SignalHandler::Register(int signalNumber)
{
    assert(signalNumber < RTSIG_MAX);
    TryInitialize();
    signal(signalNumber, SignalHandler::OnSignal);
}

bool SignalHandler::Check(int signalNumber)
{
    assert(signalNumber < RTSIG_MAX);
    TryInitialize();
    return m_happened[signalNumber];
}

void SignalHandler::Reset(int signalNumber)
{
    assert(signalNumber < RTSIG_MAX);
    m_happened[signalNumber] = false;
}

void SignalHandler::TryInitialize()
{
    if (m_initialized)
    {
        return;
    }
    
    memset(m_happened, 0, sizeof(m_happened));
}



