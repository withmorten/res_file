#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

typedef uint8 byte;

typedef uint8 bool8;
typedef int32 bool32;

typedef uintptr_t uintptr;
typedef ptrdiff_t ptrdiff;

typedef int32 rge_handle;
typedef int32 rge_res_id;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define INVALID_FILE_HANDLE -1

#define DEFAULT_READ_FLAGS _O_BINARY
#define DEFAULT_WRITE_FLAGS _O_WRONLY | _O_APPEND | _O_CREAT | _O_TRUNC | _O_BINARY
#define DEFAULT_WRITE_PMODE _S_IREAD | _S_IWRITE

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define ever ;;

#define local static

#define once(code) do { { static bool done = false; if (!done) { done = true; code; } } } while(0)

#define rge_fopen fopen
#define rge_fclose(stream) do { if (stream) { fclose(stream); stream = NULL; } } while(0)

#define rge_malloc malloc
#define rge_calloc calloc
#define rge_realloc realloc
#define rge_free(pblock) do { free(pblock); pblock = NULL; } while(0)

#define ZEROSTR { '\0' }
#define ZEROMEM { 0x00 }
#define ZEROPTR { NULL }
#define EMPTYSTR ""

#define strzero(str) memset(str, '\0', sizeof(str)) // only works on char arrays
#define memzero(mem, size) memset(mem, 0x00, size)

#define zero_array(array, count) do { for (size_t i = 0; i < (size_t)count; i++) { array[i] = NULL; } } while(0)

#define rge_new(type) new type
#define rge_new_array(type, count) new type[count]

#define rge_delete(object) do { if (object) { delete object; object = NULL; } } while(0)
#define rge_delete_array(array) do { if (array) { delete[] array; array = NULL; } } while(0)