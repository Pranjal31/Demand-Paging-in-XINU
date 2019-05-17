/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);
	unsigned long faddr, pdbr, pt, dstaddr;
	int vpno, ptfrm, store, pageth, freefrm, dst, i;
	virt_addr_t faultaddr;	
	unsigned int p,q;
	pd_t *pde;
	pt_t *pte;

	faddr = read_cr2();	/* get the faulted address */
	get_virt_addr(faddr, &faultaddr);
	get_virt_page_num(faultaddr, &vpno);
	pdbr = proctab[currpid].pdbr;

	p = faultaddr.pd_offset;
	q = faultaddr.pt_offset;
	
	pt = pdbr + (sizeof(pd_t)) * p;		/* pt points to the pth page table; both pd_t and pt_t are 4 bytes wide */
 	pde = (pd_t *) pt;
	
	/* the pth page table is not present */
	if(!pde->pd_pres){
		allocate_pt(currpid, &ptfrm);
		initialize_pt(ptfrm);
		
		/* update page directory entry with details of newly allocated page table */
		pde->pd_pres = 1;
	        pde->pd_write = 1;
                pde->pd_user = 0;
                pde->pd_pwt = 0;
                pde->pd_pcd = 0;
                pde->pd_acc = 0;
                pde->pd_mbz = 0;
                pde->pd_fmb = 0;
                pde->pd_global = 0;
                pde->pd_avail = 0;
		pde->pd_base = FRAME0 + ptfrm;
	}

	/* Paging in the faulted page */
	bsm_lookup(currpid, faddr, &store, &pageth);

	frm_tab[pde->pd_base - FRAME0].fr_refcnt++; 	/* one more entry in the page table */
	get_frm(&freefrm);
	dstaddr = (FRAME0 + freefrm) * NBPG;
	read_bs((char *)dstaddr , store, pageth);

	/* update inverted page table to reflect details of the page */
	frm_tab[freefrm].fr_status = FRM_MAPPED;
	frm_tab[freefrm].fr_pid = currpid;
	frm_tab[freefrm].fr_type = FR_PAGE;
	frm_tab[freefrm].fr_vpno = vpno;
	frm_tab[freefrm].fr_wrtype = WR_BASIC;
	
	/* for SC page replacement */
	frm_tab[freefrm].fr_acc = 1;	
	num_sc++;

	/* first frame into the SC circular queue */
	if(num_sc == 1)	
		sc_curr = freefrm;

	append_fifo(&fifohead, freefrm);	/* for FIFO page replacement	*/

	/* update page table */
	pte = (pt_t *) ((pde->pd_base * NBPG) + (sizeof(pt_t) * q));

	pte->pt_pres = 1;	/* page is present */
	pte->pt_base = FRAME0 + freefrm;
	restore(ps);
	return OK;
	
}


