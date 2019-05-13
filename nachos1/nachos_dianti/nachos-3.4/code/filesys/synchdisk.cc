// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"

//----------------------------------------------------------------------
// DiskRequestDone
// 	Disk interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
DiskRequestDone (int arg)
{
    SynchDisk* disk = (SynchDisk *)arg;

    disk->RequestDone();
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new Semaphore("synch disk", 0);
    lock = new Lock("synch disk lock");
    disk = new Disk(name, DiskRequestDone, (int) this);
    for(int i=0;i<NumSectors;i++)
    {
	    rwlock[i]=new ReadWriteLock("rwlock");
           
            num[i]=0;
    } 
    for(int i=0;i<10;i++)
    {
	cache[i].valid=0;
        cache[i].lru=0;
    }	
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
    delete disk;
    delete lock;
    delete semaphore;
//    delete []rwlock;
    
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
    int flag=-1;
    for(int i=0;i<10;i++)
    {
	    if(cache[i].valid==1 &&cache[i].sector==sectorNumber)
	    {
		    flag=i;
		    break;
	    }
    }
    if(flag!=-1)
    {
//	    printf("hit!\n");
	    cache[flag].lru++;
	    bcopy(cache[flag].data,data,SectorSize);
	    disk->HandleInterrupt();
    }
    else
    {
    	disk->ReadRequest(sectorNumber, data);
	int pos=-1;
	for(int i=0;i<10;i++)
		if(cache[i].valid==0)
		{
			pos=i;
			break;
		}
	if(pos==-1)
	{
		pos=0;
		int lru=cache[0].lru;
		for(int i=1;i<10;i++)
		{
			if(cache[i].lru<lru)
			{
				lru=cache[i].lru;
				pos=i;
			}
		}
	}
	cache[pos].valid=1;
	cache[pos].sector=sectorNumber;
	cache[pos].lru++;
	bcopy(data,cache[pos].data,SectorSize);
    }
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
    disk->WriteRequest(sectorNumber, data);
    for(int i=0;i<10;i++)
    {
	    if(cache[i].valid==1&&cache[i].sector==sectorNumber)
	    {
		    cache[i].lru++;
		    bcopy(data,cache[i].data,SectorSize);
		    break;
	    }
    }
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::RequestDone
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------

void
SynchDisk::RequestDone()
{ 
    semaphore->V();
}
