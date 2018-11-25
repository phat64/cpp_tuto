#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// This is the C Version of List files in a ZIP
// Of course, it's better to seek in a zip file than to alloc all the zip file
// It's interesting when you want to incorporate your zip file in your exe data

#define FILENAME_MAX_LEN 4096
#define MIN(a, b) ((a) <= (b) ? (a):(b))

#pragma pack(push, 1)
struct ZipGlobalFileHeader {
    uint32_t signature; // 0x02014B50
    uint16_t versionMadeBy;
    uint16_t versionNeededToExtract;
    uint16_t generalPurposeBitFlag;
    uint16_t compressionMethod;
    uint16_t lastModFileTime;
    uint16_t lastModFileDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength;
    uint16_t fileCommentLength;
    uint16_t diskNumberStart;
    uint16_t internalFileAttributes;
    uint32_t externalFileAttributes;
    uint32_t relativeOffsetOflocalHeader;
};

struct ZipEndRecord {
    uint32_t signature; // 0x06054b50
    uint16_t diskNumber;
    uint16_t centralDirectoryDiskNumber;
    uint16_t numEntriesThisDisk;
    uint16_t numEntries;
    uint32_t centralDirectorySize;
    uint32_t centralDirectoryOffset;
    uint16_t zipCommentLength;
};
#pragma pack(pop)


struct ZipEndRecord* FindZipEndRecord(void* buffer, int size)
{
 	struct ZipEndRecord * endRecord;

	// search from the end
	for(int i = size - sizeof(struct ZipEndRecord); i >= 0; i--) {
        	endRecord = (struct ZipEndRecord *)(((uint8_t*)buffer) + i);
        	if(endRecord->signature == 0x06054B50)
           	 	return endRecord;
	}

	return NULL;
}

void ShowZipFileList(void * buffer, int size, struct ZipEndRecord* endRecord)
{
	int16_t i;
	char filename[FILENAME_MAX_LEN + 1];
	struct ZipGlobalFileHeader* current;

	// simple check
	if (endRecord == NULL)
	{
		endRecord = FindZipEndRecord(buffer, size);
		if (endRecord == NULL)
		{
			printf("[ShowZipFileList] end record not found\n");
			return;
		}
	}

	current = (struct ZipGlobalFileHeader*) (((uint8_t*)buffer) + endRecord->centralDirectoryOffset);

	for (i = 0; i < endRecord->numEntries; i++)
	{
		int filenamelen;
		int percent;

		// simple check
		if (current->signature != 0x02014B50)
		{
			printf("[ShowZipFileList] error signature ZipGlobalFileHeader\n");
			return;
		}

		filenamelen = MIN(current->fileNameLength, FILENAME_MAX_LEN);
		memcpy(filename, (void*)&current[1], filenamelen); 
		filename[filenamelen] = '\0';
		
		percent = current->uncompressedSize == 0? 100 : 100 * current->compressedSize / current->uncompressedSize;

		printf("%s, %d bytes / %d bytes (%d %%)\n", filename, current->compressedSize, current->uncompressedSize, percent);

		// next
		current = (struct ZipGlobalFileHeader*)(((uint8_t*)&current[1]) + current->fileNameLength + current->extraFieldLength);
	}
}

void printZipInfo(void * buffer, int size)
{
	if (buffer != NULL)
	{
	 	struct ZipEndRecord * endRecord = FindZipEndRecord(buffer, size);
	  	if (endRecord == NULL)
		{
			printf("[printZipInfo] end record not found\n");
			return;
		}

		printf("numEntries = %d \n", endRecord->numEntries);

		ShowZipFileList(buffer, size, endRecord);
	}
	else
	{
		printf("[printZipInfo] buffer = NULL\n");
	}
}

void* LoadFILE(const char * filename, int * size)
{
	FILE* f;
	uint8_t* data;
	int n;
	int ret;
	int idx;
	int nbTry;	

	f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("[LoadFILE]  failed to open file %s\n", filename);
		return NULL;
	}

	// get size
	fseek(f, 0 ,SEEK_END);
	n = ftell(f);
	if (size) *size = n;
	fseek(f, 0 ,SEEK_SET);

	data = (uint8_t*)malloc(n);

	idx = 0;
	nbTry = 99;
	while (n > 0)
	{
		ret = fread(&data[idx], 1, n, f);
		if (ret <= 0)
		{
			nbTry--;
			if (nbTry < 0)
			{
				printf("[LoadFILE]  %s file cannot be readen\n", filename);
	
				free((void*)data);
				fclose(f);
				return NULL;
			}
		}
		else 
		{
			idx += ret;
			n -= ret;
		}
	}

	fclose(f);
	return (void*)data;

}

int main(int argc, char ** argv)
{
	int size = 0;
	const char * filename;
	void* zip;

	if (argc != 2)
	{
		printf("error need 1 param\nusage : ./test file.zip\n");
		return -1;
	}

	filename = argv[1];
	printf("list files in %s\n", filename);

	zip = LoadFILE(filename, &size);
	if (zip == NULL)
	{
		printf("open zip file failed\n");
		return -2;
	}

	printf("file size %d\n", size);
	printZipInfo(zip, size);

	free((void*)zip);
	return 0;
}

/*
> 05_testlistzip test.zip
list files in test.zip
file size 38908
numEntries = 11 
cpp_tuto/01_testsmartpointer.cpp, 598 bytes / 1774 bytes (33 %)
cpp_tuto/02_testbenchmarkduration.cpp, 602 bytes / 1400 bytes (43 %)
cpp_tuto/03_testexitcallback.cpp, 271 bytes / 605 bytes (44 %)
cpp_tuto/04_testmemorymanager.cpp, 1611 bytes / 5735 bytes (28 %)
cpp_tuto/05_testlistzip.cpp, 1475 bytes / 4205 bytes (35 %)
cpp_tuto/Makefile, 130 bytes / 323 bytes (40 %)
cpp_tuto/01_testsmartpointer, 11947 bytes / 42224 bytes (28 %)
cpp_tuto/02_testbenchmarkduration, 4900 bytes / 15240 bytes (32 %)
cpp_tuto/03_testexitcallback, 6362 bytes / 22208 bytes (28 %)
cpp_tuto/04_testmemorymanager, 5016 bytes / 14648 bytes (34 %)
cpp_tuto/05_testlistzip, 3928 bytes / 13480 bytes (29 %)
*/
