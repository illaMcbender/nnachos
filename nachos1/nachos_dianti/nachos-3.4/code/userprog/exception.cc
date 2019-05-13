// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void run(char *filename)
{
	OpenFile *openfile=fileSystem->Open(filename);
            AddrSpace *space;
            if(openfile==NULL)
            {
                    printf("unable to find file %s\n",filename);
                    machine->PCAdvanced();
            }
            else
            {
                    space=new AddrSpace(openfile);
                    currentThread->space=space;	    
		    space->InitRegisters();
                    space->RestoreState();
                    printf("file %s is going to run\n",filename);
                    machine->Run();
                    ASSERT(FALSE);
            }

}

void fork(int func)
{
	machine->WriteRegister(PCReg,func);
	machine->WriteRegister(NextPCReg,func+sizeof(int));
	currentThread->SaveUserState();
	machine->Run();
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    if(which==SyscallException&&type==SC_Exit)
    {
	    printf("Exit!\n");
	    for(int i=0;i<NumPhysPages;i++)
	    {
		    if(machine->pageTable[i].threadID==currentThread->getTid())
		    {
			printf("Thread:%d,PhysPage:%d,VirtualPage:%d,Valid:%d\n",currentThread->getTid(),i,machine->pageTable[i].virtualPage,machine->pageTable[i].valid?1:0);
		    	//printf("clear page %d\n",i);
	    	    	//machine->bitmap->Clear(i);
		    }
	    }
	   // currentThread->Suspend();
	    int next=machine->ReadRegister(NextPCReg);
	    machine->WriteRegister(PCReg,next);
	    currentThread->Finish();
    }
   /* else if(which==SyscallException&&type==SC_Create)
    {
	int base=machine->ReadRegister(4);
	int value;
	int len=0;
	char filename[20];
	while(1)
	{
		while(machine->ReadMem(base+len,1,&value)==FALSE)
		{}
		if(value==0)
		{
			filename[len]='\0';
			break;
		}
		filename[len++]=char(value);
	}
	fileSystem->Create(filename,128);
	do
	{
		machine->ReadMem(base++,1,&value);
		printf("%d \n",value);
		len++;
	}while(value!=0);
	printf("len %d\n",len);
	base-=len;
	for(int i=0;i<len;i++)
	{
		machine->ReadMem(base+i,1,&value);
		filename[i]=(char)value;
	}
	/file[len]='\0';
	//printf("len %d\n",len);
	printf("name %s\n",file);
	fileSystem->Create(file,1024);
	printf("create success\n");
	machine->PCAdvanced();
    }
    else if(which==SyscallException&&type==SC_Open)
    {
	    int base=machine->ReadRegister(4);
	    int value;
	    int len=0;
	    do
	    {
		    machine->ReadMem(base++,1,&value);
		    len++;
	    }while(value!=0);
	    base-=len;
	    char file[len];
	    for(int i=0;i<len;i++)
	    {
		    machine->ReadMem(base+i,1,&value);
		    file[i]=(char)value;
	    }
	    OpenFile *openfile=fileSystem->Open(file);
	    int id=openfile->getID();
	    printf("file %s open succeed\n",file);
	    machine->WriteRegister(2,id);
	    machine->PCAdvanced();
    }
    else if(which==SyscallException&&type==SC_Close)
    {
	    int id=machine->ReadRegister(4);
	    Close(id);
	    printf("file id %d close succeed\n",id);
	    machine->PCAdvanced();
    
    }
    else if(which==SyscallException&&type==SC_Write)
    {
	    int base=machine->ReadRegister(4);
	    int size=machine->ReadRegister(5);
	    int id=machine->ReadRegister(6);
	    int value;
	    int len=0;
	    do
	    {
		    machine->ReadMem(base++,1,&value);
		    len++;
	    }while(value!=0);
	    base-=len;
	    char str[len];
	    for(int i=0;i<len;i++)
	    {
		    machine->ReadMem(base+i,1,&value);
		    str[i]=(char)value;
	    }
	    OpenFile *openfile=new OpenFile(id);
	    openfile->Write(str,size);
	    printf("file id %d write succeed\n",id);
	    delete openfile;
	    machine->PCAdvanced();
    }
      else if(which==SyscallException&&type==SC_Read)
    {
            int base=machine->ReadRegister(4);
            int size=machine->ReadRegister(5);
            int id=machine->ReadRegister(6);
	    char str[size];
            OpenFile *openfile=new OpenFile(id);
	    int truesize=0;
            truesize=openfile->Read(str,size);
	    str[size]='\0';
	    for(int i=0;i<size;i++)
		    machine->WriteMem(base+i,1,str[i]);
            printf("file id %d read succeed\n",id);
	    printf("the string file read is %s\n",str);
	    machine->WriteRegister(2,truesize);
            delete openfile;
            machine->PCAdvanced();
    }
    else if(which==SyscallException&&type==SC_Exec)
    {
	    int base=machine->ReadRegister(4);
            int value;
            int len=0;
            char *filename=new char[20];
            while(1)
            {
                while(machine->ReadMem(base+len,1,&value)==FALSE)
                {}
                if(value==0)
                {
                        filename[len]='\0';
                        break;
                }
                filename[len++]=char(value);
       	    }
	    Thread *t=new Thread("t");
	    t->Fork(run,filename);
	    machine->WriteRegister(2,t->getTid());
	    machine->PCAdvanced();
            OpenFile *openfile=fileSystem->Open(filename);
	    AddrSpace *space;
	    if(openfile==NULL)
	    {
		    printf("unable to find file %s\n",filename);
		    machine->PCAdvanced();
	    }
	    else
	    {
		    space=new AddrSpace(openfile);
		    currentThread->space=space;
		    space->InitRegisters();
		    space->RestoreState();
		   // delete space;
		    printf("file %s is going to run\n",filename);
		    machine->Run();
		    printf(">>>\n");
            	    ASSERT(FALSE);
	    }
    }
    else if(which==SyscallException&&type==SC_Fork)
    {
            int func=machine->ReadRegister(4);
            AddrSpace *space=currentThread->space;
            Thread *t=new Thread("t");
	    t->space=space;
            t->Fork(fork,func);
            machine->PCAdvanced();
    }
   else if(which==SyscallException&&type==SC_Yield)
    {
            printf("thread %d yield\n",currentThread->getTid());
            machine->PCAdvanced();
	    currentThread->Yield();
    }
    else if(which==SyscallException&&type==SC_Join)
    {
	    int tid=machine->ReadRegister(4);
	    while(tidNum[tid])
		    currentThread->Yield();
	    machine->PCAdvanced();
    }*/
    else if(which==PageFaultException)
    {
	if(machine->tlb!=NULL)
	{
	    int vpn=(unsigned)machine->registers[BadVAddrReg]/PageSize;
	    int pos=-1;
	    for(int i=0;i<TLBSize;i++)
	    {
		    if(machine->tlb[i].valid==false)
		    {
			 pos=i;
			 break;
		    }
	    }
	    if(pos==-1)
	    {
		  /*
		 for(int i=1;i<TLBSize;i++)
			 machine->tlb[i-1]=machine->tlb[i];
		 pos=TLBSize-1;
		 */
		   
		 int mmax=machine->tlb[0].LRU;
		 pos=0;
		 for(int i=1;i<TLBSize;i++)
			if(machine->tlb[i].LRU>mmax)
			{
				mmax=machine->tlb[i].LRU;
				pos=i;
			}
	    }
	    for(int i=0;i<TLBSize;i++)
		    if(i!=pos)
			    machine->tlb[i].LRU++;
	    machine->tlb[pos].LRU=0;
	    machine->tlb[pos].valid=true;
	    machine->tlb[pos].virtualPage=vpn;
	    machine->tlb[pos].physicalPage=machine->pageTable[vpn].physicalPage;
	    machine->tlb[pos].use=false;
	    machine->tlb[pos].dirty=false;
	    machine->tlb[pos].readOnly=false;
		   // machine->tlb[pos].LRU=0;
		   // for(int i=0;i<TLBSize;i++)
		    //{
		//	    if(i!=pos)
		//		   machine->tlb[i].LRU++;
		  //  }
	}
	else
	{
	    int vpn=(unsigned)machine->registers[BadVAddrReg]/PageSize;
	    int pos=-1;
	    for(int i=0;i<NumPhysPages;i++)
	    {
		    if(machine->pageTable[i].valid==false)
		    {
			pos=i;
			break;
		    }
	    }
	    if(pos==-1)
	    {
		    if(machine->pageTable[0].dirty==true)
		    {
			    int old=machine->pageTable[0].virtualPage;
			    int tid=machine->pageTable[0].threadID;
			    memcpy(&(tidThread[tid]->space->myDisk[old*PageSize]),&(machine->mainMemory[0]),PageSize);
		    	   
		    }
		    for(int i=1;i<NumPhysPages;i++)
                    {
                        machine->pageTable[i-1]=machine->pageTable[i];
                    }
		    pos=0;
	    }
	    machine->pageTable[pos].virtualPage=vpn;
	    machine->pageTable[pos].valid=true;
	    machine->pageTable[pos].threadID=currentThread->getTid();
	    int paddr=pos*PageSize;
	    memcpy(&(machine->mainMemory[paddr]),&(currentThread->space->myDisk[vpn*PageSize]),PageSize);

	    printf("Find Page %d\n",pos);
	}
    }
    else if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
