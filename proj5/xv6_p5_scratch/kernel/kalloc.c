// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  /*
  P5 changes
  */
  uint free_pages; //track free pages
  uint ref_cnt[PHYSTOP / PGSIZE]; //track reference count

} kmem;

extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;

  initlock(&kmem.lock, "kmem");
  p = (char*)PGROUNDUP((uint)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || PADDR(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  //memset(v, 1, PGSIZE);


  acquire(&kmem.lock);
  r = (struct run*)v;

  if(kmem.ref_cnt[PADDR(v) >> PGSHIFT] > 0)         // decrease reference count of page when freed
    kmem.ref_cnt[PADDR(v) >> PGSHIFT] = kmem.ref_cnt[PADDR(v) >> PGSHIFT] - 1;

  if(kmem.ref_cnt[PADDR(v) >> PGSHIFT] == 0){       // Free page only if no references to the page
    
    memset(v, 1, PGSIZE);     // Fill garbage to catch dangling refs.
    r->next = kmem.freelist;
    kmem.free_pages = kmem.free_pages + 1;
    kmem.freelist = r;
    
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    kmem.ref_cnt[PADDR((char*)r) >> PGSHIFT] = 1;    
    kmem.free_pages = kmem.free_pages - 1;
  }
  release(&kmem.lock);
  return (char*)r;
}

int 
getFreePagesCount(void)
{
  return kmem.free_pages;
}

void
incrRefCnt(uint pa){
  if(pa >= PHYSTOP || pa < (uint)PADDR(end))
    return; 

  acquire(&kmem.lock);

  kmem.ref_cnt[pa >> PGSHIFT] = kmem.ref_cnt[pa >> PGSHIFT] + 1;

  release(&kmem.lock); 
}

void
decrRefCnt(uint pa){
  if(pa >= PHYSTOP || pa < (uint)PADDR(end))
    return; 

  acquire(&kmem.lock);

  kmem.ref_cnt[pa >> PGSHIFT] = kmem.ref_cnt[pa >> PGSHIFT] - 1;

  release(&kmem.lock); 
}

int 
isFree(void) {
  return ((char*)kmem.freelist == 0);
}

int 
freeCnt(uint index) {
  return (kmem.ref_cnt[index]);
}
