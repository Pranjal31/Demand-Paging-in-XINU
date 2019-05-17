#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

void printBSM(){
	int i,j;

	kprintf("\n------printBSM start-------\n");
        for(i=0; i< NBS; i++){
                if(bsm_tab[i].bs_npages > 0){
			kprintf("\nBS = %d\n",i);
                        kprintf("\nstatus = %d\n",bsm_tab[i].bs_status);
                        for(j=0; j< NPROC; j++){
                                if(bsm_tab[i].bs_pid[j]){
                                        kprintf(" --- pid = %d, vpage = %d ---",j,bsm_tab[i].bs_vpno[j]);
                                }
                        }
                        kprintf("npages = %d\n", bsm_tab[i].bs_npages);
                        kprintf("Private? = %d\n", bsm_tab[i].bs_ispriv);
                        kprintf("numprocs = %d\n\n", bsm_tab[i].bs_numprocs);
                }
         }
	kprintf("\n------printBSM end-------\n");

}
void proc1_test4(char *msg, int lck) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1");

	printBSM();

        get_bs(TEST1_BS, 100);

	kprintf("\n\nAfter getting BS 1");

	printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
	kprintf("\n\nAfter xmmap");
        printBSM();

        xmunmap(PROC1_VPNO);
	kprintf("\n\nAfter unmap..");
	printBSM();

        return;
}
void proc1_test4p(char *msg, int lck) {
        char *addr;
        int i,j;

        printBSM();

        return;
}

void proc1_test7p1(char *msg, int lck) {
        char *addr;
        int i,j;
	kprintf("7p1 started\n");
        printBSM();
	sleep(5);
        return;
}

void proc1_test7p2(char *msg, int lck) {
        kprintf("7p2 started\n");
	printBSM();

	kprintf("P2 trying to acquire BS0\n");
	assert(get_bs(0, 15) == SYSERR,"Test 7p failed!");
	kprintf("Test passed\n");
	printBSM(); 
	
        return;
}


void proc1_test5(char *msg, int lck) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1");

        printBSM();

	assert(get_bs(TEST1_BS, 200) == SYSERR, "Test 5 failed");

	kprintf("Test passed\n");
        return;
}

void proc1_test61(char *msg) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1 ---> %s",msg);

        printBSM();

        get_bs(TEST1_BS, 100);

        kprintf("\n\nAfter getting BS 1 ---> %s",msg);

        printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }

	sleep(10);
        kprintf("\n\nAfter xmmap --> %s", msg);
        printBSM();

        xmunmap(PROC1_VPNO);
        kprintf("\n\nAfter unmap ----> %s",msg);
        printBSM();

        return;
}


void proc1_test62(char *msg) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1 ----> %s",msg);

        printBSM();

        get_bs(TEST1_BS, 60);

        kprintf("\n\nAfter getting BS 1---> %s", msg);

        printBSM();

        if (xmmap(0x30000, TEST1_BS, 60) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
        
        sleep(10);
	kprintf("\n\nAfter xmmap---> %s", msg);
        printBSM();

        xmunmap(0x30000);
        kprintf("\n\nAfter unmap.. --> %s", msg);
        printBSM();

        return;
}

void proc1_test8(char *msg, int lck) {
        char *addr;
        int i,j;
	int store = 0;
	int pageth = 0;
        kprintf("\n\nBefore getting BS 1");

        printBSM();

        get_bs(TEST1_BS, 100);

        kprintf("\n\nAfter getting BS 1");

        printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
        kprintf("\n\nAfter xmmap");
        printBSM();

	kprintf("lookup\n");

	assert(bsm_lookup(48, 0x40063FFF, &store, &pageth) == OK, "Test 8 failed! -lookup 1");    // expected BS 1, page 99
//	assert(bsm_lookup(48, 0x40002FFF, &store, &pageth) == OK, "Test 8 failed!");	// expected BS 1, page 2

//	assert(bsm_lookup(48, PROC1_VADDR, &store, &pageth) == OK, "Test 8 failed!");	// expected BS 1, page 0
	kprintf("store = %d \n pageth = %d\n",store,pageth);

        xmunmap(PROC1_VPNO);
        assert(bsm_lookup(48, 0x40063FFF, &store, &pageth) == SYSERR, "Test 8 failed! - lookup2");    // expected BS 1, page 99


	kprintf("Test passed\n");
	printBSM();
        return;
}

