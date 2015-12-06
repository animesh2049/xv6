4150 // Simple PIO-based (non-DMA) IDE driver code.
4151 
4152 #include "types.h"
4153 #include "defs.h"
4154 #include "param.h"
4155 #include "memlayout.h"
4156 #include "mmu.h"
4157 #include "proc.h"
4158 #include "x86.h"
4159 #include "traps.h"
4160 #include "spinlock.h"
4161 #include "fs.h"
4162 #include "buf.h"
4163 
4164 #define SECTOR_SIZE   512
4165 #define IDE_BSY       0x80
4166 #define IDE_DRDY      0x40
4167 #define IDE_DF        0x20
4168 #define IDE_ERR       0x01
4169 
4170 #define IDE_CMD_READ  0x20
4171 #define IDE_CMD_WRITE 0x30
4172 
4173 // idequeue points to the buf now being read/written to the disk.
4174 // idequeue->qnext points to the next buf to be processed.
4175 // You must hold idelock while manipulating queue.
4176 
4177 static struct spinlock idelock;
4178 static struct buf *idequeue;
4179 
4180 static int havedisk1;
4181 static void idestart(struct buf*);
4182 
4183 // Wait for IDE disk to become ready.
4184 static int
4185 idewait(int checkerr)
4186 {
4187   int r;
4188 
4189   while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
4190     ;
4191   if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
4192     return -1;
4193   return 0;
4194 }
4195 
4196 
4197 
4198 
4199 
4200 void
4201 ideinit(void)
4202 {
4203   int i;
4204 
4205   initlock(&idelock, "ide");
4206   picenable(IRQ_IDE);
4207   ioapicenable(IRQ_IDE, ncpu - 1);
4208   idewait(0);
4209 
4210   // Check if disk 1 is present
4211   outb(0x1f6, 0xe0 | (1<<4));
4212   for(i=0; i<1000; i++){
4213     if(inb(0x1f7) != 0){
4214       havedisk1 = 1;
4215       break;
4216     }
4217   }
4218 
4219   // Switch back to disk 0.
4220   outb(0x1f6, 0xe0 | (0<<4));
4221 }
4222 
4223 // Start the request for b.  Caller must hold idelock.
4224 static void
4225 idestart(struct buf *b)
4226 {
4227   if(b == 0)
4228     panic("idestart");
4229   if(b->blockno >= FSSIZE)
4230     panic("incorrect blockno");
4231   int sector_per_block =  BSIZE/SECTOR_SIZE;
4232   int sector = b->blockno * sector_per_block;
4233 
4234   if (sector_per_block > 7) panic("idestart");
4235 
4236   idewait(0);
4237   outb(0x3f6, 0);  // generate interrupt
4238   outb(0x1f2, sector_per_block);  // number of sectors
4239   outb(0x1f3, sector & 0xff);
4240   outb(0x1f4, (sector >> 8) & 0xff);
4241   outb(0x1f5, (sector >> 16) & 0xff);
4242   outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
4243   if(b->flags & B_DIRTY){
4244     outb(0x1f7, IDE_CMD_WRITE);
4245     outsl(0x1f0, b->data, BSIZE/4);
4246   } else {
4247     outb(0x1f7, IDE_CMD_READ);
4248   }
4249 }
4250 // Interrupt handler.
4251 void
4252 ideintr(void)
4253 {
4254   struct buf *b;
4255 
4256   // First queued buffer is the active request.
4257   acquire(&idelock);
4258   if((b = idequeue) == 0){
4259     release(&idelock);
4260     // cprintf("spurious IDE interrupt\n");
4261     return;
4262   }
4263   idequeue = b->qnext;
4264 
4265   // Read data if needed.
4266   if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
4267     insl(0x1f0, b->data, BSIZE/4);
4268 
4269   // Wake process waiting for this buf.
4270   b->flags |= B_VALID;
4271   b->flags &= ~B_DIRTY;
4272   wakeup(b);
4273 
4274   // Start disk on next buf in queue.
4275   if(idequeue != 0)
4276     idestart(idequeue);
4277 
4278   release(&idelock);
4279 }
4280 
4281 
4282 
4283 
4284 
4285 
4286 
4287 
4288 
4289 
4290 
4291 
4292 
4293 
4294 
4295 
4296 
4297 
4298 
4299 
4300 // Sync buf with disk.
4301 // If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
4302 // Else if B_VALID is not set, read buf from disk, set B_VALID.
4303 void
4304 iderw(struct buf *b)
4305 {
4306   struct buf **pp;
4307 
4308   if(!(b->flags & B_BUSY))
4309     panic("iderw: buf not busy");
4310   if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
4311     panic("iderw: nothing to do");
4312   if(b->dev != 0 && !havedisk1)
4313     panic("iderw: ide disk 1 not present");
4314 
4315   acquire(&idelock);  //DOC:acquire-lock
4316 
4317   // Append b to idequeue.
4318   b->qnext = 0;
4319   for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
4320     ;
4321   *pp = b;
4322 
4323   // Start disk if necessary.
4324   if(idequeue == b)
4325     idestart(b);
4326 
4327   // Wait for request to finish.
4328   while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
4329     sleep(b, &idelock);
4330   }
4331 
4332   release(&idelock);
4333 }
4334 
4335 
4336 
4337 
4338 
4339 
4340 
4341 
4342 
4343 
4344 
4345 
4346 
4347 
4348 
4349 
