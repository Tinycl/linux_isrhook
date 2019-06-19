#include "isrhelper.h"

//registers
uint64_t g_backup_rax[CORENUM];
uint64_t g_backup_rbx[CORENUM];
uint64_t g_backup_rcx[CORENUM];
uint64_t g_backup_rdx[CORENUM];
uint64_t g_backup_rsi[CORENUM];
uint64_t g_backup_rdi[CORENUM];
uint64_t g_backup_rsp[CORENUM];
uint64_t g_backup_rbp[CORENUM];
uint64_t g_backup_r8[CORENUM];
uint64_t g_backup_r9[CORENUM];
uint64_t g_backup_r10[CORENUM];
uint64_t g_backup_r11[CORENUM];
uint64_t g_backup_r12[CORENUM];
uint64_t g_backup_r13[CORENUM];
uint64_t g_backup_r14[CORENUM];
uint64_t g_backup_r15[CORENUM];
uint64_t g_backup_rflags[CORENUM];
uint64_t g_backup_rip[CORENUM];
uint64_t g_backup_cs[CORENUM];
uint64_t g_backup_ds[CORENUM];
uint64_t g_backup_es[CORENUM];
uint64_t g_backup_fs[CORENUM];
uint64_t g_backup_gs[CORENUM];
uint64_t g_backup_ss[CORENUM];
uint64_t g_backup_cr0[CORENUM];
uint64_t g_backup_cr2[CORENUM];
uint64_t g_backup_cr3[CORENUM];
uint64_t g_backup_cr4[CORENUM];
uint64_t g_backup_cr8[CORENUM];
IDTR g_backup_idtr[CORENUM];
GDTR g_backup_gdtr[CORENUM];
GDTR g_backup_ldtr[CORENUM];
uint64_t g_backup_tr[CORENUM];

//mtrr

const uint32_t mtrr_msrs_index[29] = {
    0x200, 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b, 0x20c, 0x20d, 0x20e, 0x20f,
    0x250, 0x258, 0x259, 0x268, 0x269, 0x26a, 0x26b, 0x26c, 0x26d, 0x26e, 0x26f, 0x277, 0x2ff

};

const uint32_t sysenter_msrs_index[3] = { 0x174 /*cs */, 0x175 /*esp */, 0x176 /*eip */};
const uint32_t effer_msr_index = 0xc0000080;
const uint32_t star_msr_index = 0xc0000081;
const uint32_t lstar_msr_index = 0xc0000082;
const uint32_t fmask_msr_index = 0xc0000084;
const uint32_t fs_base_msr_index = 0xc0000100;
const uint32_t gs_base_msr_index = 0xc0000101;
const uint32_t kernel_gs_base_msr_index = 0xc0000102;
const uint32_t cstar_msr_index = 0xc0000083;


