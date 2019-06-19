
.global save_fpu_mmx_xmm_mxcsr_state_asm_fun; # extern file can use 
.global save_processor_extended_state_asm_fun;
.global hook_core0_isr_asm_fun;
.global hook_core1_isr_asm_fun;
.global hook_core2_isr_asm_fun;
.global hook_core3_isr_asm_fun;
.global hook_core4_isr_asm_fun;
.global hook_core5_isr_asm_fun;
.global hook_core6_isr_asm_fun;
.global hook_core7_isr_asm_fun;

.code64

.section .text


save_fpu_mmx_xmm_mxcsr_state_asm_fun:
    fxsave (%rdi);  #one param use rdi
    retq;


save_processor_extended_state_asm_fun:
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
    mov %rax, (g_backup_rax); // () gas address not immediate
    mov %rbx, (g_backup_rbx);
    mov %rcx, (g_backup_rcx);
    mov %rdx, (g_backup_rdx);
    mov %rsi, (g_backup_rsi);
    mov %rdi, (g_backup_rdi);
    mov %rbp, (g_backup_rbp);
    mov %r8,  (g_backup_r8);
    mov %r9,  (g_backup_r9);
    mov %r10, (g_backup_r10);
    mov %r11, (g_backup_r11);
    mov %r12, (g_backup_r12);
    mov %r13, (g_backup_r13);
    mov %r14, (g_backup_r14);
    mov %r15, (g_backup_r15);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss);

    mov %ds, (g_backup_ds);
    mov %es, (g_backup_es);
    mov %fs, (g_backup_fs);
    mov %gs, (g_backup_gs);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8);

    sgdt (g_backup_gdtr);
    sldt (g_backup_ldtr);
    sidt (g_backup_idtr);
    str  (g_backup_tr);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15), %r15;
    mov (g_backup_r14), %r14;
    mov (g_backup_r13), %r13;
    mov (g_backup_r12), %r12;
    mov (g_backup_r11), %r11;
    mov (g_backup_r10), %r10;
    mov (g_backup_r9),  %r9;
    mov (g_backup_r8),  %r8;
    mov (g_backup_rbp), %rbp;
    mov (g_backup_rdi), %rdi;
    mov (g_backup_rsi), %rsi;
    mov (g_backup_rdx), %rdx;
    mov (g_backup_rcx), %rcx;
    mov (g_backup_rbx), %rbx;
    mov (g_backup_cr3), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core0_isr_addr(,1);  # go to origin isr 




.align 0x100;
hook_core1_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core1_label;

hook_core1_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8);
    mov %rcx, (g_backup_rcx+8);
    mov %rdx, (g_backup_rdx+8);
    mov %rsi, (g_backup_rsi+8);
    mov %rdi, (g_backup_rdi+8);
    mov %rbp, (g_backup_rbp+8);
    mov %r8,  (g_backup_r8+8);
    mov %r9,  (g_backup_r9+8);
    mov %r10, (g_backup_r10+8);
    mov %r11, (g_backup_r11+8);
    mov %r12, (g_backup_r12+8);
    mov %r13, (g_backup_r13+8);
    mov %r14, (g_backup_r14+8);
    mov %r15, (g_backup_r15+8);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8);

    mov %ds, (g_backup_ds+8);
    mov %es, (g_backup_es+8);
    mov %fs, (g_backup_fs+8);
    mov %gs, (g_backup_gs+8);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8);

    sgdt (g_backup_gdtr+8);
    sldt (g_backup_ldtr+8);
    sidt (g_backup_idtr+8);
    str  (g_backup_tr+8);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8), %r15;
    mov (g_backup_r14+8), %r14;
    mov (g_backup_r13+8), %r13;
    mov (g_backup_r12+8), %r12;
    mov (g_backup_r11+8), %r11;
    mov (g_backup_r10+8), %r10;
    mov (g_backup_r9+8),  %r9;
    mov (g_backup_r8+8),  %r8;
    mov (g_backup_rbp+8), %rbp;
    mov (g_backup_rdi+8), %rdi;
    mov (g_backup_rsi+8), %rsi;
    mov (g_backup_rdx+8), %rdx;
    mov (g_backup_rcx+8), %rcx;
    mov (g_backup_rbx+8), %rbx;
    mov (g_backup_cr3+8), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core1_isr_addr(,1);  # go to origin isr 




.align 0x100;
hook_core2_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core2_label;