void proc1_test9a(char *msg, int lck) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1");

        printBSM();

        get_bs(TEST1_BS, 100);

        kprintf("\n\nAfter getting BS 1");

        printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                return;
        }
        kprintf("\n\nAfter xmmap");
        printBSM();
	
	xmunmap(PROC1_VPNO);
	free_bsm(TEST1_BS);
	
	kprintf("\n\nAfter free");
	printBSM();


        return;
}

void proc1_test9b(char *msg, int lck) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1");

        printBSM();

        get_bs(TEST1_BS, 70);

        kprintf("\n\nAfter getting BS 1");

        printBSM();

        if (xmmap(0x50000, TEST1_BS, 70) == SYSERR) {
                kprintf("xmmap call failed\n");
                return;
        }
        kprintf("\n\nAfter xmmap");
        printBSM();

        return;
}


void proc1_test11a(char *msg) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1 ---> %s",msg);

        printBSM();

        get_bs(TEST1_BS, 100);

        kprintf("\n\nAfter getting BS 1 ---> %s",msg);

        printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
	sleep(10);
        kprintf("\n\nAfter xmmap --> %s", msg);
        printBSM();

	
        return;
}


void proc1_test11b(char *msg) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1 ----> %s",msg);

        printBSM();

        get_bs(TEST1_BS, 60);

        kprintf("\n\nAfter getting BS 1---> %s", msg);

        printBSM();

        if (xmmap(0x30000, TEST1_BS, 60) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
	kprintf("Test\n");
	assert(release_bs(1) == SYSERR, "Test11 failed!");
	
        return;
}


void proc1_test12(char *msg) {
        char *addr;
        int i,j;
        kprintf("\n\nBefore getting BS 1 ---> %s",msg);

        printBSM();

        get_bs(TEST1_BS, 100);

        kprintf("\n\nAfter getting BS 1 ---> %s",msg);

        printBSM();

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }
        sleep(10);
        kprintf("\n\nAfter xmmap --> %s", msg);
        printBSM();
	
	xmunmap(PROC1_VPNO);
	kprintf("\n\nAfter xmunmap");
	printBSM();

	
	kprintf("\nTest -- released BS 1\n");
	assert(release_bs(1) == OK, "Test 12 failed!");
	
	printBSM();

        return;
}

proc1_testpad1(char *msg, int lck) 
{
	int *x;
	int temp;
	x = vgetmem(1000);  /* allocates some memory in the virtual heap which is in virtual memory */
	/* the following  statement will cause a page fault. The page fault handling routing will read in the required page from backing store into the main memory, set the proper page tables and the page directory entries and reexecute the statement. */

	*x = 100;
	x++;
	*x = 200;

	temp = *x;  /* You are reading back from the virtual heap to check if the previous write was successful */
	
	kprintf("temp = %d\n",temp);
	vfreemem(--x, 1000); /* frees the allocation in the virtual heap */
}

proc1_testpad2(char*msg, int lck){

	char *x;
	char temp;
	get_bs(4, 100);
	xmmap(7000, 4, 100);    /* This call simply creates an entry in the backing store mapping */
	x = 7000*4096;
	*x = 'Y';                            /* write into virtual memory, will create a fault and system should proceed as in the prev example */
	temp = *x;                        /* read back and check */
	kprintf("temp = %c\n",temp);
	xmunmap(7000);
}

