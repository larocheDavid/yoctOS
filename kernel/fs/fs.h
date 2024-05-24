#ifndef _FS_H_
#define _FS_H_

#include "common/types.h"
#include "common/file.h"
#include "common/fs/fs.h"
#include "common/fs/superblock.h"
#include "drivers/ide.h"
#include "drivers/term.h"
#include "common/fs/inode.h"
#include "kernel/mem/frame.h"
#include "common/mem.h"

#define MAX_OPEN_FILES 16

typedef struct {
	int offset;  
	inode_t *inode;
} fd_t;

static fd_t fd_table[MAX_OPEN_FILES];
static superblock_t *sb;
static bitmap_t inode_bitmap;
static bitmap_t datablock_bitmap;
static inode_table_t inode_table;

extern void fs_init();
extern int file_stat(const char *filename, stat_t *stat);
extern int file_open(const char *filename);
extern int file_read(int fd, void *buf, uint_t count);
extern int file_seek(int fd, uint_t offset);
extern void file_close(int fd);

extern file_iterator_t file_iterator();
extern bool file_has_next(file_iterator_t *it);
extern void file_next(const char *filename, file_iterator_t *it);

#endif
