// threadtest.cc 
// Simple test case for the threads assignment.
//
// Create two threads, and have them context switch
// back and forth between themselves by calling Thread::Yield, 
// to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
extern int tidNum[128];
extern int tidSum;
extern Thread *tidThread[128];
//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
// each iteration.
//
// "which" is simply a number identifying the thread, for debugging
// purposes.
//----------------------------------------------------------------------


void
SimpleThread(int which)
{
    int num; 
    for (num = 0; num < 5; num++) {
 printf("TID NUM: %d UID NUM: %d Priority: %d\n",currentThread->getTid(),currentThread->getUid(),currentThread->getPriority());
     printf("*** thread %d looped %d times\n", which, num);
      //currentThread->Yield();
    }
}

void
SimpleThread2(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
 printf("TID NUM: %d UID NUM: %d Priority: %d\n",currentThread->getTid(),currentThread->getUid(),currentThread->getPriority());
     printf("*** thread %d looped %d times\n", which, num);
     if(num==2)
     {
	     Thread *t3=new Thread("Forked Thread3",4);
	     t3->Fork(SimpleThread,1);
	}
    }
     //currentThread->Yield();
    
}

void
SimpleThread1(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
 printf("TID NUM: %d UID NUM: %d Priority: %d\n",currentThread->getTid(),currentThread->getUid(),currentThread->getPriority());
     printf("*** thread %d looped %d times\n", which, num);
    if(num==2)
    {

	    Thread *t2=new Thread("Forked Thread2",7);
	    t2->Fork(SimpleThread2,1);
	}
    }
     //currentThread->Yield();
    
}


//----------------------------------------------------------------------
// ThreadTest1
//  Set up a ping-pong between two threads, by forking a thread 
// to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------


void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t1 = new Thread("forked thread1",2);
    Thread *t2 = new Thread("forked thread2",5);
    Thread *t3 = new Thread("forked thread3",3);
    t1->Fork(SimpleThread, (void*)1);
    t2->Fork(SimpleThread, (void*)1);
    t3->Fork(SimpleThread, (void*)1);

    SimpleThread(0);
}


void ThreadTest2()
{
    DEBUG('t',"Entering ThreadTest2");
    for(int i=0;i<=128;i++)
    {     
 Thread *t=new Thread("forked thread");
 printf("TID NUM: %d UID NUM %d THREAD NAME %s\n",t->getTid(),t->getUid(),t->getName());
    }
}


void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");

    Thread *t1 = new Thread("forked thread1",2);
  
    t1->Fork(SimpleThread1, (void*)1);
   
}

Lock *lock=new Lock("locktest");
void
SimpleThread5(int which)
{
    
    int num;
    for (num = 0; num < 5; num++) {
 printf("TID NUM: %d UID NUM: %d Priority: %d\n",currentThread->getTid(),currentThread->getUid(),currentThread->getPriority());
     printf("*** thread %d looped %d times\n", which, num);
    }
  
     //currentThread->Yield();

}

void SimpleThread4(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
 printf("TID NUM: %d UID NUM: %d Priority: %d\n",currentThread->getTid(),currentThread->getUid(),currentThread->getPriority());
     printf("*** thread %d looped %d times\n", which, num);
    if(num==2)
    {

            Thread *t2=new Thread("Forked Thread2",2);
            t2->Fork(SimpleThread5,2);
        }
    }
    
     //currentThread->Yield();

}

void ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    Thread *t1 = new Thread("forked thread1",4);
    t1->Fork(SimpleThread4, (void*)1);

}

Semaphore* mutex=new Semaphore("mutex", 1);
Semaphore* empty=new Semaphore("empty", 5);
Semaphore* full=new Semaphore("full", 0);

int pos[5];
int no=0;

void Producer1(int num)
{
    while(num--)
    {
        empty->P();
        mutex->P();
        pos[no]=1;
        printf("producer put product in pos %d\n",no++);
        mutex->V();
        full->V();
    }
}

void Consumer1(int num)
{
    while(num--)
    {
        full->P();
	mutex->P();
        printf("consumer take product in pos %d\n", --no);
        mutex->V();
        empty->V();
    }
}

void ThreadTest5()
{
    Thread* t1 = new Thread("Producer");
    Thread* t2 = new Thread("Consumer");
    t1->Fork(Producer1, (void*)6);
    t2->Fork(Consumer1, (void*)8);
}

Condition* c=new Condition("condition");
Lock* PClock=new Lock("PClock");

