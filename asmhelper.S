
.global save_fpu_mmx_xmm_mxcsr_state;
.global save_processor_extended_state;
.global hook_core0_isr_asm_fun;


.code64

.section .text


save_fpu_mmx_xmm_mxcsr_state:
    fxsave (%rdi);  #one param use rdi
    retq;


save_processor_extended_state:
    # we want to use rax rbx rcx rdx, so we save first
    pushq %rax;
    pushq %rbx;
    pushq %rcx;
    pushq %rdx;

    movq $0x0d, %rax; # processor extended state enumeration main leaf eax=0x0d, ecx=0
    xorq %rcx, %rcx;
    cpuid; # cpuid input eax,ecx; output eax,ebx,ecx,edx
    movq %rax, %rbx;
    movq $0, %rcx;
    xgetbv; # reads an XCR specified by ecx into edx:eax
    orq %rbx,%rax;
    xsetbv; # write edx:eax to the XCR specified ecx
    mov $0x7, %rax;
    mov $0x0, %rdx;
    xsave (%rdi); # save state compoents specified by edx:eax to mem

    popq %rdx;
    popq %rcx;
    popq %rbx;
    popq %rax;
    retq;

.align 0x100;
hook_core0_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core0_label;

hook_core0_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (backup_rax); // () gas address not immediate
    mov %rbx, (backup_rbx);
    mov %rcx, (backup_rcx);
    mov %rdx, (backup_rdx);
    mov %rsi, (backup_rsi);
    mov %rdi, (backup_rdi);
    mov %rbp, (backup_rbp);
    mov %r8,  (backup_r8);
    mov %r9,  (backup_r9);
    mov %r10, (backup_r10);
    mov %r11, (backup_r11);
    mov %r12, (backup_r12);
    mov %r13, (backup_r13);
    mov %r14, (backup_r14);
    mov %r15, (backup_r15);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (backup_rip); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (backup_cs);
    movq 24(%rsp), %rax;
    movq %rax, (backup_rflags);
    movq 32(%rsp), %rax;
    movq %rax, (backup_rsp);
    movq 40(%rsp), %rax;
    movq %rax, (backup_ss);

    mov %ds, (backup_ds);
    mov %es, (backup_es);
    mov %fs, (backup_fs);
    mov %gs, (backup_gs);

    mov %cr0, %rax;
    mov %rax, (backup_cr0);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (backup_cr2);
    mov %cr3, %rax;
    mov %rax, (backup_cr3);
    mov %cr4, %rax;
    mov %rax, (backup_cr4);
    mov %cr8, %rax;
    mov %rax, (backup_cr8);

    sgdt (backup_gdtr);
    sldt (backup_ldtr);
    sidt (backup_idtr);
    str  (backup_tr);

    #call hook isr

    callq hook_core0_isr_c_fun;

    #restore 
    mov (backup_r15), %r15;
    mov (backup_r14), %r14;
    mov (backup_r13), %r13;
    mov (backup_r12), %r12;
    mov (backup_r11), %r11;
    mov (backup_r10), %r10;
    mov (backup_r9),  %r9;
    mov (backup_r8),  %r8;
    mov (backup_rbp), %rbp;
    mov (backup_rdi), %rdi;
    mov (backup_rsi), %rsi;
    mov (backup_rdx), %rdx;
    mov (backup_rcx), %rcx;
    mov (backup_rbx), %rbx;
    mov (backup_cr3), %rax;
    mov %rax, %cr3;
    mov (backup_cr0), %rax;
    mov %rax, %cr0;
    mov (backup_rax), %rax;

    #restore rflags
    popfq;
    jmpq *origin_core0_isr_addr(,1);  # go to origin isr 
