/*
 * Copyright (C) 2018 The OmniRom Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gelf.h"
#include "bootimg.h"
#include "zlib.h"

#include "uncompress.h"

#ifdef USE_LZMA
#include <LzmaLib.h>
#endif

#define MAX_DEBUG_HEX_OUTPUT 16
#define LZMA_DATA_OFFSET 13

#define DEBUG true

// memcmp needs buffer, no way to use preprocessor var
char gzip_header[] = { 0x1f, 0x8B };
char lzma_header[] = { 0x5d, 0x00, 0x00, 0x80, 0x00, 0xff };

size_t uncompress_gzip_memory(const byte_p compressed_data,
    const size_t compressed_data_size, const byte_p uncompressed_data,
    const size_t uncompressed_max_size) {
    z_stream zInfo = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    zInfo.avail_in = compressed_data_size;
    zInfo.total_in = compressed_data_size;
    zInfo.avail_out = uncompressed_max_size;
    zInfo.total_out = uncompressed_max_size;
    zInfo.next_in = (unsigned char *)compressed_data;
    zInfo.next_out = (unsigned char *)uncompressed_data;
    size_t return_value = 0;
    unsigned long err = inflateInit2(&zInfo, 16 + MAX_WBITS); // zlib function

    if (err == Z_OK) {
        err = inflate( &zInfo, Z_FINISH); // zlib function
        if (err == Z_STREAM_END) {
            return_value = zInfo.total_out;
        } else {
            printf("gunzip error -- Err:inflate %lu\n", err);
        }
    } else {
        printf("gunzip error -- Err:inflateInit2 %lu\n", err);
    }
    inflateEnd(&zInfo);
    return(return_value);
}

#ifdef USE_LZMA
size_t uncompress_lzma_memory(byte_p compressed_data,
	size_t compressed_data_size, byte_p uncompressed_data,
	size_t uncompressed_max_size) {

	Byte * outBuf;
	Byte * inBuf; 
	Byte * prop;

	unsigned propSize = 5;

	outBuf = (Byte *) malloc(uncompressed_max_size - LZMA_DATA_OFFSET);
	inBuf = (Byte *) malloc(compressed_data_size);

	prop = (Byte *) malloc(propSize);

	memcpy((Byte *) inBuf, (unsigned char *) compressed_data + LZMA_DATA_OFFSET, compressed_data_size - LZMA_DATA_OFFSET);
	memcpy(prop, compressed_data, propSize);

#if DEBUG
	printf("BEFORE: compressed size: %i / %i\n", compressed_data_size, sizeof(inBuf));
	printf("BEFORE: uncompressed size: %i / %i\n", uncompressed_max_size, sizeof(outBuf));
#endif //DEBUG

	/*
	* LzmaUncompress will return: 
	*	uncompressed datas into arg1, 
	*	uncompressed data size inro arg2. 
	*/
	int ret = LzmaUncompress(outBuf, 
		&uncompressed_max_size, 
		inBuf,
		&compressed_data_size,
		prop,
		propSize);

#if DEBUG
	printf("LzmaUncompress returned: %i\n", ret); 
	printf("AFTER: compressed size: %i / %i\n", compressed_data_size, sizeof(inBuf));
	printf("AFTER: uncompressed size: %i / %i\n", uncompressed_max_size, sizeof(outBuf));

	printf("inbuf: "); 
	int i=0; 
	for (i=0; i<MAX_DEBUG_HEX_OUTPUT ; i++) {
		printf(" 0x%x", inBuf[i]);
	}
	printf("). \n");

	printf("outbuf: "); 
	for (i=0; i<MAX_DEBUG_HEX_OUTPUT ; i++) {
		printf(" 0x%x", outBuf[i]);
	}
	printf("). \n");
#endif //DEBUG

	memcpy((unsigned char *) uncompressed_data, (Byte *) outBuf, uncompressed_max_size);

#if DEBUG
	printf("uncompressed_data: "); 
	for (i=0; i<MAX_DEBUG_HEX_OUTPUT ; i++) {
		printf(" 0x%x", uncompressed_data[i]);
	}
	printf("). \n");
#endif //DEBUG

	free(inBuf); 
	free(outBuf);

	return uncompressed_max_size; // uncompressed_max_size is filled by LzmaUncompress with the size of uncompressed data. Return it for the end of the process

}
#endif //USE_LZMA

size_t uncompress_memory(byte_p uncompressed_buffer, byte_p buffer, unsigned long file_size) {
    if (uncompressed_buffer == NULL) {
        free(buffer);
        printf("Unable to malloc memory for gunzip.\nFailed\n");
        return -1; 
    }   
    size_t uncompressed_size = 0;

    if (!memcmp(gzip_header, buffer, sizeof(gzip_header))) {
        printf("GZIP ramdisk found\n");
		uncompressed_size = uncompress_gzip_memory(buffer, file_size,
	        uncompressed_buffer, MEMORY_BUFFER_SIZE);
#ifdef USE_LZMA
    } else if (!memcmp(lzma_header, buffer, sizeof(lzma_header))) {
        printf("LZMA ramdisk found\n");
		uncompressed_size = uncompress_lzma_memory(buffer, file_size,
			uncompressed_buffer, MEMORY_BUFFER_SIZE);
#endif //USE_LZMA
    } else {
		char header[MAX_DEBUG_HEX_OUTPUT];
		memcpy(header, buffer, sizeof((char*)16));
        printf("Unrecognized ramdisk compression (");
		int i=0; 
		for (i=0; i<MAX_DEBUG_HEX_OUTPUT ; i++) {
			printf(" 0x%x", header[i]);
		}
		free(buffer);
		printf("). Giving up! \n");
		return -2;
    }

    free(buffer);
    if (uncompressed_size <= 0) {
        free(uncompressed_buffer);
        printf("Failed to deflate\n");
        return -2;
    }   
    printf("Original size: %lu, deflated size: %zu\n", file_size,
        uncompressed_size);

		int i=0;
		printf("buffer: "); 
		for (i=0; i<MAX_DEBUG_HEX_OUTPUT ; i++) {
			printf(" 0x%x", buffer[i]);
		}
		printf("). \n");

    return(uncompressed_size);
}

