/* frame.c - manage physical frames */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

LOCAL int get_page_to_replace();
LOCAL void update_sc_curr();
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{	
	STATWORD ps;
	disable(ps);
	int i;
	for(i=0; i < NFRAMES; i++){ 
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;
                frm_tab[i].fr_acc = 0;
		frm_tab[i].fr_wrtype = -1; 
	}

	/* for page replacement */
	fifohead = NULL;
	sc_curr = -1;
	num_sc = 0;

	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{	
	STATWORD ps;
	disable(ps);

	int i, replpg, vpage, store, pageth, pid;
        unsigned long vaddr, pdbr, pt;
        virt_addr_t virtaddr;
        unsigned int p,q;
        pd_t *pde;
        pt_t *pte;

	for(i=0; i < NFRAMES; i++){
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
			*avail = i;
			restore(ps);
			return OK;
		}
	}

	/* find a page to replace based on policy */
	replpg = get_page_to_replace();
	kprintf("Page being replaced = %d\n",FRAME0+replpg);
	
	/* free the page */
	vpage = frm_tab[replpg].fr_vpno;
	pid = frm_tab[replpg].fr_pid;
        
	free_frm(replpg);
	*avail = replpg;		

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{	
	STATWORD ps;
	disable(ps);

	/* for page replacement */
	if(frm_tab[i].fr_type == FR_PAGE)
	{
		/* remove page table entry corresponding to the frame & write page to the BS */
		remove_pte(i);

		/* for FIFO - delete frame */
		if(delete_fifo(&fifohead, i) == SYSERR){
			restore(ps);
			return SYSERR;
		}
		/* for SC - last frame in circular queue */
		num_sc--;
		if(num_sc ==0)
			sc_curr = -1;
	}
	//kprintf("unmapping in free frm\n");
        frm_tab[i].fr_status = FRM_UNMAPPED;
        frm_tab[i].fr_pid = -1;
        frm_tab[i].fr_vpno = -1;
        frm_tab[i].fr_refcnt = 0;
        frm_tab[i].fr_type = -1;
        frm_tab[i].fr_dirty = 0;
        frm_tab[i].fr_acc = 0;				/* for SC replacement 	*/
        frm_tab[i].fr_wrtype = -1;

	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * get_page_to_replace - get page to be replaced based on policy
 *-------------------------------------------------------------------------
 */
LOCAL int get_page_to_replace(){
	int i;
	
	/* Second Chance Page Replacement Policy */
	if(page_replace_policy == SC){

		/* first pass */
		i = sc_curr;

		do{
			if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE){
				if(!frm_tab[i].fr_acc){
					update_sc_curr();
					return i;
				}
				frm_tab[i].fr_acc = 0;		/* frame gets a second chance */
			}	
			i++;
			if(i >= NFRAMES)
		                i = 0;
		}while(i != sc_curr);

		/* second pass */
		i = sc_curr;
		update_sc_curr();
		return i;
	}	

	/* FIFO Page Replacement Policy */
	else if(page_replace_policy == FIFO)
		return fifo_firstkey(fifohead);	

	return SYSERR;
}


/*---------------------------------------------------------------------------------------------------------------------------------
* update_sc_curr - point sc_curr to next page in the SC circular queue - sc_curr remains unchanged if just 1 page in circular queue
*----------------------------------------------------------------------------------------------------------------------------------
*/
LOCAL void update_sc_curr(){
	int i;
	i = sc_curr + 1;
        if(i >= NFRAMES)
                i = 0;
        while(i != sc_curr){
                if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE){
                        sc_curr = i;
                        break;
                }
                i++;
                if(i >= NFRAMES)
                        i = 0;
        }
}


