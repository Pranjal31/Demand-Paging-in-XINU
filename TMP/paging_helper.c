/* paging_helper.c */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * printfrmtab - print inverted page table
 *-------------------------------------------------------------------------
 */
void printfrmtab(){
	int i;
	kprintf("\nprinting frame tab\n");
	for(i=0; i < NFRAMES; i++)
	{
		if(frm_tab[i].fr_status)	
			kprintf("i = %d, status = %d, type = %d, pid = %d, vpno = %d\n",i,frm_tab[i].fr_status, frm_tab[i].fr_type, frm_tab[i].fr_pid, frm_tab[i].fr_vpno);

	}
	kprintf("\n");
}

/*-------------------------------------------------------------------------
 * allocate_pd - allocate a page directory
 *-------------------------------------------------------------------------
 */
int allocate_pd(int pid, int *pdfrm){
	if( get_frm(pdfrm) == SYSERR)
		return SYSERR;
       	frm_tab[*pdfrm].fr_type = FR_DIR;
        frm_tab[*pdfrm].fr_status = FRM_MAPPED;
        frm_tab[*pdfrm].fr_pid = pid;
        /* No need to update/maintain refcnt for PD, as we don't try to conserve frame occupied by directory */

	return OK;
}

/*-------------------------------------------------------------------------
 * initialize_pd - initialize the page directory
 *-------------------------------------------------------------------------
 */
void initialize_pd(int pdfrm){
	int i;
	pd_t *pde;

	pde = (pd_t *)((FRAME0 + pdfrm) * NBPG);
        for(i=0; i < NPDE; i++){

		/* Global Page Tables */
		if(i < 4){
			pde->pd_pres = 1;
			pde->pd_base = FRAME0 + i;
		}
		else{
			pde->pd_pres = 0;
			/* pd_base is irrelevant as the page table is not present */
		}

                pde->pd_write = 1;
                pde->pd_user = 0;
                pde->pd_pwt = 0;
                pde->pd_pcd = 0;
                pde->pd_acc = 0;
                pde->pd_mbz = 0;
                pde->pd_fmb = 0;
                pde->pd_global = 0;
                pde->pd_avail = 0;
                pde++;  /* next PD entry */

        }

}


/*-------------------------------------------------------------------------
 *  * allocate_pt - allocate a page table
 *   *-------------------------------------------------------------------------
 *    */
int allocate_pt(int pid, int *ptfrm){
        if( get_frm(ptfrm) == SYSERR)
                return SYSERR;

        frm_tab[*ptfrm].fr_type = FR_TBL;
        frm_tab[*ptfrm].fr_status = FRM_MAPPED;
        frm_tab[*ptfrm].fr_pid = pid;

        return OK;
}

/*-------------------------------------------------------------------------
 *  * initialize_pt - initialize page table
 *   *-------------------------------------------------------------------------
 *    */
void initialize_pt(int ptfrm){
        int i;
        pt_t *pte;

        pte = (pt_t *)((FRAME0 + ptfrm) * NBPG);
        for(i=0; i < NPTE; i++){
                pte->pt_pres = 0;
                pte->pt_write = 1;
                pte->pt_user = 0;
                pte->pt_pwt = 0;
                pte->pt_pcd = 0;
                pte->pt_acc = 0;
                pte->pt_dirty = 0;
                pte->pt_mbz = 0;
                pte->pt_global = 0;
                pte->pt_avail = 0;
		/* pt_base is irrelevant as the page is not present */
                pte++; 		/* next PT entry */

        }

}
/*-----------------------------------------------------------------------------------------
 * remove_pte - remove page table entry from page table and write to BS based on write type
 *-----------------------------------------------------------------------------------------
 */
void remove_pte(int i){
	
	int pid, vpno, write_type;
	unsigned int p,q,store,pageth;
	pd_t *pde;
	pt_t *pte;
	unsigned long vaddr, pdbr, pt;
	virt_addr_t virtaddr;

	pid = frm_tab[i].fr_pid;
	vpno = frm_tab[i].fr_vpno;
	write_type = frm_tab[i].fr_wrtype;
	
	vaddr = vpno * NBPG;	
	get_virt_addr(vaddr, &virtaddr);        
        p = virtaddr.pd_offset;
        q = virtaddr.pt_offset; 

        pdbr = proctab[pid].pdbr;
        pt = pdbr + sizeof(pd_t) * p;   
        pde = (pd_t *) pt;                                              /* pde points to pth page table */
        pte = (pt_t *) (pde->pd_base * NBPG + sizeof(pt_t) * q );       /* pte points to the page table entry that needs to be removed */

      	/* marking the page absent */
       	pte->pt_pres = 0;

        /* invalidate the TLB entry for this page if it belongs to the current process */
      	if(pid == currpid)
      		 ;                               /* invlpg instruction */

        frm_tab[pde->pd_base].fr_refcnt--;    

        /* page table has no entries - a frame can be conserved */
	if(!frm_tab[pde->pd_base].fr_refcnt)
 	   pde->pd_pres = 0;

	/* if page to be removed is dirty - write to the BS */
        if(pte->pt_dirty){
                if(bsm_lookup(pid, vaddr, &store, &pageth) == SYSERR){

			/* to avoid a never ending loop of kills */
			if(write_type != WR_FORCED){
                        	kprintf("Backing Store lookup failed! Killing process %d!\n",pid);
	                        kill(pid);
			}
                }
              	write_bs((char *)(pte->pt_base * NBPG), store, pageth);
		
        }
	

                                                  
}

/*------------------------------------------------------------------------------
 *  * get_virt_addr - convert virtual address from unsigned long to virt_addr_t
 *  *---------------------------------------------------------------------------
 *  */

int get_virt_addr(unsigned long vaddr, virt_addr_t *virtaddr){
	virtaddr->pg_offset = vaddr & 4095UL;
	virtaddr->pt_offset = (vaddr & 4190208UL)>>12;
	virtaddr->pd_offset = (vaddr & 4290772992UL)>>22;
	
	return OK;
}
/*--------------------------------------------------------------------------
 *  * get_virt_page_num - extract virtual page number from virtual address 
 *  *-----------------------------------------------------------------------
 *  */

int get_virt_page_num(virt_addr_t virtaddr, int *vpno){
	*vpno = (((int)virtaddr.pd_offset) << 10) | ((int)virtaddr.pt_offset);
	return OK;
}

