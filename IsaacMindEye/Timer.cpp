#include "pch.h"
#include "Timer.h"

void MainTimer()
{
    HANDLE hevent = ::CreateEvent(NULL, FALSE, FALSE, L"IsaacMainTimer");
    while (true)
    {
        ::SetEvent(hevent);
        ::Sleep(1000);
    }
}
