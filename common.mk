LD=gcc
BAREMETAL_FLAGS=-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pie -static
CC=gcc -std=gnu11 $(BAREMETAL_FLAGS) -Wall -Wextra -MMD -c

COMMON_DIR=../common
COMMON_C=$(shell find $(COMMON_DIR) -iname "*.c")
COMMON_OBJ=$(COMMON_C:.c=.o)
