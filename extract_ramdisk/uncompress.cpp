/*
 *  Copyright (C) 2018 The OmniROM Project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gelf.h"
#include "bootimg.h"
#include "zlib.h"
#include <LzmaLib.h>

#include "uncompress.h"

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

size_t uncompress_lzma_memory(byte_p compressed_data,
	size_t compressed_data_size, byte_p uncompressed_data,
	size_t uncompressed_max_size) {
	unsigned int propSize = 5;
	Byte *prop = (Byte *)compressed_data;
	Byte *compressed_data_start = (Byte *) compressed_data + LZMA_DATA_OFFSET;

	int ret = LzmaUncompress((Byte *)uncompressed_data,
		&uncompressed_max_size,
		compressed_data_start,
		&compressed_data_size,
		prop,
		propSize);

	if (ret != SZ_OK) {
        printf("Lzma uncompress error. Errno: 0x%x.\n", ret);
		return -1;
    }

    // uncompressed_max_size is now the actual size of the uncompressed data.
	return uncompressed_max_size;
}

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
	} else if (!memcmp(lzma_header, buffer, sizeof(lzma_header))) {
		printf("LZMA ramdisk found\n");
		uncompressed_size = uncompress_lzma_memory(buffer, file_size,
			uncompressed_buffer, MEMORY_BUFFER_SIZE);
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

	return(uncompressed_size);
}

