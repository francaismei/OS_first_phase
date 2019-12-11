#include "pcb.h"
#include "filedisk.h"
#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include             "string.h"
#include             <stdlib.h>
#include             <ctype.h>
#include             "studentConfiguration.h"

int diskFormat(int DiskID)
{
    if (DiskID >= 8){
        return -1;
    }
    int i, j;
    FILEDISK* FileDisk = (FILEDISK* )malloc(sizeof(FILEDISK));
    FileDisk->block_int[0][0] = (UINT16)DiskID | (UINT16)0x005A;
    FileDisk->block_int[0][1] = (UINT16)0x0800;
    FileDisk->block_int[0][2] = (UINT16)0x8004;
    FileDisk->block_int[0][3] = (UINT16)0x0001;
    FileDisk->block_int[0][4] = (UINT16)0x0011;
    FileDisk->block_int[0][5] = (UINT16)0x0600;
    FileDisk->block_int[0][6] = (UINT16)0x0000;
    FileDisk->block_int[0][7] = (UINT16)0x2E00;
    FileDisk->block_int[1][0] = (UINT16)0xFFFF;
    FileDisk->block_int[1][1] = (UINT16)0xE0FF;
    for (j = 2; j < 8; j++){
        FileDisk->block_int[1][j] = (UINT16)0x0000;
    }
    for (i = 2; i < 13; i++){
        for (j = 0; j < 8; j++){
            FileDisk->block_int[i][j] = (UINT16)0x0000;
        }
    }
    for (i = 13; i < 17; i++){
        for (j = 0; j < 8; j++){
            FileDisk->block_int[i][j] = (UINT16)0xFFFF;
        }
    }
    /*for (i = 2; i < 13; i++){
        memset(FileDisk->block_int[i], 0, sizeof(int));
    }
    for (; i < 17; i++){
        memset(FileDisk->block_int[i], -1, sizeof(int));
    }*/
    FileDisk->block_int[17][0] = (UINT16)0x7200;
    FileDisk->block_int[17][1] = (UINT16)0x6F6F;
    FileDisk->block_int[17][2] = (UINT16)0x0074;
    FileDisk->block_int[17][3] = (UINT16)0x0000;
    FileDisk->block_int[17][4] = (UINT16)0x09EE;
    FileDisk->block_int[17][5] = (UINT16)0xFD00;
    FileDisk->block_int[17][6] = (UINT16)0x0012;
    FileDisk->block_int[17][7] = (UINT16)0x0000;
    UINT16 temp = 0x0013;
    for (j = 0; j < 8; j++){
        FileDisk->block_int[18][j] = temp;
        temp += (UINT16)0x1;
    }
    for (i = 0; i < 19; i++){
        //PHYSICAL_DISK_WRITE(DiskID, i, (char*)(FileDisk->block_char[i]));
        diskWritten(DiskID, i, (char*)(FileDisk->block_char[i]));
    }
    return 0;
}

