7450 // The I/O APIC manages hardware interrupts for an SMP system.
7451 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7452 // See also picirq.c.
7453 
7454 #include "types.h"
7455 #include "defs.h"
7456 #include "traps.h"
7457 
7458 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7459 
7460 #define REG_ID     0x00  // Register index: ID
7461 #define REG_VER    0x01  // Register index: version
7462 #define REG_TABLE  0x10  // Redirection table base
7463 
7464 // The redirection table starts at REG_TABLE and uses
7465 // two registers to configure each interrupt.
7466 // The first (low) register in a pair contains configuration bits.
7467 // The second (high) register contains a bitmask telling which
7468 // CPUs can serve that interrupt.
7469 #define INT_DISABLED   0x00010000  // Interrupt disabled
7470 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7471 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7472 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7473 
7474 volatile struct ioapic *ioapic;
7475 
7476 // IO APIC MMIO structure: write reg, then read or write data.
7477 struct ioapic {
7478   uint reg;
7479   uint pad[3];
7480   uint data;
7481 };
7482 
7483 static uint
7484 ioapicread(int reg)
7485 {
7486   ioapic->reg = reg;
7487   return ioapic->data;
7488 }
7489 
7490 static void
7491 ioapicwrite(int reg, uint data)
7492 {
7493   ioapic->reg = reg;
7494   ioapic->data = data;
7495 }
7496 
7497 
7498 
7499 
7500 void
7501 ioapicinit(void)
7502 {
7503   int i, id, maxintr;
7504 
7505   if(!ismp)
7506     return;
7507 
7508   ioapic = (volatile struct ioapic*)IOAPIC;
7509   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7510   id = ioapicread(REG_ID) >> 24;
7511   if(id != ioapicid)
7512     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7513 
7514   // Mark all interrupts edge-triggered, active high, disabled,
7515   // and not routed to any CPUs.
7516   for(i = 0; i <= maxintr; i++){
7517     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7518     ioapicwrite(REG_TABLE+2*i+1, 0);
7519   }
7520 }
7521 
7522 void
7523 ioapicenable(int irq, int cpunum)
7524 {
7525   if(!ismp)
7526     return;
7527 
7528   // Mark interrupt edge-triggered, active high,
7529   // enabled, and routed to the given cpunum,
7530   // which happens to be that cpu's APIC ID.
7531   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7532   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7533 }
7534 
7535 
7536 
7537 
7538 
7539 
7540 
7541 
7542 
7543 
7544 
7545 
7546 
7547 
7548 
7549 
