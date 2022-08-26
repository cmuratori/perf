#include <stdio.h>
#include <windows.h>

struct HARDWARE_COUNTER
{
    HARDWARE_COUNTER_TYPE Type;
    ULONG Reserved;
    ULONG64 Index;
};
typedef NTSTATUS keshcc_prototype(HARDWARE_COUNTER *CounterArray, ULONG Count);

int main(int ArgCount, char **Args)
{
    HANDLE Thread = GetCurrentThread();
    
    keshcc_prototype *KeSetHardwareCounterConfiguration = 0;
    HMODULE NTOS = LoadLibrary("NtosKrnl.exe");
    KeSetHardwareCounterConfiguration = (keshcc_prototype *)GetProcAddress(NTOS, "KeSetHardwareCounterConfiguration");
    if(KeSetHardwareCounterConfiguration)
    {
        HARDWARE_COUNTER Counters[4] =
        {
            {PMCCounter, 0,  2},
            {PMCCounter, 0,  6},
            {PMCCounter, 0, 10},
            {PMCCounter, 0, 11},
        };
        NTSTATUS KESHCCError = KeSetHardwareCounterConfiguration(Counters, 4);
        if(KESHCCError == 0)
        {
            HANDLE Perf;
            DWORD ETPError = EnableThreadProfiling(Thread, THREAD_PROFILING_FLAG_DISPATCH, 0xffff, &Perf);
            if(ETPError == ERROR_SUCCESS)
            {
                for(int Repeat = 0; Repeat < 10; ++Repeat)
                {
                    Sleep(1000);
                    
                    DWORD Flags = READ_THREAD_PROFILING_FLAG_DISPATCHING|READ_THREAD_PROFILING_FLAG_HARDWARE_COUNTERS;
                    PERFORMANCE_DATA PerfData = {};
                    PerfData.Size = sizeof(PerfData);
                    PerfData.Version = PERFORMANCE_DATA_VERSION;
                    
                    DWORD RTPDError = ReadThreadProfilingData(Perf, Flags, &PerfData);
                    if(RTPDError == ERROR_SUCCESS)
                    {
                        fprintf(stdout, "\n");
                        fprintf(stdout, "HwCountersCount: %u\n", PerfData.HwCountersCount);
                        fprintf(stdout, "ContextSwitchCount: %u\n", PerfData.ContextSwitchCount);
                        fprintf(stdout, "WaitReasonBitmap: %llu\n", PerfData.WaitReasonBitMap);
                        fprintf(stdout, "CycleTime: %llu\n", PerfData.CycleTime);
                        fprintf(stdout, "RetryCount: %u\n", PerfData.RetryCount);
                        for(int Index = 0; Index < sizeof(PerfData.HwCounters)/sizeof(PerfData.HwCounters[0]); ++Index)
                        {
                            HARDWARE_COUNTER_DATA Counter = PerfData.HwCounters[Index];
                            fprintf(stdout, "[%u] %u: %llu\n", Index, Counter.Type, Counter.Value);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "ReadThreadProfilingData returned %u.\n", RTPDError);
                    }
                }
                
                DisableThreadProfiling(Perf);
            }
            else
            {
                fprintf(stderr, "EnableThreadProfiling returned %u.\n", ETPError);
            }
        }
        else
        {
            fprintf(stderr, "KeSetHardwareCounterConfiguration returned %u.\n", KESHCCError);
        }
    }
    else
    {
        fprintf(stderr, "Unable to get function address.\n");
    }
    
    return 0;
}