void Producer2(int num)
{
    while(num--)
    {
        PClock->Acquire();
        while(no==5)
	{
            c->Wait(PClock);
        }
        pos[no]=1;
        printf("producer put product in pos %d\n",no++);
        if(no==1)
            c->Signal(PClock);
        PClock->Release();    
    }
}

void Consumer2(int num)
{
    while(num--)
    {
        PClock->Acquire();
        while(no==0)
	{
            c->Wait(PClock);
        }
        printf("consumer take product in pos %d\n", --no);
        if(no==4)
            c->Signal(PClock);
        PClock->Release();
    }
}

void ThreadTest6()
{
    Thread* t1 = new Thread("Producer");
    Thread* t2 = new Thread("Consumer");
    t1->Fork(Producer2, (void*)6);
    t2->Fork(Consumer2, (void*)8);
}


ReadWriteLock *mylock=new ReadWriteLock("readwritelock");
void Write2(int which)
{
	mylock->WriteLockAcquire();
	printf("Writer %d is writing\n", which);
	mylock->WriteLockRelease();
}
void Write1(int which)
{
	mylock->WriteLockAcquire();
	Thread* t4 = new Thread("Write2",4);
	t4->Fork(Write2, (void*)4);
	printf("Writer %d is Writing\n", which);
	mylock->WriteLockRelease();
}
void Read2(int which)
{
	mylock->ReadLockAcquire();
	Thread* t3 = new Thread("Write1",5);
	t3->Fork(Write1, (void*)3);
	printf("Reader %d is reading\n", which);
	mylock->ReadLockRelease();
}

void Read1(int which)
{
	mylock->ReadLockAcquire();
	Thread* t2 = new Thread("Reader2",6);
	t2->Fork(Read2, (void*)2);
	printf("Reader %d is reading\n", which);
	mylock->ReadLockRelease();
}

void ThreadTest7()
{
    Thread* t1 = new Thread("Reader1",7);
    t1->Fork(Read1, (void*)1);
}
Condition* bc=new Condition("bcondition");
Lock* block=new Lock("block");

void Barrier(int which)
{
	for(int i=0;i<3;i++)
	{
		block->Acquire();
		 printf("Thread %d tests for %d times\n",which,i+1);

		bc->Barrier(block);
	
		block->Release();
	}
}
void ThreadTest8()
{
    Thread* t1 = new Thread("Barrier1");
    Thread* t2 = new Thread("Barrier2");
    Thread* t3 = new Thread("Barrier3");

    t1->Fork(Barrier, (void*)1);

    t2->Fork(Barrier, (void*)2);

    t3->Fork(Barrier, (void*)3);
}
Semaphore* pempty=new Semaphore("empty", 1);
Semaphore* apple=new Semaphore("full", 0);
Semaphore* orange=new Semaphore("full", 0);
Semaphore* m=new Semaphore("m",1);
void D(int which)
{
	pempty->P();
	m->P();
	printf("dad\n");
	m->V();
	apple->V();
}

void M(int which)
{
        pempty->P();
	m->P();
        printf("mom\n");
	m->V();
        orange->V();
}

void S(int which)
{
        orange->P();
	m->P();
        printf("SON\n");
	m->V();
        pempty->V();
}
void DA(int which)
{
        apple->P();
	m->P();
        printf("dau\n");
	m->V();
        pempty->V();
}



void t()
{
	Thread *t1=new Thread("d");
	t1->Fork(D,(void *)1);
        Thread *t2=new Thread("d");
        t2->Fork(M,(void *)1);
        Thread *t3=new Thread("d");
        t3->Fork(S,(void *)1);
        Thread *t4=new Thread("d");
        t4->Fork(DA,(void *)1);
}





//----------------------------------------------------------------------
// ThreadTest
//  Invoke a test routine.
//----------------------------------------------------------------------
void ThreadStatus()
{
 printf("print thread status\n");
 for(int i=0;i<128;i++)
 {
     if(tidNum[i]==1)
     {
 printf("TID NUM: %d UID NUM %d\n",tidThread[i]->getTid(),tidThread[i]->getUid());


     
     }
 }
}
void
ThreadTest()
{
    switch (testnum) {
    case 1:
 ThreadTest1();
 break;
    case 2:
 ThreadTest2();
 break;
 	case 3:
 ThreadTest3();
 break;
 	case 4:
 ThreadTest4();
 break;
 	case 5:
 ThreadTest5();
 break;
 	case 6:
 ThreadTest6();
 break;
 	case 7:
 ThreadTest7();
 break;
 	case 8:
 ThreadTest8();
 break;
 	case 9:
 t();
 break;
    default:
 printf("No test specified.\n");
 break;
    }
}