hook_core2_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*2); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*2);
    mov %rcx, (g_backup_rcx+8*2);
    mov %rdx, (g_backup_rdx+8*2);
    mov %rsi, (g_backup_rsi+8*2);
    mov %rdi, (g_backup_rdi+8*2);
    mov %rbp, (g_backup_rbp+8*2);
    mov %r8,  (g_backup_r8+8*2);
    mov %r9,  (g_backup_r9+8*2);
    mov %r10, (g_backup_r10+8*2);
    mov %r11, (g_backup_r11+8*2);
    mov %r12, (g_backup_r12+8*2);
    mov %r13, (g_backup_r13+8*2);
    mov %r14, (g_backup_r14+8*2);
    mov %r15, (g_backup_r15+8*2);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*2); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*2);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*2);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*2);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*2);

    mov %ds, (g_backup_ds+8*2);
    mov %es, (g_backup_es+8*2);
    mov %fs, (g_backup_fs+8*2);
    mov %gs, (g_backup_gs+8*2);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*2);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*2);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*2);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*2);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*2);

    sgdt (g_backup_gdtr+8*2);
    sldt (g_backup_ldtr+8*2);
    sidt (g_backup_idtr+8*2);
    str  (g_backup_tr+8*2);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*2), %r15;
    mov (g_backup_r14+8*2), %r14;
    mov (g_backup_r13+8*2), %r13;
    mov (g_backup_r12+8*2), %r12;
    mov (g_backup_r11+8*2), %r11;
    mov (g_backup_r10+8*2), %r10;
    mov (g_backup_r9+8*2),  %r9;
    mov (g_backup_r8+8*2),  %r8;
    mov (g_backup_rbp+8*2), %rbp;
    mov (g_backup_rdi+8*2), %rdi;
    mov (g_backup_rsi+8*2), %rsi;
    mov (g_backup_rdx+8*2), %rdx;
    mov (g_backup_rcx+8*2), %rcx;
    mov (g_backup_rbx+8*2), %rbx;
    mov (g_backup_cr3+8*2), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*2), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*2), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core2_isr_addr(,1);  # go to origin isr 



.align 0x100;
hook_core3_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core3_label;

hook_core3_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*3); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*3);
    mov %rcx, (g_backup_rcx+8*3);
    mov %rdx, (g_backup_rdx+8*3);
    mov %rsi, (g_backup_rsi+8*3);
    mov %rdi, (g_backup_rdi+8*3);
    mov %rbp, (g_backup_rbp+8*3);
    mov %r8,  (g_backup_r8+8*3);
    mov %r9,  (g_backup_r9+8*3);
    mov %r10, (g_backup_r10+8*3);
    mov %r11, (g_backup_r11+8*3);
    mov %r12, (g_backup_r12+8*3);
    mov %r13, (g_backup_r13+8*3);
    mov %r14, (g_backup_r14+8*3);
    mov %r15, (g_backup_r15+8*3);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*3); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*3);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*3);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*3);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*3);

    mov %ds, (g_backup_ds+8*3);
    mov %es, (g_backup_es+8*3);
    mov %fs, (g_backup_fs+8*3);
    mov %gs, (g_backup_gs+8*3);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*3);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*3);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*3);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*3);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*3);

    sgdt (g_backup_gdtr+8*3);
    sldt (g_backup_ldtr+8*3);
    sidt (g_backup_idtr+8*3);
    str  (g_backup_tr+8*3);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*3), %r15;
    mov (g_backup_r14+8*3), %r14;
    mov (g_backup_r13+8*3), %r13;
    mov (g_backup_r12+8*3), %r12;
    mov (g_backup_r11+8*3), %r11;
    mov (g_backup_r10+8*3), %r10;
    mov (g_backup_r9+8*3),  %r9;
    mov (g_backup_r8+8*3),  %r8;
    mov (g_backup_rbp+8*3), %rbp;
    mov (g_backup_rdi+8*3), %rdi;
    mov (g_backup_rsi+8*3), %rsi;
    mov (g_backup_rdx+8*3), %rdx;
    mov (g_backup_rcx+8*3), %rcx;
    mov (g_backup_rbx+8*3), %rbx;
    mov (g_backup_cr3+8*3), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*3), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*3), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core3_isr_addr(,1);  # go to origin isr 



.align 0x100;
hook_core4_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core4_label;

