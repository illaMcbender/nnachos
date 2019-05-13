// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    if(numSectors<=NumDirect)
    {
	    int first=freeMap->FindSeq(numSectors);
	if(first==-1)
	{
	for (int i = 0; i < numSectors; i++)
	{
		dataSectors[i] = freeMap->Find();
		 printf("find direct %d\n",dataSectors[i]);
	}
    	return TRUE;
	}
	else
	{
		for (int i = 0; i < numSectors; i++)
	        {
                	dataSectors[i] = first+i;
                 	printf("find seq %d\n",dataSectors[i]);
        	}
	}
    }
    else
    {
	    for(int i=0;i<NumDirect;i++)
	    {
		 dataSectors[i] = freeMap->Find();
	    }
	    int second[NumIndex];
	    dataSectors[NumDirect]=freeMap->Find();
	    printf("dataSectors[NumDirect]%d \n",dataSectors[NumDirect]);
	    for(int i=0;i<numSectors-NumDirect;i++)
	    {
		    second[i]=freeMap->Find();
		    printf("find second %d\n",second[i]);
	    }
	    synchDisk->WriteSector(dataSectors[NumDirect],(char *)second);
	  

    }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    if(numSectors<=NumDirect)
    {
        for (int i = 0; i < numSectors; i++)
	{	
		ASSERT(freeMap->Test((int)dataSectors[i]));
		freeMap->Clear((int)dataSectors[i]);
	}
        return ;
    }
    else
    {
            for(int i=0;i<NumDirect;i++)
            {
                ASSERT(freeMap->Test((int)dataSectors[i]));
                freeMap->Clear((int)dataSectors[i]);
            }
    }
    int left=numSectors-NumDirect;
    int *second=new int[NumIndex];
    synchDisk->ReadSector(dataSectors[NumDirect],(char *)second);
    for(int i=0;i<left;i++)
    {
	        ASSERT(freeMap->Test((int)second[i]));
                freeMap->Clear((int)second[i]);
    }
    return;


}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    int num=offset/SectorSize;
    if(num<NumDirect)
   {
	    return dataSectors[num];
	    
    }
    int *second=new int[NumIndex];
    synchDisk->ReadSector(dataSectors[NumDirect],(char *)second);
    return second[num-NumDirect];
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
   /* for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }*/
    if(numSectors<=NumDirect)
    {
	    printf("Direct Index Sector: ");
	    for(int i=0;i<numSectors;i++)
		    printf("%d ",dataSectors[i]);
    }
    else
    {
	    printf("Direct Index Sector: ");
	    for(int i=0;i<NumDirect;i++)
		    printf("%d ",dataSectors[i]);
	    int *second;
	    second=new int[NumIndex];
	    synchDisk->ReadSector(dataSectors[NumDirect],(char *)second);
	    printf("Second Index in Sector %d: ",dataSectors[NumDirect]);
	    for(int i=0;i<numSectors-NumDirect;i++)
	    {
		    printf("%d ",second[i]);
	    }
	    printf("\n");
    }
    if(numSectors<=NumDirect)
    {
       printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
        synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                printf("%c", data[j]);
            else
                printf("\\%x", (unsigned char)data[j]);
        }
        printf("\n");
    }
    }

    printf("\ntype:");
    for(int i=0;i<strlen(type);i++)
	    printf("%c",type[i]);
    printf("\n");
    printf("createTime:%d\n",createTime);
    printf("LastVisitTime:%d\n",visitTime);
    printf("LastModTime:%d\n",modTime);

    delete [] data;
}

bool 
FileHeader::Extend(BitMap *freeMap,int bytes)
{
	numBytes=numBytes+bytes;
	int pre=numSectors;
	numSectors=divRoundUp(numBytes,SectorSize);
	if(pre==numSectors)
		return TRUE;
	if(freeMap->NumClear()<numSectors-pre)
		return FALSE;
	for(int i=pre;i<numSectors;i++)
		dataSectors[i]=freeMap->Find();
	return TRUE;
}