void diskWritten(int diskID, int sector, char* str)
{
    MEMORY_MAPPED_IO mmio;
    mmio.Mode = Z502Status;
    mmio.Field1 = diskID;
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);
    PCB* pcb_disk = (PCB* )malloc(sizeof(PCB));
    PCB* pcb_wready = (PCB* )malloc(sizeof(PCB));
    //printf("Disk Test 1: Time = %d ",  GetSampleTimeNow());
    if (mmio.Field2 != DEVICE_FREE){
        doOnelck(2);
        pcb_disk = QNextItemInfo(2);
        doOneUnlck(2);
        //removeProcess(2);
        addToQueue(5 + diskID, pcb_disk);
        //addToQueue(1, pcb_disk);
            //aprintf("what hell?\n");
        // Disk hasn't been used - should be free
        //doOnelck(10);  //critical variable ok
        while (processToRun() == (void*)-1){
            //CALL(DoASleep(1));
            CALL(DoASleep(0));
        }
        //ok = 0;
        //doOneUnlck(10);
        mmio.Mode = Z502Status;
        mmio.Field1 = diskID;
        mmio.Field2 = mmio.Field3 = 0;
        MEM_READ(Z502Disk, &mmio);
        if (mmio.Field2 != DEVICE_FREE){
            //aprintf("what hell?\n");
            pcb_disk = removeProcess(2);
            //pcb_disk->diskID = SystemCallData->Argument[0];
            //addToQueue(5, pcb_disk);
            pcb_wready = processToRun();
            addToQueue(2, pcb_wready);
            removeProcess(1);
            mmio.Mode = Z502StartContext;
            mmio.Field1 = (long)(pcb_wready->NewContext);
            //mmio.Field2 = SUSPEND_CURRENT_CONTEXT_ONLY;
            mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
            //printf("asdasdaeeeeeeeeeee\n");
            //check_state("running", pcb_wready);
            //printf("asdasdaeeeeeeeeeee\n");
            MEM_WRITE(Z502Context, &mmio);
            //  aprintf("%dfffff\n",mmio.Field4);
            if (mmio.Field4 != ERR_SUCCESS){
                printf("Start Context has an error.\n");
                exit(0);
            }
        }
        else{
            /*pcb_wready = processToRun();
            if (pcb_wready->ProcessID == pcb_disk->ProcessID){
                removeProcess2(1, pcb_disk);
            }*/
            doOnelck(1);
            pcb_wready = QItemExists(1, pcb_disk);
            doOneUnlck(1);
            if (pcb_wready != (void*)-1){
                removeProcess2(1, pcb_disk);
            }
            else{
                //aprintf("ARE YOU OK?\n");
                //check_state("ttt", pcb_disk);
                removeProcess2(5 + diskID, pcb_disk);
            }
        }
    }
    // Start the disk by writing a block of data


    mmio.Mode = Z502DiskWrite;
    mmio.Field1 = diskID;
    mmio.Field2 = sector;
    mmio.Field3 = (long)str;
    MEM_WRITE(Z502Disk, &mmio);


  /*  mmio.Mode = Z502Status;
    mmio.Field1 = SystemCallData->Argument[0];
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);
    if (mmio.Field2 != DEVICE_IN_USE) {       // Disk should report being used
        printf( "Got erroneous result for Disk Status - Device is free.\n");
    }*/

    //PCB* pcb_disk = (PCB* )malloc(sizeof(PCB));
    //PCB* pcb_wready = (PCB* )malloc(sizeof(PCB));

    mmio.Mode = Z502Status;
    mmio.Field1 = diskID;
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);
    //printf("Disk Test 1: Time = %d ",  GetSampleTimeNow());
    if (mmio.Field2 != DEVICE_FREE){
        doOnelck(2);
        pcb_disk = QNextItemInfo(2);
        doOneUnlck(2);
        //removeProcess(2);
        addToQueue(5 + diskID, pcb_disk);
            //aprintf("what hell?\n");
        // Disk hasn't been used - should be free
        //doOnelck(10);  //critical variable ok
        while (processToRun() == (void*)-1){
            //CALL(DoASleep(1));
            CALL(DoASleep(0));
        }
        //ok = 0;
        //doOneUnlck(10);
        mmio.Mode = Z502Status;
        mmio.Field1 = diskID;
        mmio.Field2 = mmio.Field3 = 0;
        MEM_READ(Z502Disk, &mmio);
        if (mmio.Field2 != DEVICE_FREE){
            //aprintf("what hell?\n");
            pcb_disk = removeProcess(2);
            //pcb_disk->diskID = SystemCallData->Argument[0];
            //addToQueue(5, pcb_disk);
            pcb_wready = processToRun();
            addToQueue(2, pcb_wready);
            removeProcess(1);
            mmio.Mode = Z502StartContext;
            mmio.Field1 = (long)(pcb_wready->NewContext);
            //mmio.Field2 = SUSPEND_CURRENT_CONTEXT_ONLY;
            mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
            //printf("asdasdaeeeeeeeeeee\n");
            //check_state("running", pcb_wready);
            //printf("asdasdaeeeeeeeeeee\n");
            MEM_WRITE(Z502Context, &mmio);
            //  aprintf("%dfffff\n",mmio.Field4);
            if (mmio.Field4 != ERR_SUCCESS){
                printf("Start Context has an error.\n");
                exit(0);
            }
        }
        else{
            doOnelck(1);
            pcb_wready = QItemExists(1, pcb_disk);
            doOneUnlck(1);
            if (pcb_wready != (void*)-1){
                removeProcess2(1, pcb_disk);
            }
            else{
                //check_state("wrong", pcb_disk);
                removeProcess2(5 + diskID, pcb_disk);
            }
        }
    }
}
