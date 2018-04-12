
#define MEMORY_BUFFER_SIZE (const size_t)40*1024*1024 // Max size of uncompressed
                                                   // ramdisk (40 MB)

typedef char* byte_p;

size_t uncompress_gzip_memory(const byte_p compressed_data,
		const size_t compressed_data_size, const byte_p uncompressed_data,
		const size_t uncompressed_max_size);

size_t uncompress_memory(byte_p uncompressed_buffer, byte_p buffer, unsigned long file_size);