hook_core4_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*4); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*4);
    mov %rcx, (g_backup_rcx+8*4);
    mov %rdx, (g_backup_rdx+8*4);
    mov %rsi, (g_backup_rsi+8*4);
    mov %rdi, (g_backup_rdi+8*4);
    mov %rbp, (g_backup_rbp+8*4);
    mov %r8,  (g_backup_r8+8*4);
    mov %r9,  (g_backup_r9+8*4);
    mov %r10, (g_backup_r10+8*4);
    mov %r11, (g_backup_r11+8*4);
    mov %r12, (g_backup_r12+8*4);
    mov %r13, (g_backup_r13+8*4);
    mov %r14, (g_backup_r14+8*4);
    mov %r15, (g_backup_r15+8*4);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*4); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*4);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*4);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*4);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*4);

    mov %ds, (g_backup_ds+8*4);
    mov %es, (g_backup_es+8*4);
    mov %fs, (g_backup_fs+8*4);
    mov %gs, (g_backup_gs+8*4);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*4);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*4);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*4);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*4);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*4);

    sgdt (g_backup_gdtr+8*4);
    sldt (g_backup_ldtr+8*4);
    sidt (g_backup_idtr+8*4);
    str  (g_backup_tr+8*4);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*4), %r15;
    mov (g_backup_r14+8*4), %r14;
    mov (g_backup_r13+8*4), %r13;
    mov (g_backup_r12+8*4), %r12;
    mov (g_backup_r11+8*4), %r11;
    mov (g_backup_r10+8*4), %r10;
    mov (g_backup_r9+8*4),  %r9;
    mov (g_backup_r8+8*4),  %r8;
    mov (g_backup_rbp+8*4), %rbp;
    mov (g_backup_rdi+8*4), %rdi;
    mov (g_backup_rsi+8*4), %rsi;
    mov (g_backup_rdx+8*4), %rdx;
    mov (g_backup_rcx+8*4), %rcx;
    mov (g_backup_rbx+8*4), %rbx;
    mov (g_backup_cr3+8*4), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*4), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*4), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core4_isr_addr(,1);  # go to origin isr 



.align 0x100;
hook_core5_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core5_label;

hook_core5_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*5); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*5);
    mov %rcx, (g_backup_rcx+8*5);
    mov %rdx, (g_backup_rdx+8*5);
    mov %rsi, (g_backup_rsi+8*5);
    mov %rdi, (g_backup_rdi+8*5);
    mov %rbp, (g_backup_rbp+8*5);
    mov %r8,  (g_backup_r8+8*5);
    mov %r9,  (g_backup_r9+8*5);
    mov %r10, (g_backup_r10+8*5);
    mov %r11, (g_backup_r11+8*5);
    mov %r12, (g_backup_r12+8*5);
    mov %r13, (g_backup_r13+8*5);
    mov %r14, (g_backup_r14+8*5);
    mov %r15, (g_backup_r15+8*5);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*5); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*5);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*5);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*5);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*5);

    mov %ds, (g_backup_ds+8*5);
    mov %es, (g_backup_es+8*5);
    mov %fs, (g_backup_fs+8*5);
    mov %gs, (g_backup_gs+8*5);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*5);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*5);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*5);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*5);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*5);

    sgdt (g_backup_gdtr+8*5);
    sldt (g_backup_ldtr+8*5);
    sidt (g_backup_idtr+8*5);
    str  (g_backup_tr+8*5);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*5), %r15;
    mov (g_backup_r14+8*5), %r14;
    mov (g_backup_r13+8*5), %r13;
    mov (g_backup_r12+8*5), %r12;
    mov (g_backup_r11+8*5), %r11;
    mov (g_backup_r10+8*5), %r10;
    mov (g_backup_r9+8*5),  %r9;
    mov (g_backup_r8+8*5),  %r8;
    mov (g_backup_rbp+8*5), %rbp;
    mov (g_backup_rdi+8*5), %rdi;
    mov (g_backup_rsi+8*5), %rsi;
    mov (g_backup_rdx+8*5), %rdx;
    mov (g_backup_rcx+8*5), %rcx;
    mov (g_backup_rbx+8*5), %rbx;
    mov (g_backup_cr3+8*5), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*5), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*5), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core5_isr_addr(,1);  # go to origin isr 


.align 0x100;
hook_core6_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core6_label;

hook_core6_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*6); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*6);
    mov %rcx, (g_backup_rcx+8*6);
    mov %rdx, (g_backup_rdx+8*6);
    mov %rsi, (g_backup_rsi+8*6);
    mov %rdi, (g_backup_rdi+8*6);
    mov %rbp, (g_backup_rbp+8*6);
    mov %r8,  (g_backup_r8+8*6);
    mov %r9,  (g_backup_r9+8*6);
    mov %r10, (g_backup_r10+8*6);
    mov %r11, (g_backup_r11+8*6);
    mov %r12, (g_backup_r12+8*6);
    mov %r13, (g_backup_r13+8*6);
    mov %r14, (g_backup_r14+8*6);
    mov %r15, (g_backup_r15+8*6);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*6); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*6);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*6);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*6);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*6);

    mov %ds, (g_backup_ds+8*6);
    mov %es, (g_backup_es+8*6);
    mov %fs, (g_backup_fs+8*6);
    mov %gs, (g_backup_gs+8*6);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*6);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*6);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*6);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*6);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*6);

    sgdt (g_backup_gdtr+8*6);
    sldt (g_backup_ldtr+8*6);
    sidt (g_backup_idtr+8*6);
    str  (g_backup_tr+8*6);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*6), %r15;
    mov (g_backup_r14+8*6), %r14;
    mov (g_backup_r13+8*6), %r13;
    mov (g_backup_r12+8*6), %r12;
    mov (g_backup_r11+8*6), %r11;
    mov (g_backup_r10+8*6), %r10;
    mov (g_backup_r9+8*6),  %r9;
    mov (g_backup_r8+8*6),  %r8;
    mov (g_backup_rbp+8*6), %rbp;
    mov (g_backup_rdi+8*6), %rdi;
    mov (g_backup_rsi+8*6), %rsi;
    mov (g_backup_rdx+8*6), %rdx;
    mov (g_backup_rcx+8*6), %rcx;
    mov (g_backup_rbx+8*6), %rbx;
    mov (g_backup_cr3+8*6), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*6), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*6), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core6_isr_addr(,1);  # go to origin isr 


