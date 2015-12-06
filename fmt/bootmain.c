9150 // Boot loader.
9151 //
9152 // Part of the boot block, along with bootasm.S, which calls bootmain().
9153 // bootasm.S has put the processor into protected 32-bit mode.
9154 // bootmain() loads an ELF kernel image from the disk starting at
9155 // sector 1 and then jumps to the kernel entry routine.
9156 
9157 #include "types.h"
9158 #include "elf.h"
9159 #include "x86.h"
9160 #include "memlayout.h"
9161 
9162 #define SECTSIZE  512
9163 
9164 void readseg(uchar*, uint, uint);
9165 
9166 void
9167 bootmain(void)
9168 {
9169   struct elfhdr *elf;
9170   struct proghdr *ph, *eph;
9171   void (*entry)(void);
9172   uchar* pa;
9173 
9174   elf = (struct elfhdr*)0x10000;  // scratch space
9175 
9176   // Read 1st page off disk
9177   readseg((uchar*)elf, 4096, 0);
9178 
9179   // Is this an ELF executable?
9180   if(elf->magic != ELF_MAGIC)
9181     return;  // let bootasm.S handle error
9182 
9183   // Load each program segment (ignores ph flags).
9184   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9185   eph = ph + elf->phnum;
9186   for(; ph < eph; ph++){
9187     pa = (uchar*)ph->paddr;
9188     readseg(pa, ph->filesz, ph->off);
9189     if(ph->memsz > ph->filesz)
9190       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9191   }
9192 
9193   // Call the entry point from the ELF header.
9194   // Does not return!
9195   entry = (void(*)(void))(elf->entry);
9196   entry();
9197 }
9198 
9199 
9200 void
9201 waitdisk(void)
9202 {
9203   // Wait for disk ready.
9204   while((inb(0x1F7) & 0xC0) != 0x40)
9205     ;
9206 }
9207 
9208 // Read a single sector at offset into dst.
9209 void
9210 readsect(void *dst, uint offset)
9211 {
9212   // Issue command.
9213   waitdisk();
9214   outb(0x1F2, 1);   // count = 1
9215   outb(0x1F3, offset);
9216   outb(0x1F4, offset >> 8);
9217   outb(0x1F5, offset >> 16);
9218   outb(0x1F6, (offset >> 24) | 0xE0);
9219   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9220 
9221   // Read data.
9222   waitdisk();
9223   insl(0x1F0, dst, SECTSIZE/4);
9224 }
9225 
9226 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9227 // Might copy more than asked.
9228 void
9229 readseg(uchar* pa, uint count, uint offset)
9230 {
9231   uchar* epa;
9232 
9233   epa = pa + count;
9234 
9235   // Round down to sector boundary.
9236   pa -= offset % SECTSIZE;
9237 
9238   // Translate from bytes to sectors; kernel starts at sector 1.
9239   offset = (offset / SECTSIZE) + 1;
9240 
9241   // If this is too slow, we could read lots of sectors at a time.
9242   // We'd write more to memory than asked, but it doesn't matter --
9243   // we load in increasing order.
9244   for(; pa < epa; pa += SECTSIZE, offset++)
9245     readsect(pa, offset);
9246 }
9247 
9248 
9249 
