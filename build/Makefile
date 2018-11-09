# Don't use normal gcc, use the arm cross compiler
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS= -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fpic -ffreestanding
CSRCFLAGS= -O2 -Wall -Wextra
LFLAGS= -ffreestanding -O2 -nostdlib -lgcc

# Location of the files
KER_SRC = ../src/kernel
KER_HEAD = ../include
COMMON_SRC = ../src/common
DRIVERS_SRC = ../src/drivers
OBJ_DIR = objects
KERSOURCES = $(wildcard $(KER_SRC)/*.c)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.c)
DRIVERSSOURCES = $(wildcard $(DRIVERS_SRC)/*.c)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
OBJECTS = $(patsubst $(KER_SRC)/%.S, $(OBJ_DIR)/%.o, $(ASMSOURCES))
OBJECTS += $(patsubst $(KER_SRC)/%.c, $(OBJ_DIR)/%.o, $(KERSOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.c, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(DRIVERS_SRC)/%.c, $(OBJ_DIR)/%.o, $(DRIVERSSOURCES))
HEADERS = $(wildcard $(KER_HEAD)/*.h)

IMG_NAME=mmsjos

build-hardware: build
	$(PREFIX)objdump -D $(IMG_NAME).elf > $(IMG_NAME).lst
	$(OBJCOPY) $(IMG_NAME).elf -O binary kernel.img

build: $(OBJECTS) $(HEADERS)
#	echo $(OBJECTS)
	$(CC) -T linker.ld -o $(IMG_NAME).elf $(LFLAGS) $(OBJECTS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.c
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	$(CC) $(CFLAGS) -I$(KER_SRC) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.c
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(DRIVERS_SRC)/%.c
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

clean:
	rm -rf $(OBJ_DIR)
	rm $(IMG_NAME).elf
	rm $(IMG_NAME).img

run: build
	