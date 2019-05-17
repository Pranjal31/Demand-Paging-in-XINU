/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{	
	STATWORD ps; 
	disable(ps);
	
	int i,j;

	for(i=0; i < NBS; i++){
		bsm_tab[i].bs_status =  BSM_UNMAPPED;

		for(j=0; j < NPROC; j++){		
			bsm_tab[i].bs_pid[j] = 0;
        	        bsm_tab[i].bs_vpno[j] = -1;
		}

                bsm_tab[i].bs_npages = -1;
		bsm_tab[i].bs_ispriv = BSM_SHARED;
		bsm_tab[i].bs_numprocs = 0;
		
	}
	restore(ps);
	return OK;	 
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{	
        STATWORD ps; 
        disable(ps);
	
	int i;
	for(i=0; i< NBS; i++){
		if(bsm_tab[i].bs_status == BSM_UNMAPPED)
			{	
				bsm_tab[i].bs_status = BSM_MAPPED;
				*avail = i;
				restore(ps);
				return OK;
			}
			
	}
	restore(ps);
	return SYSERR;	/* no backing store is available */
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{	
        STATWORD ps; 
        disable(ps);

	if(isbadbs(i)){ 
		restore(ps);
		return SYSERR;
	}
	
	bsm_tab[i].bs_status = BSM_UNMAPPED;

	if(bsm_tab[i].bs_ispriv == BSM_PRIV)	
		bsm_tab[i].bs_ispriv = BSM_SHARED;
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{	
        STATWORD ps;
        disable(ps);

	/* sanity check */
       if(isbadpid(pid)){
		restore(ps);
                return SYSERR;	        
	}

	int vpno,i;
	virt_addr_t virtaddr;
	get_virt_addr(vaddr, &virtaddr);
	get_virt_page_num(virtaddr,&vpno);
	
	for(i=0; i < NBS; i++){

		if(bsm_tab[i].bs_pid[pid] && bsm_tab[i].bs_vpno[pid] != -1 && vpno >= bsm_tab[i].bs_vpno[pid] && vpno < bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages){
			*store = i;
			*pageth = vpno - bsm_tab[i].bs_vpno[pid]; 
			restore(ps); 
			return OK;

		}
	}
	restore(ps);
	return SYSERR;	
	
}


/*-------------------------------------------------------------------------
 * bsm_map - add a mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{	
        STATWORD ps;
        disable(ps);

	/* sanity check */
	if(isbadpid(pid) || isbadvpno(vpno) || isbadbs(source) || npages <= 0 || npages > NPBS || bsm_tab[source].bs_ispriv == BSM_PRIV){
                restore(ps);
                return SYSERR;
        }
		
	bsm_tab[source].bs_pid[pid] = 1;
	bsm_tab[source].bs_numprocs = bsm_tab[source].bs_numprocs + 1;
        bsm_tab[source].bs_vpno[pid] = vpno;

	/* For a shared BS, the first process acquiring the BS decides the value of npages; For a private BS, it doesn't matter - there will be only 1 process */
	if(bsm_tab[source].bs_npages == -1)
	        bsm_tab[source].bs_npages = npages;
	
	restore(ps);
	return OK;		
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete a mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
        disable(ps);

        /* sanity check */
	if(isbadpid(pid) || isbadvpno(vpno)){
		restore(ps);
		return SYSERR;
	}
	int i,j,pg;
	unsigned int p,q;
	unsigned long vaddr, pdbr, pt;
	virt_addr_t virtaddr;
	pd_t *pde;
        pt_t *pte;

	int store, pageth;

	pdbr = proctab[pid].pdbr;
	
	for(i=0;i<NBS; i++){
		
                if(bsm_tab[i].bs_pid[pid] && bsm_tab[i].bs_vpno[pid] != -1 && vpno >= bsm_tab[i].bs_vpno[pid] && vpno < bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages){
			
			/* write the dirty pages to the backing store */
			
		   	for(j=0; j < NFRAMES; j++){

                        	if(frm_tab[j].fr_pid == pid && frm_tab[j].fr_type == FR_PAGE)    /* if all page table entries are removed - page table frames will automatically be freed/conserved */
					free_frm(j);
	                        
        	        }	
			bsm_tab[i].bs_pid[pid] = 0;
			bsm_tab[i].bs_vpno[pid] = -1;
			
			bsm_tab[i].bs_numprocs = bsm_tab[i].bs_numprocs - 1;
			
			/* it was the last process (for shared BS) or the only process (for private BS) that was mapped to the BS */
			if(!bsm_tab[i].bs_numprocs){
				bsm_tab[i].bs_npages = -1;
				free_bsm(i);
			}
			restore(ps);
			return OK;
		}
		
	}
	restore(ps);
	return SYSERR;	
}
