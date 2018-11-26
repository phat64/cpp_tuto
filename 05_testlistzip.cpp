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
	int i;
 	struct ZipEndRecord * endRecord;

	// search from the end
	for(i = size - sizeof(struct ZipEndRecord); i >= 0; i--) {
		endRecord = (struct ZipEndRecord *)(((uint8_t*)buffer) + i);
		if(endRecord->signature == 0x06054B50)
			return endRecord;
	}

	return NULL;
}

void ShowZipFileListFromBuffer(void * buffer, int size, struct ZipEndRecord* endRecord)
{
	uint16_t i;
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

	printf("numEntries = %d \n", endRecord->numEntries);
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

bool ReadDataFromFile(FILE * f, void* buffer, int n)
{
	int idx;
	int ret;
	int nbTries;
	uint8_t* data = (uint8_t*)buffer;

	idx = 0;
	nbTries = 99;
	while (n > 0)
	{
		ret = fread((void*)&data[idx], 1, n, f);
		if (ret <= 0)
		{
			if (--nbTries == 0)
			{
				break;
			}
		}
		else
		{
			n -= ret;
			idx += ret;
		}
	}

	return n == 0;
}

void ShowZipFileListFromFile(FILE * f)
{
	int i;
	char filename[FILENAME_MAX_LEN + 1];
	struct ZipGlobalFileHeader current;
	struct ZipEndRecord endRecord;
	int size;

	// get size
	fseek(f, 0 ,SEEK_END);
	size = ftell(f);
	fseek(f, 0 ,SEEK_SET);


	i = size - sizeof(struct ZipEndRecord) + 1;
	fseek(f , i, SEEK_SET);

	while(i >= 0)
	{
		fseek(f, -1, SEEK_CUR);
		if (!ReadDataFromFile(f, &endRecord, sizeof(struct ZipEndRecord)))
		{
			break;
		}
		if(endRecord.signature == 0x06054B50)
		{
			break;
		}
		i--;
	}

	// simple check
	if (endRecord.signature != 0x06054B50)
	{
		printf("[ShowZipFileList] end record not found\n");
		return;
	}

	printf("numEntries = %d\n", endRecord.numEntries);

	fseek(f, endRecord.centralDirectoryOffset, SEEK_SET);
	for (i = 0; i < endRecord.numEntries; i++)
	{
		int filenamelen;
		int percent;

		if (!ReadDataFromFile(f, &current, sizeof(struct ZipGlobalFileHeader)))
		{
			printf("[ShowZipFileList] read data error in ZipGlobalFileHeader\n");
			return;
		}

		// simple check
		if (current.signature != 0x02014B50)
		{
			printf("[ShowZipFileList] error signature ZipGlobalFileHeader\n");
			return;
		}

		filenamelen = MIN(current.fileNameLength, FILENAME_MAX_LEN);
		if (!ReadDataFromFile(f, filename, filenamelen))
		{
			printf("[ShowZipFileList] read data error in ZipGlobalFileHeader\n");
			return;
		}
		filename[filenamelen] = '\0';

		percent = current.uncompressedSize == 0? 100 : 100 * current.compressedSize / current.uncompressedSize;

		printf("%s, %d bytes / %d bytes (%d %%)\n", filename, current.compressedSize, current.uncompressedSize, percent);

		fseek(f, current.fileNameLength - filenamelen + current.extraFieldLength, SEEK_CUR);
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

	data = NULL;
	f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("[LoadFILE]  failed to open file %s\n", filename);
		goto err;
	}

	// get size
	fseek(f, 0 ,SEEK_END);
	n = ftell(f);
	if (size) *size = n;
	fseek(f, 0 ,SEEK_SET);

	data = (uint8_t*)malloc(n);
	if (data == NULL)
	{
		goto err;
	}

	if (!ReadDataFromFile(f, data, n))
	{
		printf("[LoadFILE]  failed to alloc for file %s\n", filename);
		goto err;
	}

	return (void*)data;
err:
	if (data != NULL)
	{
		free((void*)data);
		data = NULL;
	}

	if (f != NULL)
	{
		fclose(f);
	}
	return NULL;
}

int main(int argc, char ** argv)
{
	int size = 0;
	const char * filename;
	void* zip;

	if (argc != 2)
	{
		printf("error need 1 param\n");
		printf("usage : ./test file.zip\n");
		return -1;
	}

	filename = argv[1];
	printf("list files in %s\n", filename);


	printf("show zip list from a buffer\n");
	zip = LoadFILE(filename, &size);
	if (zip == NULL)
	{
		printf("open or load zip file failed\n");
		return -2;
	}

	printf("file size %d\n", size);
	ShowZipFileListFromBuffer(zip, size, NULL);
	free((void*)zip);

	printf("show zip list from a file\n");
	FILE* f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("open zip file failed\n");
		return -3;
	}
	ShowZipFileListFromFile(f);
	fclose(f);

	return 0;
}

/*
> 05_testlistzip test.zip
list files in test.zip
show zip list from a buffer
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
show zip list from a file
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