proc2_testpad2(char *msg, int lck){

	char *x;
	char temp_b;
	xmmap(6000, 4, 100);
	x = 6000 * 4096;
	temp_b = *x;   /* Surprise: Now, temp_b will get the value ‘Y’ written by the process A to this backing store ‘4’ */
	kprintf("x = %u, *x = %c\n",x,*x);
	kprintf("temp_b = %c\n",temp_b);

}
void proc1_st(char *msg, int lck){

	char *addr;
        int i,j;
	printfrmtab();
	for(j=0; j < NBS; j++){		// NBS = 16
		
		kprintf("\n\nBS = %d\n",j);
	        assert( get_bs(j, NPBS) != SYSERR, "get_bs call failed!\n");		// NPBS = 128

        	assert(xmmap(PROC1_VPNO + j*NPBS, j , NPBS) != SYSERR, "xmmap call failed!\n");

        	addr = (char*) (PROC1_VADDR + j * NPBS * NBPG);		// NPBS = 128 ; NBPG = 4096
	        for (i = 0; i < NPBS; i++) {
        	        *(addr + i * NBPG) = 'P';
	        }

        	sleep(3);

	        for (i = 0; i < NPBS; i++) {
        	        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	        }

        	//xmunmap(PROC1_VPNO + j*NPBS);
	}
	printfrmtab();
//	printBSM();
       
	return;
}
proc1_sbs(char *msg, int lck){

	char *addr;
	int i;
	get_bs(TEST1_BS, 100);

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }

	addr = (char*) PROC1_VADDR;
        for (i = 0; i < 26; i++) {
                *(addr + i * NBPG) = 'A' + i;
		//sleep(0.001);
        }
	kprintf("proc1 (pid = %d) has written data...sleeping now\n",currpid);
        sleep(6);
	
	kprintf("proc1(pid = %d) is reading..expecting smalls\n",currpid);
        for (i = 0; i < 26; i++) {
                kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
        }

        xmunmap(PROC1_VPNO);
	release_bs(TEST1_BS);
        return;
}
proc2_sbs(char *msg, int lck){

	char *addr;
        int i;
        get_bs(TEST1_BS, 100);

        if (xmmap(PROC2_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }

        addr = (char*) PROC2_VADDR;

        kprintf("proc2(pid=%d) is reading..expecting capitals\n",currpid);
	
        for (i = 0; i < 26; i++) {
                kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
        }


        for (i = 0; i < 26; i++) {
                *(addr + i * NBPG) = 'a' + i;
        }
	kprintf("proc2 (pid = %d) has written data...unmapping now\n",currpid);
	
        xmunmap(PROC2_VPNO);

        release_bs(TEST1_BS);

        return;

}

void proc1_test21(char *msg, int lck) {
        int *x;

        kprintf("ready to allocate heap space\n");
	assert(vgetmem(5000) == SYSERR, "test failed\n");	/* exceeds allocation */
	kprintf("Test OK");
}
void proc1_test22(char *msg, int lck) {
        int *x,*y,*u,*v;

        kprintf("ready to allocate heap space\n");
        x = vgetmem(8);       
	y = vgetmem(8);

	*x = 100;
        *y = 300;

        kprintf("x = %u, *x = %d\n", x, *x);
        kprintf("y = %u, *y = %d\n", y, *y);

        vfreemem(x, 8);
	kprintf("\nfreed 8 bytes at x = %u\n", x);
        u = vgetmem(8);
        v = vgetmem(8);

	*u = 200;
        *v = 400;

        kprintf("u = %u, *u = %d\n", u, *u);
        kprintf("v = %u, *v = %d\n", v, *v);

}

/* single memory chunk requested more than any BS's capacity */
void proc1_test23(char *msg, int lck) {

        kprintf("ready to allocate heap space\n");
        assert(vgetmem(528384)==SYSERR,"test failed!\n");		// 528384 = 129 * 4096 bytes  --> more than the space available in any given BS 
	kprintf("Test OK\n");
}
/* an overall large memory request split into two requests that can be handled by two BSs */
void proc1_test24(char *msg, int lck) {
	int *x,*y;
        kprintf("ready to allocate heap space\n");
        assert((x = vgetmem(520192)) != SYSERR, "test failed! --> x\n");	// 520192 = 127 * 4096 bytes  
	assert((y = vgetmem(8192)) != SYSERR, "test failed! --->y \n");		     // 8192	= 2 * 4090 bytes

	*x = 500;
        *y = 600;

        kprintf("x = %u, *x = %d\n", x, *x);
        kprintf("y = %u, *y = %d\n", y, *y);


}
/* xmmmap test */
proc1_testxm(char*msg, int lck){

        get_bs(4, 50);
        assert(xmmap(7000, 4, 50)==OK,"xmmap test failed - 1.1\n");    /* This call simply creates an entry in the backing store mapping */
	sleep(6);
}
proc2_testxm(char*msg, int lck){

        assert(get_bs(4, 70) == 50, "xmmap test failed - 2.1\n");
        assert(xmmap(7000, 4, 70)==SYSERR,"xmmap test failed - 2.2\n");    /* This call simply creates an entry in the backing store mapping */

        kprintf("xmmmap test passed\n");
}
/* ------ Original Test Cases ------- */

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}
	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}
	

	for (i = 0; i < 1024; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

/* Replacement Test cases */
void proc1_sc(char *msg, int lck) {
        char *addr;
        int i;

        get_bs(TEST1_BS, 100);

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }

	addr = (char*) PROC1_VADDR;
        for (i = 0; i < 26; i++) {
                *(addr + i * NBPG) = 'A' + i;
		
		if(i==6)
		{	
			free_frm(10);
			free_frm(12);
		}
	}
	
	sleep(6);
        kprintf("\n\n");

        for (i = 0; i < 26; i++) {
                kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
        }

        xmunmap(PROC1_VPNO);
        return;

}


