**Objective:** The goal of this project is to implement Demand Paging in XINU OS.

**Introduction:** 
Demand paging is a method of mapping a large address space into a relatively small amount of physical memory. It allows a program to use an address space that is larger than the physical memory, and access non-contiguous sections of the physical memory in a contiguous way. Demand paging is accomplished by using a “backing store” (usually disk) to hold pages of memory that are not currently in use.

**System Calls:**
*SYSCALL xmmap (int virtpage, bsd_t source, int npages)*

Much like its Unix counterpart (see man mmap), it maps a source file (“backing store” here) of size npages pages to the virtual page virtpage. A process may call this multiple times to map data structures, code, etc.

*SYSCALL xmunmap (int virtpage)*

This call, like munmap, should remove a virtual memory mapping. See man munmap for the details of the Unix call.

*SYSCALL vcreate (int \*procaddr, int ssize, int hsize, int priority, char \*name, int nargs, long args)*

This call will create a new Xinu process. The difference from create() is that the process’ heap will be private and exist in its virtual memory.
The size of the heap (in number of pages) is specified by the user through hsize.

Processes created with create() should not have a private heap, but should still be able to use xmmap().

WORD *vgetmem (int nbytes)

Much like getmem(), vgetmem() will allocate the desired amount of memory if possible. The difference is that vgetmem() will get the memory from a process’ private heap located in virtual memory. getmem() still allocates memory from the regular Xinu kernel heap.

SYSCALL srpolicy (int policy)

This function will be used to set the page replacement policy to Second-Chance (SC) or Least Recently Used (FIFO). You can declare constant SC as 3 and FIFO as 4 for this purpose.

SYSCALL vfreemem (block_ptr, int size_in_bytes)

vfreemem() takes two parameters and returns OK or SYSERR. The two parameters are similar to those of the original freemem() in Xinu. The type of the first parameter block_ptr depends on your own implementation.

**Overall Organization**
The following sections discuss at a high level the organization of the system, the various pieces that were implemented in Xinu and how they relate to each other.

*(1) Memory and Backing Store*

(1.1) Backing Stores

Virtual memory commonly uses disk space to extend the physical memory. However, our version of Xinu has no file system support. Therefore, the backing store was emulated (how it is emulated will be detailed in 1.3). To access the backing store, the following functions were implemented:

* bsd_t is the type of backing store descriptors. Each descriptor is used to reference a backing store. Its type declaration is in h. This type is merely unsigned int. There are 16 backing stores. You will use IDs 0 through 15 to identify them.
* int get_bs (bsd_t store, unsigned int npages) requests a new backing store with ID store of size npages (in pages, not bytes). If a new backing store can be created, or a backing store with this ID already exists, the size of the new or existing backing store is returned. This size is in pages. If a size of 0 is requested, or the creation encounters an error, SYSERR should be returned. Also for practical reasons, npages should be no more than 128.
* int release_bs (bsd_t store) releases the backing store with the ID store.
* SYSCALL read_bs (char *dst, bsd_t store, int page) copies the page-th page from the backing store referenced by store to dst. It returns OK on success, SYSERR otherwise. The first page of a backing store is page zero.
* SYSCALL write_bs (char *src, bsd_t store, int page) copies a page referenced by src to the page-th page of the backing store referenced by store. It returns OK on success, SYSERR otherwise.

(1.2) Memory Layout

The basic Xinu memory layout is as follows (page size = 4096 bytes):

———————————
Virtual memory

(pages 4096 & beyond)

———————————
3072 frames

(pages 1024 – 4095)

———————————

Kernel Memory 
(pages 406 – 1023)

———————————
Kernel Memory
(pages 160 – 405)

———————————
Kernel Memory 
(pages 25 – 159)

———————————
Xinu text, data, bss 
(pages 0 – 24)
———————————-

As you can see, this Xinu version compiles to about 100KB, or 25 pages. There is an area of memory from page 160 through the end of page 405 that cannot be used (this is referred to as the “HOLE” in initialize.c). The free frames would be placed into pages 1024 through 4095, giving 3072 frames.

The frames are used to store resident pages, page directories, and page tables. The remaining free memory below page 4096 is used for Xinu’s kernel heap (organized as a freelist). getmem()and getstk() will obtain memory from this area (from the bottom and top, respectively).

