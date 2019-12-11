#ifndef FILEDISK_H_INCLUDED
#define FILEDISK_H_INCLUDED

#include "global.h"
typedef union
{
    char block_char[20][32];
    UINT16 block_int[20][32 / sizeof(short)];
}FILEDISK;

int diskFormat(int DiskID);
void diskWritten(int diskID, int sector, char* str);
#endif // FILEDISK_H_INCLUDED
