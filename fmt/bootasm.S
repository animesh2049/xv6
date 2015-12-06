9050 #include "asm.h"
9051 #include "memlayout.h"
9052 #include "mmu.h"
9053 
9054 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9055 # The BIOS loads this code from the first sector of the hard disk into
9056 # memory at physical address 0x7c00 and starts executing in real mode
9057 # with %cs=0 %ip=7c00.
9058 
9059 .code16                       # Assemble for 16-bit mode
9060 .globl start
9061 start:
9062   cli                         # BIOS enabled interrupts; disable
9063 
9064   # Zero data segment registers DS, ES, and SS.
9065   xorw    %ax,%ax             # Set %ax to zero
9066   movw    %ax,%ds             # -> Data Segment
9067   movw    %ax,%es             # -> Extra Segment
9068   movw    %ax,%ss             # -> Stack Segment
9069 
9070   # Physical address line A20 is tied to zero so that the first PCs
9071   # with 2 MB would run software that assumed 1 MB.  Undo that.
9072 seta20.1:
9073   inb     $0x64,%al               # Wait for not busy
9074   testb   $0x2,%al
9075   jnz     seta20.1
9076 
9077   movb    $0xd1,%al               # 0xd1 -> port 0x64
9078   outb    %al,$0x64
9079 
9080 seta20.2:
9081   inb     $0x64,%al               # Wait for not busy
9082   testb   $0x2,%al
9083   jnz     seta20.2
9084 
9085   movb    $0xdf,%al               # 0xdf -> port 0x60
9086   outb    %al,$0x60
9087 
9088   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9089   # virtual addresses map directly to physical addresses so that the
9090   # effective memory map doesn't change during the transition.
9091   lgdt    gdtdesc
9092   movl    %cr0, %eax
9093   orl     $CR0_PE, %eax
9094   movl    %eax, %cr0
9095 
9096 
9097 
9098 
9099 
9100   # Complete transition to 32-bit protected mode by using long jmp
9101   # to reload %cs and %eip.  The segment descriptors are set up with no
9102   # translation, so that the mapping is still the identity mapping.
9103   ljmp    $(SEG_KCODE<<3), $start32
9104 
9105 .code32  # Tell assembler to generate 32-bit code now.
9106 start32:
9107   # Set up the protected-mode data segment registers
9108   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9109   movw    %ax, %ds                # -> DS: Data Segment
9110   movw    %ax, %es                # -> ES: Extra Segment
9111   movw    %ax, %ss                # -> SS: Stack Segment
9112   movw    $0, %ax                 # Zero segments not ready for use
9113   movw    %ax, %fs                # -> FS
9114   movw    %ax, %gs                # -> GS
9115 
9116   # Set up the stack pointer and call into C.
9117   movl    $start, %esp
9118   call    bootmain
9119 
9120   # If bootmain returns (it shouldn't), trigger a Bochs
9121   # breakpoint if running under Bochs, then loop.
9122   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9123   movw    %ax, %dx
9124   outw    %ax, %dx
9125   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9126   outw    %ax, %dx
9127 spin:
9128   jmp     spin
9129 
9130 # Bootstrap GDT
9131 .p2align 2                                # force 4 byte alignment
9132 gdt:
9133   SEG_NULLASM                             # null seg
9134   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9135   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9136 
9137 gdtdesc:
9138   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9139   .long   gdt                             # address gdt
9140 
9141 
9142 
9143 
9144 
9145 
9146 
9147 
9148 
9149 
