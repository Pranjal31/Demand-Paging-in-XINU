/* vcreate.c - vcreate */
//#include <stdlib.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>
/*
 * static unsigned long esp;
 * */

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  *  create  -  create a process to start running a procedure
 *   *------------------------------------------------------------------------
 *    */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;    
	int		pid;		/* stores new process id	*/
	int 		bs;		/* private backing store	*/
	unsigned long 	unmap_vpno;	/* first unmapped virtual page for the current process	*/
	struct mblock 	*mptr;		/* pointer to free memory block */
	
	disable(ps);

	if (hsize <= 0 || hsize > NBS*NPBS) {	/* max heap size is when the heap occupies all 16 backing stores completely */
		restore(ps);
		return(SYSERR);
	}

	pid = create(procaddr,ssize,priority,name,nargs,args);
	
	unmap_vpno = FVPP;
	mptr = &proctab[pid].vmemlist;
	
	/* backing store allocation and virtual heap mapping for process */
	while(hsize >0){
		/* BS not available */
		if(get_bsm(&bs) == SYSERR){
			restore(ps);
			return SYSERR;
		}

		mptr->mnext = (struct mblock*)  (BACKING_STORE_BASE + bs * BACKING_STORE_UNIT_SIZE);

		/* this won't be the last BS allocated to the proc */
		if(hsize > NPBS){
			bsm_map(pid, unmap_vpno, bs, NPBS);	
			unmap_vpno += NPBS;	
			hsize -= NPBS;
			mptr->mnext->mlen = NPBS * NBPG;
			mptr = mptr->mnext;
		}
                /* this will be the last BS allocated to the proc */
		else{
			bsm_map(pid, unmap_vpno, bs, hsize);
			unmap_vpno += hsize;
			mptr->mnext->mlen = hsize * NBPG;
			mptr->mnext->mnext = NULL;
			hsize = 0;
		}

		bsm_tab[bs].bs_ispriv = BSM_PRIV;	/* the backing store is private to the process */

	}
	

	restore(ps);
	return(pid);
	
}

/*------------------------------------------------------------------------
 *  * newpid  --  obtain a new (free) process id
 *   *------------------------------------------------------------------------
 *    */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}

