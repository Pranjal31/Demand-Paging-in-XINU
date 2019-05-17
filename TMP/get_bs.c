#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID bs_id */
int get_bs(bsd_t bs_id, unsigned int npages) {
	
	/* bad input or a process is trying to acquire someother process' private backing store */
	if(isbadbs(bs_id) || npages == 0 || npages > NPBS || (bsm_tab[bs_id].bs_ispriv == BSM_PRIV) )
		return SYSERR;

	if( (bsm_tab[bs_id].bs_status == BSM_MAPPED) && (bsm_tab[bs_id].bs_npages != -1) )
		return bsm_tab[bs_id].bs_npages;

	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	return npages;

}


