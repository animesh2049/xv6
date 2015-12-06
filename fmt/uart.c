8250 // Intel 8250 serial port (UART).
8251 
8252 #include "types.h"
8253 #include "defs.h"
8254 #include "param.h"
8255 #include "traps.h"
8256 #include "spinlock.h"
8257 #include "fs.h"
8258 #include "file.h"
8259 #include "mmu.h"
8260 #include "proc.h"
8261 #include "x86.h"
8262 
8263 #define COM1    0x3f8
8264 
8265 static int uart;    // is there a uart?
8266 
8267 void
8268 uartinit(void)
8269 {
8270   char *p;
8271 
8272   // Turn off the FIFO
8273   outb(COM1+2, 0);
8274 
8275   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8276   outb(COM1+3, 0x80);    // Unlock divisor
8277   outb(COM1+0, 115200/9600);
8278   outb(COM1+1, 0);
8279   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8280   outb(COM1+4, 0);
8281   outb(COM1+1, 0x01);    // Enable receive interrupts.
8282 
8283   // If status is 0xFF, no serial port.
8284   if(inb(COM1+5) == 0xFF)
8285     return;
8286   uart = 1;
8287 
8288   // Acknowledge pre-existing interrupt conditions;
8289   // enable interrupts.
8290   inb(COM1+2);
8291   inb(COM1+0);
8292   picenable(IRQ_COM1);
8293   ioapicenable(IRQ_COM1, 0);
8294 
8295   // Announce that we're here.
8296   for(p="xv6...\n"; *p; p++)
8297     uartputc(*p);
8298 }
8299 
8300 void
8301 uartputc(int c)
8302 {
8303   int i;
8304 
8305   if(!uart)
8306     return;
8307   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8308     microdelay(10);
8309   outb(COM1+0, c);
8310 }
8311 
8312 static int
8313 uartgetc(void)
8314 {
8315   if(!uart)
8316     return -1;
8317   if(!(inb(COM1+5) & 0x01))
8318     return -1;
8319   return inb(COM1+0);
8320 }
8321 
8322 void
8323 uartintr(void)
8324 {
8325   consoleintr(uartgetc);
8326 }
8327 
8328 
8329 
8330 
8331 
8332 
8333 
8334 
8335 
8336 
8337 
8338 
8339 
8340 
8341 
8342 
8343 
8344 
8345 
8346 
8347 
8348 
8349 
