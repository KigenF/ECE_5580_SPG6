#include "cycles.h"

unsigned long long measure_cycles() {
    unsigned int hi, lo;
    __asm__ __volatile__ (
        "cpuid\n\t"            // Serialize before rdtsc
        "rdtsc\n\t"            // Read TSC
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        : "=r" (hi), "=r" (lo)
        :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((unsigned long long)hi << 32) | lo;
}

unsigned long long measure_cycles_end() {
    unsigned int hi, lo;
    __asm__ __volatile__ (
        "rdtscp\n\t"           // Read TSC + serialize
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"            // Serialize after rdtscp
        : "=r" (hi), "=r" (lo)
        :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((unsigned long long)hi << 32) | lo;
}

void pin_thread_to_cpu(int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
    }
}
