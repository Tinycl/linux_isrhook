#include "isrhelper.h"

void cpuid_helper(uint32_t id, uint32_t *buff)
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t in = 0;
    __asm__ volatile ("cpuid":"=a"(eax),"=b"(ebx),"=c"(ecx),"=d"(edx):"0"(in));
    *buff = eax;
    *(buff+1) = ebx;
    *(buff + 2) = ecx;
    *(buff + 3) = edx;
    printk(KERN_ALERT "cpuid %x : eax 0x%x, ebx 0x%x, ecx 0x%x, edx 0x%x \n", id, eax, ebx, ecx, edx);
}