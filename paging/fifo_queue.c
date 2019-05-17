/* fifo_queue.c */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <mem.h>


/*-------------------------------------------------------------------------
 * append_fifo - append item into FIFO queue
 *-------------------------------------------------------------------------
 */
void append_fifo(struct fifo_node ** phead, int item){
	struct fifo_node *p,*temp;
	p = (struct fifo_node *)getmem(sizeof(struct fifo_node));
	p->key = item;
	p->next = NULL;
    
	if(fifo_isempty(*phead)){
		*phead = p;
		return; 
    	}
    
	temp = *phead;
        while(temp->next != NULL)
   	     temp = temp->next;
   	temp->next = p;
}

/*-------------------------------------------------------------------------
 * delete_fifo - delete an item from FIFO queue
 *-------------------------------------------------------------------------
 */
int delete_fifo(struct fifo_node **phead, int item){

        struct fifo_node *temp, *prev;
        if(fifo_isempty(*phead))
            return SYSERR;
        
	temp = *phead;
	prev = *phead;

        /* removing first node */
        if((*phead)->key == item){
            *phead = (*phead)->next;
            freemem((struct mblock *)temp, sizeof(struct fifo_node));
            return item;
        }

	while(temp != NULL && temp->key != item){
		prev = temp;
		temp = temp->next;
	}
	if(temp->key == item){
		prev->next = temp->next;
		freemem((struct mblock *)temp, sizeof(struct fifo_node));
            	return item;
	}
        return SYSERR;
}


