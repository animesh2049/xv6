7800 #include "types.h"
7801 #include "x86.h"
7802 #include "defs.h"
7803 #include "kbd.h"
7804 
7805 int
7806 kbdgetc(void)
7807 {
7808   static uint shift;
7809   static uchar *charcode[4] = {
7810     normalmap, shiftmap, ctlmap, ctlmap
7811   };
7812   uint st, data, c;
7813 
7814   st = inb(KBSTATP);
7815   if((st & KBS_DIB) == 0)
7816     return -1;
7817   data = inb(KBDATAP);
7818 
7819   if(data == 0xE0){
7820     shift |= E0ESC;
7821     return 0;
7822   } else if(data & 0x80){
7823     // Key released
7824     data = (shift & E0ESC ? data : data & 0x7F);
7825     shift &= ~(shiftcode[data] | E0ESC);
7826     return 0;
7827   } else if(shift & E0ESC){
7828     // Last character was an E0 escape; or with 0x80
7829     data |= 0x80;
7830     shift &= ~E0ESC;
7831   }
7832 
7833   shift |= shiftcode[data];
7834   shift ^= togglecode[data];
7835   c = charcode[shift & (CTL | SHIFT)][data];
7836   if(shift & CAPSLOCK){
7837     if('a' <= c && c <= 'z')
7838       c += 'A' - 'a';
7839     else if('A' <= c && c <= 'Z')
7840       c += 'a' - 'A';
7841   }
7842   return c;
7843 }
7844 
7845 void
7846 kbdintr(void)
7847 {
7848   consoleintr(kbdgetc);
7849 }
