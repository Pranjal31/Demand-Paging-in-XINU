/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
	
	pop pferrcode		/* pop the errorcode from stack and store it in the global variable pferrcode */
	cli			/* disable/clear all interrupts */
	pushfl			/* push EFLAGS register on the stack */
	pushal			/* push general-purpose registers on the stack */
	call pfint		/* call Page Fault ISR */
	popal			/* pop all general-purpose registers from the stack */
	popfl			/* pop EFLAGS register from the stack */
	sti			/* re-enable all interrupts */
	iret			/* return from interrupt-handler procedure */

