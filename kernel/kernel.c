#include "common/string.h"
#include "common/stdio.h"
#include "common/mem.h"
#include "common/keycodes.h"
#include "boot/module.h"
#include "boot/multiboot.h"
#include "drivers/vbe.h"
#include "drivers/term.h"
#include "drivers/pic.h"
#include "drivers/timer.h"
#include "drivers/keyboard.h"
#include "interrupt/idt.h"
#include "mem/paging.h"
#include "mem/frame.h"
#include "mem/gdt.h"
#include "task/task.h"
#include "x86.h"
#include "drivers/ide.h"
#include "fs/fs.h"
#include "drivers/vbe.h"


// These are defined in the linker script: kernel.ld
extern void ld_kernel_start();
extern void ld_kernel_end();
uint_t kernel_start = (uint_t)&ld_kernel_start;
uint_t kernel_end = (uint_t)&ld_kernel_end;

void kernel_main(multiboot_info_t *mbi) {
    multiboot_set_info(mbi);
    uint_t RAM_in_KB = multiboot_get_RAM_in_KB();

    gdt_init();

    // This function must be initialized first! (before using any term_xxx functions!)
    vbe_init();
    vbe_fb_t *fb = vbe_get_fb();

    paging_init(RAM_in_KB);  // must be called AFTER vbe_init()!    

    term_init();
    term_printf("YoctOS started\n");
    term_printf("VBE mode %dx%d %dbpp initialized (addr=0x%x, pitch=%d).\n", fb->width, fb->height, fb->bpp, fb->addr, fb->pitch_in_bytes);
    term_printf("Detected %dKB of RAM.\n", RAM_in_KB);
    term_printf("%dKB of RAM available.\n", frame_total_free()*FRAME_SIZE/1024);
    term_printf("Kernel loaded at [0x%x-0x%x], size=%dKB\n", kernel_start, kernel_end, (kernel_end-kernel_start)/1024);

    modules_display_info();

    pic_init();
    idt_init();
    keyb_init();

    // IMPORTANT: timer frequency must be >= 50
    int timer_freq = 1000;
    timer_init(timer_freq);

    tasks_init();

	ide_init(MODE_HW, NULL);
	fs_init();

    // Unmask hardware interrupts
    sti();
    term_puts("Interrupts enabled.\n");
    int fd;
    char *filename = NULL;

    // //static void list_files() {
	// file_iterator_t it = file_iterator();

	// while (file_has_next(&it)) {
	// 	char name[256];
	// 	file_next(name, &it);
	// 	stat_t stat;
	// 	file_stat(name, &stat);
	// 	term_printf("%s (inode %d, %d bytes)\n", name, stat.inode, stat.size);
	// }

    // //cat(const char *filename) {
    // filename = "big";
    // fd = file_open(filename);
    // if (fd == -1) {
    //     term_printf("cat: failed reading \"%s\"\n", filename);
    //     return;
    // }
    // int sz = 37;
    // char buf[sz+1];
    // int n;
    // do {
    //     n = file_read(fd, buf, sz);
    //     buf[n] = 0;
    //     term_puts(buf);
            
    // } while (n);
    // file_close(fd);

    // //static void simult_reads_test() {
    // const int FD_COUNT = 3;
    // int fd3[FD_COUNT];
    // filename = "plan9.txt";

    // for (int i = 0; i < FD_COUNT; i++) {
    //     fd3[i] = file_open(filename);
    //     if (fd3[i] == -1) {
    //         term_printf("Failed reading \"%s\"\n", filename);
    //         return;
    //     }
    // }
    // uint8_t buf3[128];
    // // 1: The single most important feature of Plan 9
    // // 2: Plan 9 was integrating graphics and ubiquitous networking
    // // 3: Why didn't it take over the world?
    // int offsets[] = { 1191, 297, 2711 };
    // int counts[] = { 43, 57, 34 };
    // for (int i = 0; i < FD_COUNT; i++) {
    //     file_seek(fd3[i], offsets[i]);
    //     file_read(fd3[i], buf3, counts[i]);
    //     buf3[counts[i]] = 0;
    //     term_printf("%c%s%c\n", 179, buf3, 179);
    // }
    // for (int i = 0; i < FD_COUNT; i++) {
    //     file_close(fd3[i]);
    // }

    ///////////////////////////////
    //static void seek_read_test() {
    filename = "big";
	fd = file_open(filename);
	if (fd == -1) {
		term_printf("Failed reading \"%s\"\n", filename);
		return;
	}
    file_seek(fd, 877193);
    int count = 512;
    uint8_t buf2[count];
	file_read(fd, buf2, count);
	file_close(fd);

    for (int i = 0; i < count; i++) {
        term_printf("%d ", buf2[i]);
    }
    term_printf("\n");


    // //static void invalid_seek_test() {
    // filename = "big";
	// fd = file_open(filename);
	// if (fd == -1) {
	// 	term_printf("Failed reading \"%s\"\n", filename);
	// 	return;
	// }
    // int pos = file_seek(fd, 1024*1024*50);
	// file_close(fd);

    // term_printf("Position = %d (should be %d)\n", pos, -1);

    // // splash = 360 bytes
    // //static void invalid_read_test() {
    // filename = "splash";  // MUST use splash
	// fd = file_open(filename);
	// if (fd == -1) {
	// 	term_printf("Failed reading \"%s\"\n", filename);
	// 	return;
	// }
    // uint8_t buf4[1000];
    // buf4[360] = 137;
	// int bytes_read = file_read(fd, buf4, 1000);
	// file_close(fd);
    // term_printf("Bytes read = %d (should be %d)\n", bytes_read, 360);
    // term_printf("Guard value = %d (should be %d)\n", buf4[360], 137);
    
    // char *init = "testfs.exe";
    // if (!task_exec(init)) {
    //     term_printf("FAILED executing %s!", init);
    // }

    term_printf("\nSystem halted.");
    halt();
}