void proc1_fifo(char *msg, int lck) {
        char *addr;
        int i;

        get_bs(TEST1_BS, 100);

        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                sleep(3);
                return;
        }

        addr = (char*) PROC1_VADDR;
        for (i = 0; i < 26; i++) {
                *(addr + i * NBPG) = 'A' + i;
		if(i==6)
           	{
                        free_frm(10);
                        free_frm(12);
                }
        }

        sleep(6);
	kprintf("\n\n");
        for (i = 0; i < 26; i++) {
                kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
        }

        xmunmap(PROC1_VPNO);
        return;
}

int main() {
	int pid1, pid2, pid3, pid4,pid5,pid6,pid7,pid8,pid9,pid10;
	kprintf("In main\n");	

/*	pid1 = create(proc1_test12,2000,20,"A",0,NULL);
	resume(pid1);
*/


	
/*	pid1 = create(proc1_test9a,2000,20,"A",0,NULL);
	resume(pid1);
	sleep(8);

	pid2 = create(proc1_test9b,2000,20,"B",0,NULL);
	resume(pid2);
	sleep(2);
*/
	
	
/* 	pid1 = create(proc1_test8,2000,20,"A",0,NULL);
	resume(pid1);
	sleep(2);
*/	

/* 	pid1 = vcreate(proc1_test7p1, 2000,28, 20, "proc1_test7p1", 1, "1");
        resume(pid1);
        sleep(2);

        //pid2 = create(proc1_test7p2, 2000, 20, "proc1_test7p2", 1, "2");
        pid2 = vcreate(proc1_test7p2, 2000,18, 20, "proc1_test7p2", 1, "2");
        resume(pid2);
        sleep(10); 
*/

/*	pid1 = vcreate(proc1_test4p, 2000,258, 20, "proc1_test1", 0, NULL);
        resume(pid1);
        sleep(2);
*/

/*	pid1 = create(proc1_test4, 2000,20,"A",0,NULL);
	resume(pid1);
	sleep(2);
*/
/*	pid1 = create(proc1_test5, 2000,20, "A",0,NULL);
	resume(pid1);
	sleep(2);
*/
/*	pid1 = create(proc1_test61, 2000, 20, "proc1_test61", 1, "1");
        resume(pid1);
	sleep(2);

        pid2 = create(proc1_test62, 2000, 20, "proc1_test62", 1, "2");
        resume(pid2);
        sleep(10); 
*/	
/*	kprintf("BS Operations - xmmap\n");
	pid3 = create(proc1_testxm, 2000, 20, "proc1_testxm", 0, NULL);
 	pid4 = create(proc2_testxm, 2000, 20, "proc2_testxm", 0, NULL);
        resume(pid3);
        sleep(3);
        resume(pid4);
*/
/*	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
*/

/*
	kprintf("\n2: vgetmem/vfreemem\n");
	pid7 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	resume(pid7);
	sleep(3);
*/

/*	kprintf("\n2: vgetmem/vfreemem 2.1\n");
        pid10 = vcreate(proc1_test21, 2000, 1, 20, "proc1_test21", 0, NULL);
        resume(pid10);
        sleep(3);
*/
/*	kprintf("\n2: vgetmem/vfreemem 2.2\n");
        pid10 = vcreate(proc1_test22, 2000, 1, 20, "proc1_test22", 0, NULL);
        resume(pid10);
        sleep(3);
*/
/*	kprintf("\n2: vgetmem/vfreemem 2.3\n");
        pid10 = vcreate(proc1_test23, 2000, 130, 20, "proc1_test23", 0, NULL);
        resume(pid10);
        sleep(3);
*/
/*	kprintf("\n2: vgetmem/vfreemem 2.4\n");
        pid10 = vcreate(proc1_test24, 2000, 130, 20, "proc1_test24", 0, NULL);
        resume(pid10);
        sleep(3);
*/
/*	kprintf("\n3: Frame test\n");
	pid3 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid3);
	sleep(3);
*/
/*	kprintf("\nPA desc test 1\n");
	pid6 = vcreate(proc1_testpad1,2000 ,100,20,"proc1_testpad1",0,NULL);       // process A is created with a virtual heap of 100 pages 
	resume(pid6);
	sleep(3);
*/
/*	kprintf("\nPA desc test 2\n");
        pid3 = create(proc1_testpad2, 2000, 20, "proc1_testpad1", 0, NULL);
        resume(pid3);
        sleep(5);

        pid4 = create(proc2_testpad2, 2000, 20, "proc1_testpad2", 0, NULL);
        resume(pid4);
        sleep(5);
*/
/*	kprintf("stress test\n");
	pid1 = create(proc1_st, 2000, 20, "proc1_st",0,NULL);
	resume(pid1);
	sleep(60);	// so that main doesn't finish before proc1_st and main's pd is not deallocated
*/
	kprintf("shared backing store\n");
        pid8 = create(proc1_sbs, 2000, 25, "proc1_sbs", 0, NULL);
        pid9 = create(proc2_sbs, 2000, 25, "proc2_sbs", 0, NULL);

	resume(pid8);
	sleep(3);
	
	resume(pid9);
	sleep(10);
	
	kill(pid8);
	kill(pid9);


/* 	for testing replacement reduce NFRAMES in ../h/paging.h */

/*	kprintf("\nSC - Page replacement\n");
	pid1 = create(proc1_sc, 2000, 20, "proc1_sc", 0, NULL);
        resume(pid1);
        sleep(10);
*/
/*	kprintf("\nFIFO - Page Replacement\n");
	srpolicy(FIFO);
	pid2 = create(proc1_fifo, 2000, 20, "proc1_fifo", 0, NULL);
        resume(pid2);
        sleep(10);
*/
}
