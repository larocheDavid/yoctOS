#ifndef _TSS_H_
#define _TSS_H_

#include "common/types.h"

// Task-State Segment (TSS) structure.
typedef struct {
	uint16_t previous_task_link, reserved0;
	uint32_t esp0;
	uint16_t ss0, reserved1;
	uint32_t esp1;
	uint16_t ss1, reserved2;
	uint32_t esp2;
	uint16_t ss2, reserved3;
	uint32_t cr3;
	uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint16_t es, reserved4;
	uint16_t cs, reserved5;
	uint16_t ss, reserved6;
	uint16_t ds, reserved7;
	uint16_t fs, reserved8;
	uint16_t gs, reserved9;
	uint16_t ldt_selector, reserved10;
	uint16_t reserved11;
	uint16_t iomap_base_addr;  // adress (relative to byte 0 of the TSS) of the IO permission bitmap
} __attribute__ ((packed)) tss_t;

#endif
