#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <x86intrin.h> // __rdtsc(), __cpuid()
#include <unistd.h>
#include <sys/syscall.h>

unsigned long long measure_cycles(); // get the start cycle
unsigned long long measure_cycles_end(); // get the end cycle
void pin_thread_to_cpu(int cpu); // fix program on the same CPU
