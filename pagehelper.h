#ifndef __PAGEHELPER_H__
#define __PAGEHELPER_H__

#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <asm/io.h>
#include "typehelper.h"

#define SET_BIT(data,n) (data |= 1<<n)//data: 0xaa=1010 1010 -> n:0 0xab
#define CLEAR_BIT(data,n) (data &= ~(1<<n)) //data: 0xaa=1010 1010 -> n:1 0xa8
#define GET_BIT(data,n) (data&(1<<n)) //data: 0xaa= 1010 1010 -> n:0 0x0; n:1 0x2; n:3 0x8
#define NOT_BIT(data,n) (data^=(1<<n)) //data: 0xaa= 1010 1010 -> n:1 0xa8
struct tagMyPageHelper
{
    unsigned int order; //how many 4k page
    char flag2Mpage;  // is 4k page or 2m page
    uint64_t allocate_page_start_va; //page start address va
    uint64_t allocate_page_last_level_entry_va; //*va value point physical page 
};

int allocate_kernel_continuous_4k_pages(struct tagMyPageHelper *page_helper);
void free_continuous_page(struct tagMyPageHelper page_helper);
uint32_t read_mmio_32bit_by_pagehelper(uint64_t mmio_pa, struct tagMyPageHelper page_helper);
uint32_t write_mmio_32bit_by_pagehelper(uint64_t mmio_pa, uint32_t data,struct tagMyPageHelper page_helper);
void read_physical_address_by_pagehelper(uint64_t physicaladdress, size_t size, char* buf, struct tagMyPageHelper page_helper);
uint32_t read_mmio_32bit_by_ioremap(uint64_t mmiopa);
void write_mmio_32bit_by_ioremap(uint64_t mmiopa, uint32_t data);
#endif