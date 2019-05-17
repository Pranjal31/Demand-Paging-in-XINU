/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* BSM_MAPPED or BSM_UNMAPPED	*/
  int bs_pid[NPROC];			/* process ids using this slot   */
  int bs_vpno[NPROC];			/* starting virtual page numbers for all processes sharing the BS */
  int bs_npages;			/* number of pages in the store */
  int bs_ispriv;			/* BSM_SHARED OR BSM_PRIV	*/
  int bs_numprocs;			/* number of processes which have acquired this BS  */
  int bs_sem;				/* semaphore mechanism ?	*/
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no; relevant only when fr_type = FR_PAGE */
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;	
  int fr_acc;				/* used for SC page replacement */
  int fr_wrtype;			/* write type for mapped page	*/
}fr_map_t;

/* Structure for FIFO queue's element */
struct fifo_node{
  int key;
  struct fifo_node *next;
};

/* FIFO Queue - inline list manipulation procedures */
#define fifo_isempty(list) (list == NULL)
#define fifo_firstkey(list) (list->key)

/* FIFO Queue - functions */
void append_fifo(struct fifo_node **, int);
int delete_fifo(struct fifo_node **, int);

extern int page_replace_policy;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
extern int 	gpt_tab[];
extern struct fifo_node *fifohead;
extern int sc_curr;
extern int num_sc;			/* number of pages in sc queue */
/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

/* paging helper functions */
int get_virt_page_num(virt_addr_t, int *);
int get_virt_addr(unsigned long, virt_addr_t *);
int allocate_pd(int , int *);
void initialize_pd(int );
int allocate_pt(int, int *);
void initialize_pt(int);
void printfrmtab();
void remove_pte(int);


#define NBPG		4096	/* number of bytes per page	*/
#define NPBS		128	/* number of pages per BS	*/
#define FVPP		4096	/* first mappable virtpage for process 	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/
//#define NFRAMES		13	/* to test page replacement	*/
#define NBS		16	/* number of backing stores	*/
#define NPTE		1024	/* number of page table entries in a page table */
#define NPDE            1024    /* number of page directory entries in a page directory */

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define BSM_SHARED	0
#define BSM_PRIV	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define FIFO 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define WR_BASIC 1
#define WR_FORCED 2

#define	isbadbs(x)	(x<0 || x>=NBS)
#define isbadvpno(x)	(x<4096 || x>=1048575)	/* pages upto 4095 can't be mapped; upper limit is 2^20 */