.align 0x100;
hook_core7_isr_asm_fun:
    pushfq; #save rflag , rsp - 8

    jmp hook_core7_label;

hook_core7_label:

    # change iopl
    pushfq;
    andq $0xffffffffffffcfff, (%rsp);
    popfq;

    # save registers
    mov %rax, (g_backup_rax+8*7); // () gas address not immediate
    mov %rbx, (g_backup_rbx+8*7);
    mov %rcx, (g_backup_rcx+8*7);
    mov %rdx, (g_backup_rdx+8*7);
    mov %rsi, (g_backup_rsi+8*7);
    mov %rdi, (g_backup_rdi+8*7);
    mov %rbp, (g_backup_rbp+8*7);
    mov %r8,  (g_backup_r8+8*7);
    mov %r9,  (g_backup_r9+8*7);
    mov %r10, (g_backup_r10+8*7);
    mov %r11, (g_backup_r11+8*7);
    mov %r12, (g_backup_r12+8*7);
    mov %r13, (g_backup_r13+8*7);
    mov %r14, (g_backup_r14+8*7);
    mov %r15, (g_backup_r15+8*7);

    # save interrupt stack context, after transfer to isr handler
    ## ss +40
    ## rsp +32
    ## rflags +24
    ## cs +16
    ## rip +8
    ## error code +0   rsp   some interrupt and exception have no error node, such this
    movq 8(%rsp), %rax;
    movq %rax, (g_backup_rip+8*7); # pushfq lead rsp-8
    movq 16(%rsp), %rax;
    movq %rax, (g_backup_cs+8*7);
    movq 24(%rsp), %rax;
    movq %rax, (g_backup_rflags+8*7);
    movq 32(%rsp), %rax;
    movq %rax, (g_backup_rsp+8*7);
    movq 40(%rsp), %rax;
    movq %rax, (g_backup_ss+8*7);

    mov %ds, (g_backup_ds+8*7);
    mov %es, (g_backup_es+8*7);
    mov %fs, (g_backup_fs+8*7);
    mov %gs, (g_backup_gs+8*7);

    mov %cr0, %rax;
    mov %rax, (g_backup_cr0+8*7);
    and $~0x08, %rax;  # cr0 bit 3 ts task switched bit clear
    mov %rax, %cr0;
    
    mov %cr2, %rax;
    mov %rax, (g_backup_cr2+8*7);
    mov %cr3, %rax;
    mov %rax, (g_backup_cr3+8*7);
    mov %cr4, %rax;
    mov %rax, (g_backup_cr4+8*7);
    mov %cr8, %rax;
    mov %rax, (g_backup_cr8+8*7);

    sgdt (g_backup_gdtr+8*7);
    sldt (g_backup_ldtr+8*7);
    sidt (g_backup_idtr+8*7);
    str  (g_backup_tr+8*7);

    #call hook isr

    callq user_isr_c_fun;

    #restore 
    mov (g_backup_r15+8*7), %r15;
    mov (g_backup_r14+8*7), %r14;
    mov (g_backup_r13+8*7), %r13;
    mov (g_backup_r12+8*7), %r12;
    mov (g_backup_r11+8*7), %r11;
    mov (g_backup_r10+8*7), %r10;
    mov (g_backup_r9+8*7),  %r9;
    mov (g_backup_r8+8*7),  %r8;
    mov (g_backup_rbp+8*7), %rbp;
    mov (g_backup_rdi+8*7), %rdi;
    mov (g_backup_rsi+8*7), %rsi;
    mov (g_backup_rdx+8*7), %rdx;
    mov (g_backup_rcx+8*7), %rcx;
    mov (g_backup_rbx+8*7), %rbx;
    mov (g_backup_cr3+8*7), %rax;
    mov %rax, %cr3;
    mov (g_backup_cr0+8*7), %rax;
    mov %rax, %cr0;
    mov (g_backup_rax+8*7), %rax;

    #restore rflags
    popfq;
    jmpq *g_origin_core7_isr_addr(,1);  # go to origin isr 