All memory below page 4096 is global. That is, it is usable and visible by all processes and accessible by simply using actual physical addresses. As a result, the first four page tables for every process must be the same, and thus should be shared.

Memory at page 4096 and above constitute a process’ virtual memory. This address space is private and visible only to the process which owns it. Note that the process’ private heap and (optionally) stack are located somewhere in this area.

(1.3) Backing Store Emulation

Since our version of Xinu does not have file system support, we need to emulate the backing store with physical memory. In particular, consider the following Xinu memory layout:

———————————
Virtual Memory

(pages 4096 & beyond)

———————————
16 Backing stores

(pages 2048 – 4095)

———————————
1024 frames

(pages 1024 – 2047)

———————————

Kernel Memory 
(pages 406 – 1023)

———————————
Kernel Memory
(pages 160 – 405)

———————————
Kernel Memory
(pages 25 – 159)

———————————
Xinu text, data, bss 
(pages 0 – 24)
———————————-

A Xinu instance has 16 MB (4096 pages) of real memory in total. The top 8MB real memory  was reserved for backing stores. There are 16 backing stores and each backing store maps up to 128 pages (each page is 4K size).

(1.4) Page Tables and Page Directories

Page tables and page directories (i.e. outer page tables) can be placed in any free frames. For this project, page tables and page directories were not paged. As page tables are always resident in memory, it is not practical to allocate all potential page tables for a process when it is created (however, a page directory would be allocated). To map all 4 GB of memory would require 4 MB of page tables! To conserve memory, page tables must be created on-demand. That is, the first time a page is legally touched (i.e. it has been mapped by the process) for which no page table is present, a page table should be allocated. Conversely, when a page table is no longer needed it should be removed to conserve space.

*(2) Supporting Data Structures*

(2.1) Finding the backing store for a virtual address

There is a problem – if a process can map multiple address ranges to different backing stores, how does one figure out which backing store a page needs to be read from (or written to) when it is being brought into (removed from) a frame?

To solve the problem, which backing store is allocated when a process is created by vcreate() was kept track of. Then, a particular page to write/read from can be calculated using its virtual page number within the related store. To achieve this a new kernel data structure which maps virtual address spaces to backing store descriptors was declared. It was called the backing store map. It is a tuple like:

{ pid, vpage, npages, store }

a function that performs the lookup was also implemented:

f (pid , vaddr)= > {store, pageoffset within store}

The function xmmap() adds a mapping to this table. xmunmap() removes a mapping from this table.

(2.2) Inverted Page Table

When writing out a dirty page, the only way to figure out which virtual page and process (and thus which backing store) a dirty frame belongs to would be to traverse the page tables of every process looking for a frame location that corresponds to the frame we wish to write out. This is highly inefficient. To prevent this, another kernel data structure, an inverted page table, was used. The inverted page table contains tuples like:

{ frame number, pid, virtual page number }

 With this structure, the pid and virtual page number of the page held within any frame i can be easily found. From that, one can easily find the backing store (using the backing store map) and compute which page within the backing store corresponds with the page in frame i.

Note: This table was also used to hold other information for page replacement

*(3) Process Considerations*

With each process having its own page directory and page tables, there are some new considerations in dealing with processes.

(3.1) Process Creation

When a process is created, a page directory must be created and its address recorded. Since the first 16 megabytes of each process will be mapped to the 16 megabytes of physical memory,  the process’ page directory  must be initialized accordingly. This is important as backing stores also depend on this correct mapping.

A mapping must be created for the new process’ private heap and stack , if created with vcreate(). 

(3.2) Process Destruction

When a process dies, the following should happen.

* All frames which currently hold any of its pages should be written to the backing store and be freed.
* All of its mappings should be removed from the backing store map.
* The backing stores for its heap (and stack if have chosen to implement a private stack) should be released (remember backing stores allocated to a process should persist unless the process explicitly releases them).
* The frame used for the page directory should be released.

(3.3) Context Switching

As processes get switched, the address spaces must also get switched accordingly. This is accomplished by adjusting the PDBR register with every context switch. 

(3.4) System Initialization

The NULL process is somewhat of a special case, as it builds itself in the function sysinit(). The NULL process should not have a private heap (like any processes created with create()).

