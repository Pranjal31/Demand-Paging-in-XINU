/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	STATWORD ps;
	disable(ps);

	/* sanity check */
	if(policy != SC && policy != FIFO){
		restore(ps);
		return SYSERR;			
	}
	
	page_replace_policy = policy;
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
	return page_replace_policy;
}
