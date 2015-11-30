// test

#include <stdint.h>

// 2MB flash
#define COFFEE_SIZE 2097152UL

// A larger page size makes the overhead smaller but logs larger.
#define COFFEE_PAGE_SIZE 256

// The erase sector size of the flash used.
#define COFFEE_SECTOR_SIZE 65536UL

// 16 should be sufficient, it's like foobarbazcux.txt
#define COFFEE_NAME_LENGTH 16

// This does only affect memory usage and can be changed later without
// formatting the filesystem
#define COFFEE_MAX_OPEN_FILES 64

// This has to be researched further
#define COFFEE_FD_SET_SIZE 64

// Minimum number of bytes to reserve for a new file.
#define COFFEE_DYN_SIZE 1024

// This has to be researched further
#define COFFEE_LOG_TABLE_LIMIT 1024

// The amount of data which can be stored in the log entries before data is written back to the files. Larger values gives fewer merges. This value should be a multiplum of the PAGE_SIZE
#define COFFEE_LOG_SIZE 1024

// is this ok?
typedef int coffee_page_t;

#define COFFEE_WRITE my_write
#define COFFEE_READ my_read
#define COFFEE_ERASE my_erase
