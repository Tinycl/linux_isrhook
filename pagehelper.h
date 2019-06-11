#ifndef __PAGEHELPER_H__
#define __PAGEHELPER_H__

#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <asm/io.h>

#define set_bit(data,n) (data |= 1<<n)
#define clear_bit(data,n) (data &= ~(1<<n))
#define get_bit(data,n) (data&(1<<n)) // get n bit 
#define cpl_bit(data,n) (data^=(1<<n)) // n bit reverse
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