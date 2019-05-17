#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* release the backing store with ID bs_id */
SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps; 
	disable(ps);

	int i;
	
	/* bad input or entries have not been unmapped before calling release - raise an error */
	if(isbadbs(bs_id) || bsm_tab[bs_id].bs_numprocs){ 
		restore(ps);
		return SYSERR;
	}
	/* already unmapped - nothing to do */
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
		return OK;
	
	/* additional check - entries have not been unmapped before calling release - raise an error */
	for(i=0; i < NPROC; i++){
		if(bsm_tab[bs_id].bs_pid[i]){
			restore(ps);
			return SYSERR;
		}
	}	
	
	/* safe to release the BS */
	free_bsm(bs_id);	
	
	restore(ps);
	return OK;

}

