/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	STATWORD ps;
	disable(ps);

	if(isbadbs(source) || isbadvpno(virtpage) || npages <= 0 || npages > NPBS || bsm_tab[source].bs_npages != -1 && (bsm_tab[source].bs_npages < npages)){
		restore(ps);
		return SYSERR;	
	}
	
	bsm_map(currpid, virtpage, source, npages);
	
	restore(ps);
	return OK;

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD ps;
	disable(ps);
	
	if(isbadvpno(virtpage)){
		restore(ps);
		return SYSERR;
	}

	bsm_unmap(currpid, virtpage, 0);

	restore(ps);
	return OK;
}
