#include "isrhelper.h"

uint64_t backup_rax[CORENUM];
uint64_t backup_rbx[CORENUM];
uint64_t backup_rcx[CORENUM];
uint64_t backup_rdx[CORENUM];
uint64_t backup_rsi[CORENUM];
uint64_t backup_rdi[CORENUM];
uint64_t backup_rsp[CORENUM];
uint64_t backup_rbp[CORENUM];
uint64_t backup_r8[CORENUM];
uint64_t backup_r9[CORENUM];
uint64_t backup_r10[CORENUM];
uint64_t backup_r11[CORENUM];
uint64_t backup_r12[CORENUM];
uint64_t backup_r13[CORENUM];
uint64_t backup_r14[CORENUM];
uint64_t backup_r15[CORENUM];
uint64_t backup_rflags[CORENUM];
uint64_t backup_rip[CORENUM];
uint64_t backup_cs[CORENUM];
uint64_t backup_ds[CORENUM];
uint64_t backup_es[CORENUM];
uint64_t backup_fs[CORENUM];
uint64_t backup_gs[CORENUM];
uint64_t backup_ss[CORENUM];
uint64_t backup_cr0[CORENUM];
uint64_t backup_cr2[CORENUM];
uint64_t backup_cr3[CORENUM];
uint64_t backup_cr4[CORENUM];
uint64_t backup_cr8[CORENUM];
IDTR backup_idtr[CORENUM];
GDTR backup_gdtr[CORENUM];
GDTR backup_ldtr[CORENUM];
uint64_t backup_tr[CORENUM];

IDT_GATE_DESCRIPTOR old_idt_table_gates_cpu0[256];
__attribute__((aligned(0x1000))) IDT_GATE_DESCRIPTOR new_idt_table_gates_cpu0[256];
uint64_t old_handlers_cpu0[256];
uint64_t origin_core0_isr_addr;

IDTR backupidtrs[CORENUM];
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

void backup_idt_gate_descriptors(unsigned char core)
{
    IDTR idtr;
    __asm__ volatile("sidt %0"
                     : "=m"(idtr));
    printk(" core %d idtr.base 0x%llx, idtr.limit 0x%x \n", core, idtr.base, idtr.limit);
    switch (core)
    {
    case 0:
    {
        memcpy((void *)&old_idt_table_gates_cpu0, (void *)idtr.base, idtr.limit + 1); //memory start: base, memory end: base+limit, size= limit + 1
        memcpy((void *)&new_idt_table_gates_cpu0, (void *)idtr.base, idtr.limit + 1);
        break;
    }
    default:
        break;
    }
    memcpy((void *)&backupidtrs[core], (void *)&idtr, sizeof(idtr));
}

void restore_idtrs(unsigned char core)
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

void hook_core0_isr_c_fun(void)
{
    unsigned char core;
    core = get_core_initial_apic_id();
    hook_flag = 1;

    outb(0x60, 0x80);
}

void register_hook_isr(unsigned char core, hook_isr_fun_type hookisr, unsigned char vector)
{
    uint64_t idtgatedes_addr = 0;
    IDT_GATE_DESCRIPTOR idtgatedes;
    IDTR newidtr;
    uint64_t handler_addr;
    switch (core)
    {

    case 0:
    {
        idtgatedes_addr = (uint64_t)((char*)new_idt_table_gates_cpu0 + vector * sizeof(IDT_GATE_DESCRIPTOR));
        //idtgatedes_addr = (uint64_t)&new_idt_table_gates_cpu0[vector];
        printk("idtgatedes_addr is 0x%llx\n", idtgatedes_addr);
        memcpy((void *)&idtgatedes, (void *)idtgatedes_addr, sizeof(IDT_GATE_DESCRIPTOR));
        //find origin isr address
        origin_core0_isr_addr = idt_descriptor_offset_to_va(idtgatedes);
        printk("origin_isr_addr 0x%llx \n",origin_core0_isr_addr);
        //use new idt table to get new idtr
        newidtr.base = (uint64_t)new_idt_table_gates_cpu0;
        newidtr.limit = backupidtrs[0].limit;
        break;
    }
    default:
        break;
    }

    //use new isr address to get new idt gate descriptor
    handler_addr = (uint64_t)hookisr;
    printk("handler_addr address 0x%llx\n", handler_addr);
    va_to_idt_descriptor_offset(handler_addr, &idtgatedes);
    printk("offset0_15 0x%x, offset16_31 0x%x, offset32_63 0x%x, access 0x%x, unused 0x%x, selector 0x%x,", idtgatedes.offset0_15, \
    idtgatedes.offset16_31,\
     idtgatedes.offset32_63,idtgatedes.access, \
     idtgatedes.unused,idtgatedes.selector);
    //new idt gate descriptor join in the new idt table
    memcpy((void *)idtgatedes_addr, (void *)&idtgatedes, sizeof(IDT_GATE_DESCRIPTOR));
     
    __asm__ volatile("lidt %0"
                     :
                     : "m"(newidtr)
                     : "memory");
                     
    printk("isrhook hook  core %d isr vector 0x%x  yes\n", core, vector);
}