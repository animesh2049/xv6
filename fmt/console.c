7850 // Console input and output.
7851 // Input is from the keyboard or serial port.
7852 // Output is written to the screen and serial port.
7853 
7854 #include "types.h"
7855 #include "defs.h"
7856 #include "param.h"
7857 #include "traps.h"
7858 #include "spinlock.h"
7859 #include "fs.h"
7860 #include "file.h"
7861 #include "memlayout.h"
7862 #include "mmu.h"
7863 #include "proc.h"
7864 #include "x86.h"
7865 
7866 static void consputc(int);
7867 
7868 static int panicked = 0;
7869 
7870 static struct {
7871   struct spinlock lock;
7872   int locking;
7873 } cons;
7874 
7875 static void
7876 printint(int xx, int base, int sign)
7877 {
7878   static char digits[] = "0123456789abcdef";
7879   char buf[16];
7880   int i;
7881   uint x;
7882 
7883   if(sign && (sign = xx < 0))
7884     x = -xx;
7885   else
7886     x = xx;
7887 
7888   i = 0;
7889   do{
7890     buf[i++] = digits[x % base];
7891   }while((x /= base) != 0);
7892 
7893   if(sign)
7894     buf[i++] = '-';
7895 
7896   while(--i >= 0)
7897     consputc(buf[i]);
7898 }
7899 
7900 // Print to the console. only understands %d, %x, %p, %s.
7901 void
7902 cprintf(char *fmt, ...)
7903 {
7904   int i, c, locking;
7905   uint *argp;
7906   char *s;
7907 
7908   locking = cons.locking;
7909   if(locking)
7910     acquire(&cons.lock);
7911 
7912   if (fmt == 0)
7913     panic("null fmt");
7914 
7915   argp = (uint*)(void*)(&fmt + 1);
7916   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
7917     if(c != '%'){
7918       consputc(c);
7919       continue;
7920     }
7921     c = fmt[++i] & 0xff;
7922     if(c == 0)
7923       break;
7924     switch(c){
7925     case 'd':
7926       printint(*argp++, 10, 1);
7927       break;
7928     case 'x':
7929     case 'p':
7930       printint(*argp++, 16, 0);
7931       break;
7932     case 's':
7933       if((s = (char*)*argp++) == 0)
7934         s = "(null)";
7935       for(; *s; s++)
7936         consputc(*s);
7937       break;
7938     case '%':
7939       consputc('%');
7940       break;
7941     default:
7942       // Print unknown % sequence to draw attention.
7943       consputc('%');
7944       consputc(c);
7945       break;
7946     }
7947   }
7948 
7949 
7950   if(locking)
7951     release(&cons.lock);
7952 }
7953 
7954 void
7955 panic(char *s)
7956 {
7957   int i;
7958   uint pcs[10];
7959 
7960   cli();
7961   cons.locking = 0;
7962   cprintf("cpu%d: panic: ", cpu->id);
7963   cprintf(s);
7964   cprintf("\n");
7965   getcallerpcs(&s, pcs);
7966   for(i=0; i<10; i++)
7967     cprintf(" %p", pcs[i]);
7968   panicked = 1; // freeze other CPU
7969   for(;;)
7970     ;
7971 }
7972 
7973 
7974 
7975 
7976 
7977 
7978 
7979 
7980 
7981 
7982 
7983 
7984 
7985 
7986 
7987 
7988 
7989 
7990 
7991 
7992 
7993 
7994 
7995 
7996 
7997 
7998 
7999 
8000 #define BACKSPACE 0x100
8001 #define CRTPORT 0x3d4
8002 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8003 
8004 static void
8005 cgaputc(int c)
8006 {
8007   int pos;
8008 
8009   // Cursor position: col + 80*row.
8010   outb(CRTPORT, 14);
8011   pos = inb(CRTPORT+1) << 8;
8012   outb(CRTPORT, 15);
8013   pos |= inb(CRTPORT+1);
8014 
8015   if(c == '\n')
8016     pos += 80 - pos%80;
8017   else if(c == BACKSPACE){
8018     if(pos > 0) --pos;
8019   } else
8020     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8021 
8022   if((pos/80) >= 24){  // Scroll up.
8023     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8024     pos -= 80;
8025     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8026   }
8027 
8028   outb(CRTPORT, 14);
8029   outb(CRTPORT+1, pos>>8);
8030   outb(CRTPORT, 15);
8031   outb(CRTPORT+1, pos);
8032   crt[pos] = ' ' | 0x0700;
8033 }
8034 
8035 void
8036 consputc(int c)
8037 {
8038   if(panicked){
8039     cli();
8040     for(;;)
8041       ;
8042   }
8043 
8044   if(c == BACKSPACE){
8045     uartputc('\b'); uartputc(' '); uartputc('\b');
8046   } else
8047     uartputc(c);
8048   cgaputc(c);
8049 }
8050 #define INPUT_BUF 128
8051 struct {
8052   struct spinlock lock;
8053   char buf[INPUT_BUF];
8054   uint r;  // Read index
8055   uint w;  // Write index
8056   uint e;  // Edit index
8057 } input;
8058 
8059 #define C(x)  ((x)-'@')  // Control-x
8060 
8061 void
8062 consoleintr(int (*getc)(void))
8063 {
8064   int c;
8065 
8066   acquire(&input.lock);
8067   while((c = getc()) >= 0){
8068     switch(c){
8069     case C('P'):  // Process listing.
8070       procdump();
8071       break;
8072     case C('U'):  // Kill line.
8073       while(input.e != input.w &&
8074             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8075         input.e--;
8076         consputc(BACKSPACE);
8077       }
8078       break;
8079     case C('H'): case '\x7f':  // Backspace
8080       if(input.e != input.w){
8081         input.e--;
8082         consputc(BACKSPACE);
8083       }
8084       break;
8085     default:
8086       if(c != 0 && input.e-input.r < INPUT_BUF){
8087         c = (c == '\r') ? '\n' : c;
8088         input.buf[input.e++ % INPUT_BUF] = c;
8089         consputc(c);
8090         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8091           input.w = input.e;
8092           wakeup(&input.r);
8093         }
8094       }
8095       break;
8096     }
8097   }
8098   release(&input.lock);
8099 }
8100 int
8101 consoleread(struct inode *ip, char *dst, int n)
8102 {
8103   uint target;
8104   int c;
8105 
8106   iunlock(ip);
8107   target = n;
8108   acquire(&input.lock);
8109   while(n > 0){
8110     while(input.r == input.w){
8111       if(proc->killed){
8112         release(&input.lock);
8113         ilock(ip);
8114         return -1;
8115       }
8116       sleep(&input.r, &input.lock);
8117     }
8118     c = input.buf[input.r++ % INPUT_BUF];
8119     if(c == C('D')){  // EOF
8120       if(n < target){
8121         // Save ^D for next time, to make sure
8122         // caller gets a 0-byte result.
8123         input.r--;
8124       }
8125       break;
8126     }
8127     *dst++ = c;
8128     --n;
8129     if(c == '\n')
8130       break;
8131   }
8132   release(&input.lock);
8133   ilock(ip);
8134 
8135   return target - n;
8136 }
8137 
8138 
8139 
8140 
8141 
8142 
8143 
8144 
8145 
8146 
8147 
8148 
8149 
8150 int
8151 consolewrite(struct inode *ip, char *buf, int n)
8152 {
8153   int i;
8154 
8155   iunlock(ip);
8156   acquire(&cons.lock);
8157   for(i = 0; i < n; i++)
8158     consputc(buf[i] & 0xff);
8159   release(&cons.lock);
8160   ilock(ip);
8161 
8162   return n;
8163 }
8164 
8165 void
8166 consoleinit(void)
8167 {
8168   initlock(&cons.lock, "console");
8169   initlock(&input.lock, "input");
8170 
8171   devsw[CONSOLE].write = consolewrite;
8172   devsw[CONSOLE].read = consoleread;
8173   cons.locking = 1;
8174 
8175   picenable(IRQ_KBD);
8176   ioapicenable(IRQ_KBD, 0);
8177 }
8178 
8179 
8180 
8181 
8182 
8183 
8184 
8185 
8186 
8187 
8188 
8189 
8190 
8191 
8192 
8193 
8194 
8195 
8196 
8197 
8198 
8199 
