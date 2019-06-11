#include "pagehelper.h"

/*first allocate page*/
/*modify current process page table and remap physical address*/
/*page table entry save physical address, one entry 8 byte*/
/*page start va and va mapped last pte's va */
/*last page table entry value map physical page*/
int allocate_kernel_continuous_4k_pages(struct tagMyPageHelper *page_helper)
{
    const uint64_t mask_5112 = 0xfffffffff000ull;
    const uint64_t mask_pml4 = 0xff8000000000ull;
    const uint64_t mask_pdpt = 0x007fc0000000ull;
    const uint64_t mask_pde = 0x00003fe00000ull;
    const uint64_t mask_pte = 0x0000001ff000ull;

    uint64_t vaaddress;
    uint64_t pml4paaddr, pdptpaaddr, pdepaaddr, ptepaaddr;
    uint64_t pml4val, pdptval, pdeval, pteval;
    uint64_t cr3, tmppa, tmpva;
    cr3 = __read_cr3();
    printk(KERN_ALERT "ishook current cr3 is 0x%016llx \n", cr3);
    /*kernel page 4k GFP_ATOMIC,line mapping, can use in the interrupt, */
    /*use the virtual page pte remap any target physical address*/
    /*no write and need wrie a data or modify attr*/
    if (page_helper->order == 0)
    {
        vaaddress = (uint64_t)(uint64_t *)__get_free_page(GFP_ATOMIC);
    }
    else if (page_helper->order > 0)
    {
        vaaddress = (uint64_t)(uint64_t *)__get_free_pages(GFP_ATOMIC, page_helper->order);
    }
    else
    {
        printk(KERN_ALERT "ishook get_gree_page fail\n");
        return 1;
    }

    printk(KERN_ALERT "isrhook allocate continuous 4k pages virtual address is 0x%016llx \n", vaaddress);

    tmppa = (uint64_t)(uint64_t *)virt_to_phys((void *)vaaddress);
    printk(KERN_ALERT "isrhook allocate continuous 4k pages physical address is 0x%016llx \n", tmppa);

    pml4paaddr = (cr3 & mask_5112) + ((vaaddress & mask_pml4) >> 39) * 8;
    tmpva = (uint64_t)(uint64_t *)phys_to_virt((phys_addr_t)pml4paaddr);
    pml4val = *(uint64_t *)tmpva;
    *(uint64_t *)tmpva |= 0x7;
    pml4val = *(uint64_t *)tmpva;

    pdptpaaddr = (pml4val & mask_5112) + ((vaaddress & mask_pdpt) >> 30) * 8;
    tmpva = (uint64_t)(uint64_t *)phys_to_virt((phys_addr_t)pdptpaaddr);
    pdptval = *(uint64_t *)tmpva;
    *(uint64_t *)tmpva |= 0x7;
    pdptval = *(uint64_t *)tmpva;

    pdepaaddr = (pdptval & mask_5112) + ((vaaddress & mask_pde) >> 21) * 8;
    tmpva = (uint64_t)(uint64_t *)phys_to_virt((phys_addr_t)pdepaaddr);
    pdeval = *(uint64_t *)tmpva;
    *(uint64_t *)tmpva |= 0x7;
    pdeval = *(uint64_t *)tmpva;

    if (pdeval & (1 << 7))
    {
        printk(KERN_ALERT "isrhook allocate a 2M page\n");
        page_helper->flag2Mpage = 1;
    }
    else
    {
        ptepaaddr = (pdeval & mask_5112) + ((vaaddress & mask_pte) >> 12) * 8;
        tmpva = (uint64_t)(uint64_t *)phys_to_virt((phys_addr_t)ptepaaddr);
        pteval = *(uint64_t *)tmpva;
        *(uint64_t *)tmpva |= 0x7;
        pteval = *(uint64_t *)tmpva;
        printk(KERN_ALERT "isrhook allocate a 4k page pte val is 0x%016llx \n", pteval);
        page_helper->flag2Mpage = 0;
    }
    
    page_helper->allocate_page_start_va = vaaddress;
    page_helper->allocate_page_last_level_entry_va = tmpva;
    printk(KERN_ALERT "isrhook allocate continuous page pte or pde (last level page table) virtual address is %016llx \n", tmpva);
    return 0;
}

void free_continuous_page(struct tagMyPageHelper page_helper)
{
    if(page_helper.order == 0)
    {
         free_page(page_helper.allocate_page_start_va);
    }
   else if(page_helper.order > 0)
   {
       free_pages(page_helper.allocate_page_start_va, page_helper.order);
   }
    
}

