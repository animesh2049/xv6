8200 // Intel 8253/8254/82C54 Programmable Interval Timer (PIT).
8201 // Only used on uniprocessors;
8202 // SMP machines use the local APIC timer.
8203 
8204 #include "types.h"
8205 #include "defs.h"
8206 #include "traps.h"
8207 #include "x86.h"
8208 
8209 #define IO_TIMER1       0x040           // 8253 Timer #1
8210 
8211 // Frequency of all three count-down timers;
8212 // (TIMER_FREQ/freq) is the appropriate count
8213 // to generate a frequency of freq Hz.
8214 
8215 #define TIMER_FREQ      1193182
8216 #define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
8217 
8218 #define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
8219 #define TIMER_SEL0      0x00    // select counter 0
8220 #define TIMER_RATEGEN   0x04    // mode 2, rate generator
8221 #define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first
8222 
8223 void
8224 timerinit(void)
8225 {
8226   // Interrupt 100 times/sec.
8227   outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
8228   outb(IO_TIMER1, TIMER_DIV(100) % 256);
8229   outb(IO_TIMER1, TIMER_DIV(100) / 256);
8230   picenable(IRQ_TIMER);
8231 }
8232 
8233 
8234 
8235 
8236 
8237 
8238 
8239 
8240 
8241 
8242 
8243 
8244 
8245 
8246 
8247 
8248 
8249 
