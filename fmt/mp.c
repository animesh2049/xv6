6950 // Multiprocessor support
6951 // Search memory for MP description structures.
6952 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
6953 
6954 #include "types.h"
6955 #include "defs.h"
6956 #include "param.h"
6957 #include "memlayout.h"
6958 #include "mp.h"
6959 #include "x86.h"
6960 #include "mmu.h"
6961 #include "proc.h"
6962 
6963 struct cpu cpus[NCPU];
6964 static struct cpu *bcpu;
6965 int ismp;
6966 int ncpu;
6967 uchar ioapicid;
6968 
6969 int
6970 mpbcpu(void)
6971 {
6972   return bcpu-cpus;
6973 }
6974 
6975 static uchar
6976 sum(uchar *addr, int len)
6977 {
6978   int i, sum;
6979 
6980   sum = 0;
6981   for(i=0; i<len; i++)
6982     sum += addr[i];
6983   return sum;
6984 }
6985 
6986 // Look for an MP structure in the len bytes at addr.
6987 static struct mp*
6988 mpsearch1(uint a, int len)
6989 {
6990   uchar *e, *p, *addr;
6991 
6992   addr = p2v(a);
6993   e = addr+len;
6994   for(p = addr; p < e; p += sizeof(struct mp))
6995     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
6996       return (struct mp*)p;
6997   return 0;
6998 }
6999 
7000 // Search for the MP Floating Pointer Structure, which according to the
7001 // spec is in one of the following three locations:
7002 // 1) in the first KB of the EBDA;
7003 // 2) in the last KB of system base memory;
7004 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
7005 static struct mp*
7006 mpsearch(void)
7007 {
7008   uchar *bda;
7009   uint p;
7010   struct mp *mp;
7011 
7012   bda = (uchar *) P2V(0x400);
7013   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
7014     if((mp = mpsearch1(p, 1024)))
7015       return mp;
7016   } else {
7017     p = ((bda[0x14]<<8)|bda[0x13])*1024;
7018     if((mp = mpsearch1(p-1024, 1024)))
7019       return mp;
7020   }
7021   return mpsearch1(0xF0000, 0x10000);
7022 }
7023 
7024 // Search for an MP configuration table.  For now,
7025 // don't accept the default configurations (physaddr == 0).
7026 // Check for correct signature, calculate the checksum and,
7027 // if correct, check the version.
7028 // To do: check extended table checksum.
7029 static struct mpconf*
7030 mpconfig(struct mp **pmp)
7031 {
7032   struct mpconf *conf;
7033   struct mp *mp;
7034 
7035   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
7036     return 0;
7037   conf = (struct mpconf*) p2v((uint) mp->physaddr);
7038   if(memcmp(conf, "PCMP", 4) != 0)
7039     return 0;
7040   if(conf->version != 1 && conf->version != 4)
7041     return 0;
7042   if(sum((uchar*)conf, conf->length) != 0)
7043     return 0;
7044   *pmp = mp;
7045   return conf;
7046 }
7047 
7048 
7049 
7050 void
7051 mpinit(void)
7052 {
7053   uchar *p, *e;
7054   struct mp *mp;
7055   struct mpconf *conf;
7056   struct mpproc *proc;
7057   struct mpioapic *ioapic;
7058 
7059   bcpu = &cpus[0];
7060   if((conf = mpconfig(&mp)) == 0)
7061     return;
7062   ismp = 1;
7063   lapic = (uint*)conf->lapicaddr;
7064   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7065     switch(*p){
7066     case MPPROC:
7067       proc = (struct mpproc*)p;
7068       if(ncpu != proc->apicid){
7069         cprintf("mpinit: ncpu=%d apicid=%d\n", ncpu, proc->apicid);
7070         ismp = 0;
7071       }
7072       if(proc->flags & MPBOOT)
7073         bcpu = &cpus[ncpu];
7074       cpus[ncpu].id = ncpu;
7075       ncpu++;
7076       p += sizeof(struct mpproc);
7077       continue;
7078     case MPIOAPIC:
7079       ioapic = (struct mpioapic*)p;
7080       ioapicid = ioapic->apicno;
7081       p += sizeof(struct mpioapic);
7082       continue;
7083     case MPBUS:
7084     case MPIOINTR:
7085     case MPLINTR:
7086       p += 8;
7087       continue;
7088     default:
7089       cprintf("mpinit: unknown config type %x\n", *p);
7090       ismp = 0;
7091     }
7092   }
7093   if(!ismp){
7094     // Didn't like what we found; fall back to no MP.
7095     ncpu = 1;
7096     lapic = 0;
7097     ioapicid = 0;
7098     return;
7099   }
7100   if(mp->imcrp){
7101     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7102     // But it would on real hardware.
7103     outb(0x22, 0x70);   // Select IMCR
7104     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7105   }
7106 }
7107 
7108 
7109 
7110 
7111 
7112 
7113 
7114 
7115 
7116 
7117 
7118 
7119 
7120 
7121 
7122 
7123 
7124 
7125 
7126 
7127 
7128 
7129 
7130 
7131 
7132 
7133 
7134 
7135 
7136 
7137 
7138 
7139 
7140 
7141 
7142 
7143 
7144 
7145 
7146 
7147 
7148 
7149 
