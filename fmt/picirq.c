7550 // Intel 8259A programmable interrupt controllers.
7551 
7552 #include "types.h"
7553 #include "x86.h"
7554 #include "traps.h"
7555 
7556 // I/O Addresses of the two programmable interrupt controllers
7557 #define IO_PIC1         0x20    // Master (IRQs 0-7)
7558 #define IO_PIC2         0xA0    // Slave (IRQs 8-15)
7559 
7560 #define IRQ_SLAVE       2       // IRQ at which slave connects to master
7561 
7562 // Current IRQ mask.
7563 // Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
7564 static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);
7565 
7566 static void
7567 picsetmask(ushort mask)
7568 {
7569   irqmask = mask;
7570   outb(IO_PIC1+1, mask);
7571   outb(IO_PIC2+1, mask >> 8);
7572 }
7573 
7574 void
7575 picenable(int irq)
7576 {
7577   picsetmask(irqmask & ~(1<<irq));
7578 }
7579 
7580 // Initialize the 8259A interrupt controllers.
7581 void
7582 picinit(void)
7583 {
7584   // mask all interrupts
7585   outb(IO_PIC1+1, 0xFF);
7586   outb(IO_PIC2+1, 0xFF);
7587 
7588   // Set up master (8259A-1)
7589 
7590   // ICW1:  0001g0hi
7591   //    g:  0 = edge triggering, 1 = level triggering
7592   //    h:  0 = cascaded PICs, 1 = master only
7593   //    i:  0 = no ICW4, 1 = ICW4 required
7594   outb(IO_PIC1, 0x11);
7595 
7596   // ICW2:  Vector offset
7597   outb(IO_PIC1+1, T_IRQ0);
7598 
7599 
7600   // ICW3:  (master PIC) bit mask of IR lines connected to slaves
7601   //        (slave PIC) 3-bit # of slave's connection to master
7602   outb(IO_PIC1+1, 1<<IRQ_SLAVE);
7603 
7604   // ICW4:  000nbmap
7605   //    n:  1 = special fully nested mode
7606   //    b:  1 = buffered mode
7607   //    m:  0 = slave PIC, 1 = master PIC
7608   //      (ignored when b is 0, as the master/slave role
7609   //      can be hardwired).
7610   //    a:  1 = Automatic EOI mode
7611   //    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
7612   outb(IO_PIC1+1, 0x3);
7613 
7614   // Set up slave (8259A-2)
7615   outb(IO_PIC2, 0x11);                  // ICW1
7616   outb(IO_PIC2+1, T_IRQ0 + 8);      // ICW2
7617   outb(IO_PIC2+1, IRQ_SLAVE);           // ICW3
7618   // NB Automatic EOI mode doesn't tend to work on the slave.
7619   // Linux source code says it's "to be investigated".
7620   outb(IO_PIC2+1, 0x3);                 // ICW4
7621 
7622   // OCW3:  0ef01prs
7623   //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
7624   //    p:  0 = no polling, 1 = polling mode
7625   //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
7626   outb(IO_PIC1, 0x68);             // clear specific mask
7627   outb(IO_PIC1, 0x0a);             // read IRR by default
7628 
7629   outb(IO_PIC2, 0x68);             // OCW3
7630   outb(IO_PIC2, 0x0a);             // OCW3
7631 
7632   if(irqmask != 0xFFFF)
7633     picsetmask(irqmask);
7634 }
7635 
7636 
7637 
7638 
7639 
7640 
7641 
7642 
7643 
7644 
7645 
7646 
7647 
7648 
7649 
