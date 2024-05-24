#ifndef _FRAME_H_
#define _FRAME_H_

#include "common/types.h"

// Converts a 32-bit address into a frame number
#define ADDR_TO_FRAME_NB(addr) (((uint32_t)addr) >> 12)

// Converts a frame number into a 32-bit address
#define FRAME_NB_TO_ADDR(n) (((uint32_t)n) << 12)

#define FRAME_SIZE  4096

// Returns the number of frames required to store the given number of bytes
#define FRAME_COUNT(size) ((size + FRAME_SIZE - 1)/FRAME_SIZE)

// Initializes the physical frame subsystem, using the specified amount of physical memory.
extern void frame_init(uint_t RAM_in_KB);

// Allocates a frame (4KB) and returns its physical address.
// Returns the physical address of the frame or
// 0xFFFFFFFF if no more frames are available.
//
// REMARKS:
// The physical address is always aligned to a 4KB boundary.
// The frame's content is always zeroed.
extern void *frame_alloc();

// Free a frame.
// REMARK: doesn't check whether the frame was previously allocated or not.
extern void frame_free(void *frame_addr);

// Returns the total number of free frames.
// This is typically used by a syscall to retrieve the amount of free RAM.
extern uint_t frame_total_free();

// Allocates a number of contiguous frames (4KB each) and returns its physical address.
// Returns the physical address of the frame or
// 0xFFFFFFFF if no more frames are available.
//
// REMARKS:
// The physical address is always aligned to a 4KB boundary.
// The frames' content is always zeroed.
extern void *frames_alloc(int count);

 #endif