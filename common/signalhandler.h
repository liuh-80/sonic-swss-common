#pragma once
#include <limits.h>
#include <signal.h>

namespace swss {

class SignalHandler
{
public:
    static void OnSignal(int signalNumber);
    static void Register(int signalNumber);
    static bool Check(int signalNumber);
    static void Reset(int signalNumber);

private:
    static void TryInitialize();
    
    static bool m_initialized;
    static int m_happened[RTSIG_MAX];
};

}