/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;
	int bs, vpno;
	unsigned long bs_start_addr;

	disable(ps);
	if (nbytes==0 || proctab[currpid].vmemlist.mnext== (struct mblock *) NULL) {
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);

	for (q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist.mnext; p != (struct mblock *) NULL ; q=p,p=p->mnext){
		bs = ((long)p - BACKING_STORE_BASE)/(long)BACKING_STORE_UNIT_SIZE;
		bs_start_addr = BACKING_STORE_BASE + bs * BACKING_STORE_UNIT_SIZE;
		vpno = bsm_tab[bs].bs_vpno[currpid];
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);	
			return( (WORD *)((char *)p - (char *)bs_start_addr + (char *)(vpno * NBPG) ));		/* returning the correct virtual address corresponding to the BS address */
		} else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
                        return( (WORD *)((char *)p -(char *) bs_start_addr + (char *)(vpno * NBPG)) );            /* returning the correct virtual address corresponding to the BS address */

		}
	}
	restore(ps);
	return( (WORD *)SYSERR );
}


