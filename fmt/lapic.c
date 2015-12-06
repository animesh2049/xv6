7150 // The local APIC manages internal (non-I/O) interrupts.
7151 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7152 
7153 #include "types.h"
7154 #include "defs.h"
7155 #include "date.h"
7156 #include "memlayout.h"
7157 #include "traps.h"
7158 #include "mmu.h"
7159 #include "x86.h"
7160 
7161 // Local APIC registers, divided by 4 for use as uint[] indices.
7162 #define ID      (0x0020/4)   // ID
7163 #define VER     (0x0030/4)   // Version
7164 #define TPR     (0x0080/4)   // Task Priority
7165 #define EOI     (0x00B0/4)   // EOI
7166 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7167   #define ENABLE     0x00000100   // Unit Enable
7168 #define ESR     (0x0280/4)   // Error Status
7169 #define ICRLO   (0x0300/4)   // Interrupt Command
7170   #define INIT       0x00000500   // INIT/RESET
7171   #define STARTUP    0x00000600   // Startup IPI
7172   #define DELIVS     0x00001000   // Delivery status
7173   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7174   #define DEASSERT   0x00000000
7175   #define LEVEL      0x00008000   // Level triggered
7176   #define BCAST      0x00080000   // Send to all APICs, including self.
7177   #define BUSY       0x00001000
7178   #define FIXED      0x00000000
7179 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7180 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7181   #define X1         0x0000000B   // divide counts by 1
7182   #define PERIODIC   0x00020000   // Periodic
7183 #define PCINT   (0x0340/4)   // Performance Counter LVT
7184 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7185 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7186 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7187   #define MASKED     0x00010000   // Interrupt masked
7188 #define TICR    (0x0380/4)   // Timer Initial Count
7189 #define TCCR    (0x0390/4)   // Timer Current Count
7190 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7191 
7192 volatile uint *lapic;  // Initialized in mp.c
7193 
7194 static void
7195 lapicw(int index, int value)
7196 {
7197   lapic[index] = value;
7198   lapic[ID];  // wait for write to finish, by reading
7199 }
7200 
7201 
7202 
7203 
7204 
7205 
7206 
7207 
7208 
7209 
7210 
7211 
7212 
7213 
7214 
7215 
7216 
7217 
7218 
7219 
7220 
7221 
7222 
7223 
7224 
7225 
7226 
7227 
7228 
7229 
7230 
7231 
7232 
7233 
7234 
7235 
7236 
7237 
7238 
7239 
7240 
7241 
7242 
7243 
7244 
7245 
7246 
7247 
7248 
7249 
7250 void
7251 lapicinit(void)
7252 {
7253   if(!lapic)
7254     return;
7255 
7256   // Enable local APIC; set spurious interrupt vector.
7257   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7258 
7259   // The timer repeatedly counts down at bus frequency
7260   // from lapic[TICR] and then issues an interrupt.
7261   // If xv6 cared more about precise timekeeping,
7262   // TICR would be calibrated using an external time source.
7263   lapicw(TDCR, X1);
7264   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7265   lapicw(TICR, 10000000);
7266 
7267   // Disable logical interrupt lines.
7268   lapicw(LINT0, MASKED);
7269   lapicw(LINT1, MASKED);
7270 
7271   // Disable performance counter overflow interrupts
7272   // on machines that provide that interrupt entry.
7273   if(((lapic[VER]>>16) & 0xFF) >= 4)
7274     lapicw(PCINT, MASKED);
7275 
7276   // Map error interrupt to IRQ_ERROR.
7277   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7278 
7279   // Clear error status register (requires back-to-back writes).
7280   lapicw(ESR, 0);
7281   lapicw(ESR, 0);
7282 
7283   // Ack any outstanding interrupts.
7284   lapicw(EOI, 0);
7285 
7286   // Send an Init Level De-Assert to synchronise arbitration ID's.
7287   lapicw(ICRHI, 0);
7288   lapicw(ICRLO, BCAST | INIT | LEVEL);
7289   while(lapic[ICRLO] & DELIVS)
7290     ;
7291 
7292   // Enable interrupts on the APIC (but not on the processor).
7293   lapicw(TPR, 0);
7294 }
7295 
7296 
7297 
7298 
7299 
7300 int
7301 cpunum(void)
7302 {
7303   // Cannot call cpu when interrupts are enabled:
7304   // result not guaranteed to last long enough to be used!
7305   // Would prefer to panic but even printing is chancy here:
7306   // almost everything, including cprintf and panic, calls cpu,
7307   // often indirectly through acquire and release.
7308   if(readeflags()&FL_IF){
7309     static int n;
7310     if(n++ == 0)
7311       cprintf("cpu called from %x with interrupts enabled\n",
7312         __builtin_return_address(0));
7313   }
7314 
7315   if(lapic)
7316     return lapic[ID]>>24;
7317   return 0;
7318 }
7319 
7320 // Acknowledge interrupt.
7321 void
7322 lapiceoi(void)
7323 {
7324   if(lapic)
7325     lapicw(EOI, 0);
7326 }
7327 
7328 // Spin for a given number of microseconds.
7329 // On real hardware would want to tune this dynamically.
7330 void
7331 microdelay(int us)
7332 {
7333 }
7334 
7335 #define CMOS_PORT    0x70
7336 #define CMOS_RETURN  0x71
7337 
7338 // Start additional processor running entry code at addr.
7339 // See Appendix B of MultiProcessor Specification.
7340 void
7341 lapicstartap(uchar apicid, uint addr)
7342 {
7343   int i;
7344   ushort *wrv;
7345 
7346   // "The BSP must initialize CMOS shutdown code to 0AH
7347   // and the warm reset vector (DWORD based at 40:67) to point at
7348   // the AP startup code prior to the [universal startup algorithm]."
7349   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7350   outb(CMOS_PORT+1, 0x0A);
7351   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7352   wrv[0] = 0;
7353   wrv[1] = addr >> 4;
7354 
7355   // "Universal startup algorithm."
7356   // Send INIT (level-triggered) interrupt to reset other CPU.
7357   lapicw(ICRHI, apicid<<24);
7358   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7359   microdelay(200);
7360   lapicw(ICRLO, INIT | LEVEL);
7361   microdelay(100);    // should be 10ms, but too slow in Bochs!
7362 
7363   // Send startup IPI (twice!) to enter code.
7364   // Regular hardware is supposed to only accept a STARTUP
7365   // when it is in the halted state due to an INIT.  So the second
7366   // should be ignored, but it is part of the official Intel algorithm.
7367   // Bochs complains about the second one.  Too bad for Bochs.
7368   for(i = 0; i < 2; i++){
7369     lapicw(ICRHI, apicid<<24);
7370     lapicw(ICRLO, STARTUP | (addr>>12));
7371     microdelay(200);
7372   }
7373 }
7374 
7375 #define CMOS_STATA   0x0a
7376 #define CMOS_STATB   0x0b
7377 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7378 
7379 #define SECS    0x00
7380 #define MINS    0x02
7381 #define HOURS   0x04
7382 #define DAY     0x07
7383 #define MONTH   0x08
7384 #define YEAR    0x09
7385 
7386 static uint cmos_read(uint reg)
7387 {
7388   outb(CMOS_PORT,  reg);
7389   microdelay(200);
7390 
7391   return inb(CMOS_RETURN);
7392 }
7393 
7394 
7395 
7396 
7397 
7398 
7399 
7400 static void fill_rtcdate(struct rtcdate *r)
7401 {
7402   r->second = cmos_read(SECS);
7403   r->minute = cmos_read(MINS);
7404   r->hour   = cmos_read(HOURS);
7405   r->day    = cmos_read(DAY);
7406   r->month  = cmos_read(MONTH);
7407   r->year   = cmos_read(YEAR);
7408 }
7409 
7410 // qemu seems to use 24-hour GWT and the values are BCD encoded
7411 void cmostime(struct rtcdate *r)
7412 {
7413   struct rtcdate t1, t2;
7414   int sb, bcd;
7415 
7416   sb = cmos_read(CMOS_STATB);
7417 
7418   bcd = (sb & (1 << 2)) == 0;
7419 
7420   // make sure CMOS doesn't modify time while we read it
7421   for (;;) {
7422     fill_rtcdate(&t1);
7423     if (cmos_read(CMOS_STATA) & CMOS_UIP)
7424         continue;
7425     fill_rtcdate(&t2);
7426     if (memcmp(&t1, &t2, sizeof(t1)) == 0)
7427       break;
7428   }
7429 
7430   // convert
7431   if (bcd) {
7432 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7433     CONV(second);
7434     CONV(minute);
7435     CONV(hour  );
7436     CONV(day   );
7437     CONV(month );
7438     CONV(year  );
7439 #undef     CONV
7440   }
7441 
7442   *r = t1;
7443   r->year += 2000;
7444 }
7445 
7446 
7447 
7448 
7449 
