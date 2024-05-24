#include "fs.h"

int bit_to_byte(uint_t size) {
    return (size + 8 -1) / 8;
}

void print_bloc(void* buf){
    for(uint_t i=0; i < sb->block_size; ++i)
        term_printf("%c", *(char*)(buf+i));

    term_printf("\n");
};

void read_block(int block_start, void *dst, int block_count) {

    for (int i=0; i < block_count*2; ++i) {
        ide_read_sector(block_start*2 + i, dst);
        dst += SECTOR_SIZE;
    }
}

void fs_init() {

    //Superblock
    sb = frame_alloc(); //allocate 4KB
    read_block(0, sb, 1);

    //Inode bitmapindirect_block
    inode_bitmap.count = sb->inode_bitmap_block_count;
    inode_bitmap.size = bit_to_byte(inode_bitmap.count*sb->block_size);
    inode_bitmap.bits = frame_alloc(FRAME_COUNT(inode_bitmap.size));
    read_block(sb->inode_bitmap_start, inode_bitmap.bits, sb->inode_bitmap_block_count);

    //Data block bitmap
    datablock_bitmap.count = sb->datablock_bitmap_block_count;
    datablock_bitmap.size = bit_to_byte(datablock_bitmap.count); 
    datablock_bitmap.bits = frame_alloc(FRAME_COUNT(datablock_bitmap.size));
    read_block(sb->datablock_bitmap_start, datablock_bitmap.bits, sb->datablock_bitmap_block_count);
    
    //Inode Table
    inode_table.inode_count = sb->inode_count;
    inode_table.size = sb->inode_block_count*64; 
    inode_table.inodes = frames_alloc(FRAME_COUNT(inode_table.size));
    read_block(sb->inode_start, inode_table.inodes, sb->inode_block_count);

    term_printf("datablock_count %d, inode_block_count %d", sb->datablock_count, sb->inode_block_count);
    for (int i=0; i < MAX_OPEN_FILES; i++) {
        fd_table[i].offset = -1;
        fd_table[i].inode = NULL;
    }
}

int file_stat(const char *filename, stat_t *stat) {
    
    inode_t *inode = inode_get_from_filename(&inode_table, filename);
    if (inode) {
        stat->inode = inode->number;
        stat->size = inode->size;
        return 0;
    }
    return -1;
}

int file_open(const char *filename) {
    
    inode_t *inode = inode_get_from_filename(&inode_table, filename);

    if (inode) {
        for (int fd = 0; fd < MAX_OPEN_FILES; ++fd) {
            if (fd_table[fd].offset == -1) {
                fd_table[fd].offset = 0;
                fd_table[fd].inode = inode;
                return fd;
            }
        }
    }
    return -1;
}

//return i^nth block addr from indirect pointer
uint16_t get_direct_ptr(uint16_t indirect_ptr, int i) {
    uint16_t indirect_block[sb->block_size/sizeof(uint16_t)];

    read_block(sb->datablocks_start + indirect_ptr, indirect_block, 1);
    return indirect_block[i];
}

int file_read(int fd, void *buf, uint_t count) {

    inode_t *inode = fd_table[fd].inode;

    if (fd_table[fd].offset >= inode->size)
        return 0;

    uint32_t block_curr = fd_table[fd].offset / sb->block_size;
    uint32_t block_cnt = (count + sb->block_size - 1) / sb->block_size;
    int total_blocks = inode->size/sb->block_size +1;
    uint8_t block[block_cnt][sb->block_size];
    //term_printf("block_curr %d\n", block_curr);

    uint32_t n = (block_curr - INODE_DIRECT_PTR_CNT) / (sb->block_size/sizeof(uint16_t));

    //("limit %d\n", inode->size/sb->block_size);
    int i;
    for(i=0; block_curr + i < INODE_DIRECT_PTR_CNT && i < block_cnt; i++) {
        read_block(sb->datablocks_start + inode->direct_ptr[block_curr], (void*)block[i], 1);
    }

    //term_printf("i%d\n", i);
    
    int k=0;
    bool read_count = 1;
    for(int j=0; j < INODE_INDIRECT_PTR_CNT && block_curr + block_cnt > INODE_INDIRECT_PTR_CNT && read_count; j++) {
        //term_printf("AA");
        uint16_t indirect_block[sb->block_size/sizeof(uint16_t)];
        read_block(sb->datablocks_start + inode->indirect_ptr[j], indirect_block, 1);

        for(k=0; k < sb->block_size/sizeof(uint16_t) && read_count; k++) {
            if (i+j*512+k>=block_cnt )
                read_count = 0;
            //term_printf("block_curr+i+j %d\n", block_curr+k+j);
            read_block(sb->datablocks_start + indirect_block[j], block[i+j*512+k], 1);
            //term_printf("i %d, j %d, k %d, block_curr %d\n", i, j, k, block_curr);
        }
    }

    void *src = (void*)block;
    uint32_t bloc_offset = (uint32_t) (fd_table[fd].offset - block_curr*sb->block_size);
    src += bloc_offset;

    int bytes_rem = fd_table[fd].offset + count - inode->size;

    fd_table[fd].offset += count;
    if (bytes_rem > 0) {
        //term_printf("HABULLAH");
        memcpy(buf, src, count-bytes_rem);
        return count-bytes_rem;
    }
    //term_printf("YOOO");
    memcpy(buf, src, count);
    return count;
}

int file_seek(int fd, uint_t offset) {

    inode_t *inode = fd_table[fd].inode;

    if (offset < inode->size) {
        fd_table[fd].offset = offset;
        //term_printf("offset %d\n", offset);
        return offset;
    }
    return -1;
}

void file_close(int fd) {
    fd_table[fd].offset = -1;
}

file_iterator_t file_iterator() { // initialise at 0, val = index inode courant
    file_iterator_t f;
    return f;
}

bool file_has_next(file_iterator_t *it) { 
    uint8_t byte;

    it->val+=1;
    int byte_n = it->val/8;
    int bit_n = it->val%8;

    for (uint32_t i=byte_n; i <= sb->inode_bitmap_block_count*sb->block_size; i++) {
        byte = inode_bitmap.bits[i];
        
        for (int n=bit_n; n < 8; n++) {
            
            if ((byte >> n) & 1) {
                inode_t *inode = inode_get_from_inode(&inode_table, it->val);

                if (inode)
                    return true;
            }
        }
    }
    return false;
}

void file_next(const char *filename, file_iterator_t *it) { //copie nom du fichier dans filename

    inode_t *inode = inode_get_from_inode(&inode_table, it->val);
    memcpy(filename, inode->name, INODE_FILENAME_LENGTH);
}