__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core0[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core1[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core2[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core3[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core4[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core5[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core6[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR g_new_idt_table_gates_core7[256];


uint64_t g_origin_core0_isr_addr;
uint64_t g_origin_core1_isr_addr;
uint64_t g_origin_core2_isr_addr;
uint64_t g_origin_core3_isr_addr;
uint64_t g_origin_core4_isr_addr;
uint64_t g_origin_core5_isr_addr;
uint64_t g_origin_core6_isr_addr;
uint64_t g_origin_core7_isr_addr;


IDTR g_backup_idtrs[CORENUM];

unsigned char backup_port70data;

unsigned char hook_flag = 0;
void cpuid_helper(uint32_t id, uint32_t *buff)
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t in = id;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "0"(in));
    *buff = eax;
    *(buff + 1) = ebx;
    *(buff + 2) = ecx;
    *(buff + 3) = edx;
}

unsigned char get_core_initial_apic_id(void)
{
    unsigned char apicid;
    uint32_t buff[4];
    cpuid_helper(1, buff);
    apicid = (unsigned char)(buff[1] >> 24);
    return apicid;
}

uint64_t idt_descriptor_offset_to_va(IDT_GATE_DESCRIPTOR idt_gate)
{
    uint64_t va;
    va = (((uint64_t)idt_gate.offset32_63) << 32) | (((uint64_t)idt_gate.offset16_31) << 16) | (idt_gate.offset0_15);
    return va;
}

void va_to_idt_descriptor_offset(uint64_t va, IDT_GATE_DESCRIPTOR *pidat_gate)
{
    pidat_gate->offset0_15 = (uint16_t)(va & 0xffff);
    pidat_gate->offset16_31 = (uint16_t)((va >> 16) & 0xffff);
    pidat_gate->offset32_63 = (uint32_t)((va >> 32) & 0xffffffff);
}

uint8_t get_idt_descriptor_dpl(IDT_GATE_DESCRIPTOR idt_gate)
{
    uint8_t dpl;
    dpl = (uint8_t)((idt_gate.access & 0x60) >> 4);
    return dpl;
}

void set_idt_descriptor_dpl(uint8_t dpl, IDT_GATE_DESCRIPTOR *pidt_gate)
{
    pidt_gate->access &= 0x9f;
    pidt_gate->access |= (dpl << 5);
}

uint8_t get_idt_descriptor_present(IDT_GATE_DESCRIPTOR idt_gate)
{
    uint8_t present;
    present = (uint8_t)(idt_gate.access >> 7);
    return present;
}

void set_idt_descriptor_present(IDT_GATE_DESCRIPTOR *pidt_gate)
{

    pidt_gate->access |= 0x80;
}

uint8_t get_idt_descriptor_type(IDT_GATE_DESCRIPTOR idt_gate)
{
    uint8_t type;
    type = (uint8_t)(idt_gate.access & 0xf0);
    return type;
}

uint8_t get_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR idt_gate)
{
    uint8_t rpl;
    rpl = (uint8_t)(idt_gate.selector & 0x3);
    return rpl;
}

void set_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR *pidt_gate, uint8_t rpl)
{

    pidt_gate->selector &= 0xffc;
    pidt_gate->selector |= rpl;
}

void backup_and_new_idt_gate_descriptors(unsigned char core, IDT_GATE_DESCRIPTOR *new_idt_table_gates, IDTR *backupidtrs)
{
    IDTR idtr;
    __asm__ volatile("sidt %0"
                     : "=m"(idtr));
    printk(" core %d idtr.base 0x%llx, idtr.limit 0x%x \n", core, idtr.base, idtr.limit);

    //memory start: base, memory end: base+limit, size= limit + 1
    memcpy((void *)new_idt_table_gates, (void *)idtr.base, idtr.limit + 1);
    //backup idtr
    memcpy((void *)&backupidtrs[core], (void *)&idtr, sizeof(idtr));  
    printk("backup and new idt table %d \n", core);
}

void restore_idtrs(unsigned char core, IDTR *backupidtrs)
{
    IDTR idtr;
    memcpy((void *)&idtr, (void *)&backupidtrs[core], sizeof(IDTR));
    __asm__ volatile("lidt %0"
                     :
                     : "m"(idtr));
    printk("restore core %d idtr.base 0x%llx, idtr.limit 0x%x \n", core, idtr.base, idtr.limit);
}


void save_rtc(void)
{
    unsigned char data;
    backup_port70data = inb(0x70);
    outb(0xb, 0x70); //index 0xb
    data = inb(0x71);
    data |= 0x80; //disable updating
    outb(data, 0x71);
}
void restore_rtc(void)
{
    unsigned char data;
    outb(0xb, 0x70);
    data = inb(0x71);
    data &= 0x7f; //enable updating
    outb(data, 0x71);
    outb(backup_port70data, 0x70);
}


void save_hpet(struct tagMyPageHelper page_helper)
{
    unsigned int data;
    data = read_mmio_32bit_by_pagehelper(HPETBASE + HPET_CFG, page_helper);
    data &= 0x2; //disable hpet update
    write_mmio_32bit_by_pagehelper(HPETBASE + HPET_CFG, data, page_helper);
}

void restore_hpet(struct tagMyPageHelper page_helper)
{
    unsigned int data;
    data = read_mmio_32bit_by_pagehelper(HPETBASE + HPET_CFG, page_helper);
    data |= 0x1; //enable hpet update
    write_mmio_32bit_by_pagehelper(HPETBASE + HPET_CFG, data, page_helper);
}

uint64_t save_tsc(void)
{
    return rdtsc();
}
void restore_tsc(uint64_t tsc)
{
    unsigned int low, high;
    low = (unsigned int)tsc;
    high = (unsigned int)(tsc >> 32);
    native_write_msr(0x10, low, high);
}

// hook isr call this fun, so what are you want to do in this fun
// but can't sleep, you know it is now in the interrupt handler 
void user_isr_c_fun(void)
{
    unsigned char core;
    core = get_core_initial_apic_id();
    switch(core)
    {
        case 0:
        {
            hook_flag = 1;
            outb(0x60, 0x80);
            break;
        }
        case 1:
        {
            hook_flag = 1;
            outb(0x61, 0x80);
            break;
        }
        case 2:
        {
            hook_flag = 1;
            outb(0x62, 0x80);
            break;
        } 
        case 3:
        {
            hook_flag = 1;
            outb(0x63, 0x80);
            break;
        }
        case 4:
        {
            hook_flag = 1;
            outb(0x64, 0x80);
            break;
        }
        case 5:
        {
            hook_flag = 1;
            outb(0x65, 0x80);
            break;
        }
        case 6:
        {
            hook_flag = 1;
            outb(0x66, 0x80);
            break;
        } 
        case 7:
        {
            hook_flag = 1;
            outb(0x67, 0x80);
            break;
        }     
        default:
            break;
    }
   
}

void hook_one_core_isr(unsigned char core, hook_isr_fun_type hookisr, unsigned char vector,\
                       IDT_GATE_DESCRIPTOR *new_idt_table_gates, IDTR *backupidtrs, uint64_t *origin_isr_addr)
{
    uint64_t idtgatedes_addr = 0;
    IDT_GATE_DESCRIPTOR idtgatedes;
    IDTR newidtr;
    uint64_t handler_addr;

    //idtgatedes_addr = (uint64_t)((char*)g_new_idt_table_gates_core0 + vector * sizeof(IDT_GATE_DESCRIPTOR));
    idtgatedes_addr = (uint64_t)&new_idt_table_gates[vector];
    printk("idtgatedes_addr is 0x%llx\n", idtgatedes_addr);
    memcpy((void *)&idtgatedes, (void *)idtgatedes_addr, sizeof(IDT_GATE_DESCRIPTOR));
    //find origin isr address
    *origin_isr_addr = idt_descriptor_offset_to_va(idtgatedes);
    printk("origin_isr_addr 0x%llx \n",*origin_isr_addr);
    //use new idt table to get new idtr
    newidtr.base = (uint64_t)new_idt_table_gates;
    newidtr.limit = backupidtrs[core].limit;
    //use new isr address to get new idt gate descriptor
    handler_addr = (uint64_t)hookisr;
    printk("handler_addr address 0x%llx\n", handler_addr);
    va_to_idt_descriptor_offset(handler_addr, &idtgatedes);
    printk("offset0_15 0x%x, offset16_31 0x%x, offset32_63 0x%x, access 0x%x, unused 0x%x, selector 0x%x,",\
            idtgatedes.offset0_15, \
            idtgatedes.offset16_31,\
            idtgatedes.offset32_63,\
            idtgatedes.access, \
            idtgatedes.unused,\
            idtgatedes.selector);
    //new idt gate descriptor join in the new idt table
    memcpy((void *)idtgatedes_addr, (void *)&idtgatedes, sizeof(IDT_GATE_DESCRIPTOR));
     
    __asm__ volatile("lidt %0"
                     :
                     : "m"(newidtr)
                     : "memory");
                     
    printk("isrhook hook  core %d isr vector 0x%x  yes\n", core, vector);
}

void register_hook(void)
{
    unsigned char core;
    core = get_core_initial_apic_id();
    switch(core)
    {
        case 0:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core0, g_backup_idtrs);
            hook_one_core_isr(core, hook_core0_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core0, g_backup_idtrs, &g_origin_core0_isr_addr);
            break;
        }
        case 1:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core1, g_backup_idtrs);
            hook_one_core_isr(core, hook_core1_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core1, g_backup_idtrs, &g_origin_core1_isr_addr);
            break;
        }
        case 2:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core2, g_backup_idtrs);
            hook_one_core_isr(core, hook_core2_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core2, g_backup_idtrs, &g_origin_core2_isr_addr);
            break;
        }
        case 3:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core3, g_backup_idtrs);
            hook_one_core_isr(core, hook_core3_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core3, g_backup_idtrs, &g_origin_core3_isr_addr);
            break;
        }
        case 4:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core4, g_backup_idtrs);
            hook_one_core_isr(core, hook_core4_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core4, g_backup_idtrs, &g_origin_core4_isr_addr);
            break;
        }
        case 5:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core5, g_backup_idtrs);
            hook_one_core_isr(core, hook_core5_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core5, g_backup_idtrs, &g_origin_core5_isr_addr);
            break;
        }
        case 6:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core6, g_backup_idtrs);
            hook_one_core_isr(core, hook_core6_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core6, g_backup_idtrs, &g_origin_core6_isr_addr);
            break;
        }
        case 7:
        {
            backup_and_new_idt_gate_descriptors(core, g_new_idt_table_gates_core7, g_backup_idtrs);
            hook_one_core_isr(core, hook_core7_isr_asm_fun, APICTIME_VECTOR, g_new_idt_table_gates_core7, g_backup_idtrs, &g_origin_core7_isr_addr);
            break;
        }
        default:
            break;
    }
     printk("core is 0x%x\n",core);
}

void unregister_hook(void)
{
    unsigned char core;
    core = get_core_initial_apic_id();
    restore_idtrs(core, g_backup_idtrs);
}