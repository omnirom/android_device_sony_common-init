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

#ifndef __UNCOMPRESS_H__
#define __UNCOMPRESS_H__

#define MEMORY_BUFFER_SIZE (const size_t)40*1024*1024 // Max size of uncompressed
                                                   // ramdisk (40 MB)

typedef char* byte_p;

size_t uncompress_gzip_memory(const byte_p compressed_data,
		const size_t compressed_data_size, const byte_p uncompressed_data,
		const size_t uncompressed_max_size);

size_t uncompress_memory(byte_p uncompressed_buffer, byte_p buffer, unsigned long file_size);

#endif // __UNCOMPRESS_H__