uint32_t read_mmio_32bit_by_pagehelper(uint64_t mmio_pa, struct tagMyPageHelper page_helper)
{
    uint32_t data;
    uint64_t mmio_maped_va;
    uint64_t page_table_last_level_entry_value;
    uint64_t pa_maped_page_table_entry_value;
    page_table_last_level_entry_value =  *(uint64_t*)page_helper.allocate_page_last_level_entry_va; // save original value

    if(page_helper.flag2Mpage == 1) 
    {
         pa_maped_page_table_entry_value = (mmio_pa & ~0x1fffffull) | 0x1e7; //map pa 2m page
    }
    else  
    {
        pa_maped_page_table_entry_value = (mmio_pa & ~0xfffull) | 0x1e7; //map pa 4k page 
    }
    
   
     *(uint64_t*)page_helper.allocate_page_last_level_entry_va = pa_maped_page_table_entry_value;

    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");

    if(page_helper.flag2Mpage ==  1)
    {
        mmio_maped_va = (page_helper.allocate_page_start_va & ~0x1fffffull) + (mmio_pa & 0x1fffffull); //frame + offset;
    }
    else
    {
        mmio_maped_va = (page_helper.allocate_page_start_va & ~0xfffull) + (mmio_pa & 0xfffull); //frame + offset;
    }
    

    data = *(uint32_t*)mmio_maped_va;  //read data

    //restore 
    *(uint64_t*)page_helper.allocate_page_last_level_entry_va = page_table_last_level_entry_value;

    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");

    return data;
}

uint32_t write_mmio_32bit_by_pagehelper(uint64_t mmio_pa, uint32_t data,struct tagMyPageHelper page_helper)
{
    uint64_t mmio_maped_va;
    uint64_t page_table_last_level_entry_value;
    uint64_t pa_maped_page_table_entry_value;
    page_table_last_level_entry_value =  *(uint64_t*)page_helper.allocate_page_last_level_entry_va; // save original value

    if(page_helper.flag2Mpage == 1) 
    {
         pa_maped_page_table_entry_value = (mmio_pa & ~0x1fffffull) | 0x1e7; //map pa 2m page
    }
    else  
    {
        pa_maped_page_table_entry_value = (mmio_pa & ~0xfffull) | 0x1e7; //map pa 4k page 
    }
    
   
     *(uint64_t*)page_helper.allocate_page_last_level_entry_va = pa_maped_page_table_entry_value;

    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");

    if(page_helper.flag2Mpage ==  1)
    {
        mmio_maped_va = (page_helper.allocate_page_start_va & ~0x1fffffull) + (mmio_pa & 0x1fffffull); //frame + offset;
    }
    else
    {
        mmio_maped_va = (page_helper.allocate_page_start_va & ~0xfffull) + (mmio_pa & 0xfffull); //frame + offset;
    }
    

    *(uint32_t*)mmio_maped_va = data;  //write data

    //restore 
    *(uint64_t*)page_helper.allocate_page_last_level_entry_va = page_table_last_level_entry_value;

    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");

    return data;
}

/*max size is 4k or 2M pagesize*/
void read_physical_address_by_pagehelper(uint64_t physicaladdress, size_t size, char* buf, struct tagMyPageHelper page_helper)
{
    uint64_t physical_address_va;
    uint64_t dest;
    size_t count;
    uint64_t page_table_last_level_entry_value;
    uint64_t pa_maped_page_table_entry_value;
    page_table_last_level_entry_value =  *(uint64_t*)page_helper.allocate_page_last_level_entry_va; // save original value

    if(page_helper.flag2Mpage == 1) 
    {
         pa_maped_page_table_entry_value = (physicaladdress & ~0x1fffffull) | 0x1e7; //map pa 2m page
    }
    else  
    {
        pa_maped_page_table_entry_value = (physicaladdress & ~0xfffull) | 0x1e7; //map pa 4k page 
    }
       
    *(uint64_t*)page_helper.allocate_page_last_level_entry_va = pa_maped_page_table_entry_value;

    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");

    if(page_helper.flag2Mpage ==  1)
    {
        physical_address_va = (page_helper.allocate_page_start_va & ~0x1fffffull) + (physicaladdress & 0x1fffffull); //frame + offset;
    }
    else
    {
        physical_address_va = (page_helper.allocate_page_start_va & ~0xfffull) + (physicaladdress & 0xfffull); //frame + offset;
    }

    dest = (uint64_t)buf;
    if(page_helper.flag2Mpage == 1)
    {
        if (size > 0x200000)
        {
            count = 0x200000;
        }
        else
        {
            count = size;
        }
        
    }
    else
    {
        if (size > 0x1000)
        {
            count = 0x1000;
        }
        else
        {
            count = size;
        }
    }
    
    memcpy((char*)dest, (char*)physical_address_va, count);

        //restore 
    *(uint64_t*)page_helper.allocate_page_last_level_entry_va = page_table_last_level_entry_value;
    __asm__ volatile ("invlpg (%0)" : :"r"(page_helper.allocate_page_start_va) : "memory");
}


/*use ioremap and writel reaal*/
uint32_t read_mmio_32bit_by_ioremap(uint64_t mmiopa)
{
    uint32_t data;
    void __iomem *remapva;
    remapva = ioremap(mmiopa,sizeof(char)*4);
    data = readl(remapva);
    iounmap(remapva);
    return data;
}
void write_mmio_32bit_by_ioremap(uint64_t mmiopa, uint32_t data)
{
    void __iomem *remapva;
    remapva = ioremap(mmiopa,sizeof(char)*4);
    writel(data,remapva);
    iounmap(remapva);
    return ;
}