The following should occur at system initialization:

* Set the DS and SS segments’ limits to their highest values. This will allow processes to use memory up to the 4 GB limit without generating general protection faults. Make sure the initial stack pointer (initsp) is set to a real physical address (the highest physical address) as it is in normal Xinu.   “Steal” physical memory frames 2048 – 4096 for backing store purposes.
* Initialize all necessary data structures.
* Create the page tables which will map pages 0 through 4095 to the physical 16 MB. These will be called the global page tables.
* Allocate and initialize a page directory for the NULL process.
* Set the PDBR register to the page directory for the NULL process.
* Install the page fault interrupt service routine.
* Enable paging.

*(4) The Interrupt Service Routine (ISR)*

A page fault triggers an interrupt 14. When an interrupt occurs the machine pushes CS:IP and then an error code (see Intel Volume III chapter 5)

———-
error code
———-
IP
———-
CS
———-
…
…

It then jumps to a predetermined point, the ISR. To specify the ISR we use the routine:

set_evec(int interrupt, (void (*isr)(void))) (see evec.c)

*(5) Faults and Replacement Policies*

(5.1) Page Faults

A page fault indicates one of two things: the virtual page on which the faulted address exists is not present or the page table which contains the entry for the page on which the faulted address exists is not present. To deal with a page fault the following must be done:

* Get the faulted address a.
* Let vp be the virtual page number of the page containing the faulted address.
* Let pd point to the current page directory.
* Check that a is a legal address (i.e. that it has been mapped in pd). If it is not, print an error message and kill the process.
* Let p be the upper ten bits of a. [What does p represent?]
* Let q be the bits [21:12] of a. [What does q represent?]
* Let pt point to the p-th page table. If the p-th page table does not exist, obtain a frame for it and initialize it.
* To page in the faulted page do the following:
    * Using the backing store map, find the store s and page offset o which correspond to vp.
    * In the inverted page table, increase the reference count of the frame that holds pt. This indicates that one more of pt’s entries is marked as “present.”
    * Obtain a free frame, f.
    * Copy the page o store s to f.
    * Update pt to mark the appropriate entry as present and set any other fields. Also set the address portion within the entry to point to frame f.

(5.2) Obtaining a Free Frame

When a free frame is needed, it may be necessary to remove a resident page from frame. How the page to be replaced is picked depends on the page replacement policy,

The function to find a free page should do the following:

* Search inverted page table for an empty frame. If one exists stop.
* Else, Pick a page to replace.
* Using the inverted page table, get vp, the virtual page number of the page to be replaced.
* Let a be vp*4096 (the first virtual address on page vp).
* Let p be the high 10 bits of a. Let q be bits [21:12] of a.
* Let pid be the pid of the process owning vp.
* Let pd point to the page directory of process pid.
* Let pt point to the pid’s p-th page table.
* Mark the appropriate entry of pt as not present.
* If the page being removed belongs to the current process, invalidate the TLB entry for the page vp using the invlpg instruction (see Intel Volume III/II).
* In the inverted page table decrement the reference count of the frame occupied by pt. If the reference count has reached zero, you should mark the appropriate entry in pd as being not present. This conserves frames by keeping only page tables which are necessary.
* If the dirty bit for page vp was set in its page table you must do the following:
    * Use the backing store map to find the store and page offset within store given pid and a. If the lookup fails, something is wrong. Print an error message and kill the process pid.
    * Write the page back to the backing store.

(5.3) Page Replacement Policies

The following two page replacement algorithms must be implemented: SC (default) and FIFO.

For SC, when a frame is allocated for a page,  the frame must be inserted into a circular queue. When a page replacement occurs, SC first looks at the current position in the queue (current position starts from the head of the queue), checks to see whether its reference bit is set (i.e., pt_acc = 1). If it is not set, the page is swapped out. Otherwise, the reference bit is cleared, the current position moves to the next page and this process is repeated. If all the pages have their reference bits set, on the second encounter, the page will be swapped out, as it now has its reference bit cleared.

For FIFO algorithm,  it must be ensured that the page that was allocated first in the main memory is the one that gets replaced first when a victim page is needed. When each page has been brought into the memory needs to be kept track of to help evict a page.
