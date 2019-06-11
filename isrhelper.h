#ifndef __ISRHELPER_H__
#define __ISRHELPER_H__

#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/module.h>

/* x86\include\asm\processor.h  cpuid(. . . . . ) */
void cpuid_helper(uint32_t id, uint32_t *buff);
#endif