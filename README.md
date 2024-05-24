# Advanced System Programming
**Professor**: Florent Gluck  
**Date**: December 12, 2022  

## Filesystem Project

### Objective

The objective of this practical work is to continue the development of your operating system by adding support for a rudimentary inode-based filesystem to the kernel. In user space, the system library will use the functionalities provided by the kernel to offer a simplified programming interface similar to the C library on a traditional UNIX system.

### Kernel Capabilities

The kernel must be capable of:
- Reading a file, entirely or partially.
- Moving the read pointer of an open file.
- Determining if a file exists.
- Obtaining file metadata.
- Iterating over files in the filesystem.
- Managing multiple open files simultaneously.

### User Space Library

In user space, the system library will offer the same functionalities as those provided by the kernel.

### Starting Point

You will start with a functional operating system to which you will add the required functionalities. The provided code can be found in the directory `labs/lab5-filesystem/skeleton/` on the course git repository.

### Specifications

To save time, the entire user space part is provided, as well as the implementation of system calls in the kernel. Your task is to implement the filesystem support in the kernel.

This support requires implementing system functions that allow the kernel to manipulate the filesystem: open, read, iterate over files, etc. Access to files follows UNIX semantics via functions like `open`, `read`, `close`, etc., which use file descriptors. The kernel must be able to handle up to 16 open files simultaneously. The same file can be opened multiple times, each time referenced by a different file descriptor.

The API you need to implement is clearly defined and independent of the filesystem structure on disk. The functions to be implemented in your kernel are detailed below:

- `void fs_init()`: Initializes the filesystem, typically by reading the essential filesystem data structures (superblock, bitmaps, etc.) and initializing the necessary data structures.
- `int file_stat(const char *filename, stat_t *stat)`: Returns in `stat` the metadata related to the file passed as an argument. The function returns 0 on success and -1 on failure. The `stat_t` structure is defined in `common/file.h` and contains the file size and inode number.
- `int file_open(const char *filename)`: Opens a file and returns a file descriptor for access, or -1 on failure.
- `int file_read(int fd, void *buf, uint_t count)`: Reads `count` bytes from the file referenced by `fd` and places them in the buffer `buf`. Returns the number of bytes read, or 0 on end of file, or -1 on error.
- `int file_seek(int fd, uint_t offset)`: Positions the read pointer of the open file referenced by the descriptor `fd` to `offset` from the beginning of the file. Returns the new position or -1 on failure, typically if the offset is invalid.
- `void file_close(int fd)`: Closes the file referenced by the descriptor `fd`.
- `file_iterator_t file_iterator()`: Creates an iterator to iterate over the files in the filesystem.
- `bool file_has_next(file_iterator_t *it)`: Returns true if there is at least one file to iterate over.
- `void file_next(char *filename, file_iterator_t *it)`: Iterates to the next file and copies the name of the next file pointed to by the iterator into `filename`.

These functions are to be implemented in the source file `kernel/fs/fs.c`. Note that they are referenced by the kernel system call code, so it is imperative that you strictly adhere to the given prototypes!

### File Descriptors

The kernel must maintain a table of open file descriptors. A task (or the kernel) must be able to open multiple files simultaneously, or the same file multiple times, each time obtaining a different file descriptor. Thus, each file descriptor is associated with a position in the byte stream of each open file.

### Filesystem Format

The implemented filesystem is based on indexed block allocation with direct and indirect pointers. Each file is associated with an inode containing its metadata. An inode bitmap is used to determine which inodes are free or allocated. Similarly, a bitmap is used to determine which data blocks are free or allocated. For simplicity, there is no notion of directories; file names are stored directly in inodes. Thus, the filesystem is flat, with no hierarchical structure, and does not support hard links. To obtain the list of available files, simply iterate over the inodes. Any inode with a number greater than 0 indicates the presence of a file.

The structure of the superblock is as follows:

```c
#define FS_SIGNATURE_LENGTH 4
#define FS_SIGNATURE "BFS"
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define SUPERBLOCK_LABEL_LENGTH 32

typedef struct __attribute__((__packed__)) {
    int8_t signature[FS_SIGNATURE_LENGTH];
    uint8_t version_major;
    uint8_t version_minor;
    int8_t label[SUPERBLOCK_LABEL_LENGTH];
    uint32_t block_size;
    uint32_t inode_bitmap_start;
    uint32_t inode_bitmap_block_count;
    uint32_t datablock_bitmap_start;
    uint32_t datablock_bitmap_block_count;
    uint32_t inode_start;
    uint32_t inode_count;
    uint32_t inode_block_count;
    uint32_t datablocks_start;
    uint32_t datablock_count;
} superblock_t;
```

### Tools for Filesystem Image Management

Utilities are provided to manipulate the filesystem image used by the operating system. Once the filesystem image is created, it is passed to QEMU, which presents it to the virtual machine where the operating system runs. The OS then programs the disk as if it were a physical disk containing the previously created filesystem.

### Provided Utilities

The tools and their complete source codes are located in the `tools` directory. These tools allow you to:

- Create an empty filesystem image.
- Display information about a filesystem image.
- Insert a host system file into a filesystem image.
- List the files present in a filesystem image.
- Read the contents of a file from a filesystem image.
- Delete a file from a filesystem image.

### Exact List of Programs

- `fs_create`: Creates the image.
- `fs_info`: Displays information related to an image.
- `fs_add`: Adds a file to an image.
- `fs_list`: Lists the files present in the image.
- `fs_cat`: Displays the contents of a file from the image to standard output.
- `fs_read`: Displays a certain number of characters from a file at a given offset to standard output.
- `fs_del`: Deletes a file from an image.

### Tests and Validation

To validate your filesystem support code, the kernel automatically runs a user space test program. This program exhaustively uses the kernel's functionalities (via system calls) to list and read files in different ways.

Navigate to the `lab5-filesystem/skeleton/yoctos/` directory and execute `make run` to launch the test program, which includes 7 different tests. An ISO image with reference test results is available at the root of `lab5` in the `yoctos.xz` file. You should obtain exactly the same output with your implementation. Note that the test program, `testfs.exe`, can only be executed correctly once file reading is implemented!

### Important Notes

**New Provided Function**:  
A new function in the `frame` module is provided:  
`void *frames_alloc(int count);`  
This function is identical to `frame_alloc` but allows dynamic allocation of the specified number of frames. The frames are guaranteed to be allocated contiguously in the kernel's address space (i.e., in physical memory).

**Serialization**:  
Be careful with the serialization of structures: by default, they may be padded (fields aligned to word boundaries) by the compiler for performance reasons. To ensure the compiler does not add any padding, use the `__attribute__((packed))` attribute for all structures read or written to disk (i.e., serialized).

### Deadline

This practical work is to be completed by Friday, January 20, 2023.
