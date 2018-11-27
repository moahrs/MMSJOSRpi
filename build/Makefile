# Don't use normal gcc, use the arm cross compiler
PREFIX = arm-none-eabi-
CC = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy

CIRCLEHOME = ../lib/CircleLib

CFLAGS= -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fpic -ffreestanding -fno-exceptions
CSRCFLAGS= -O2 -Wall -Wextra -fno-rtti
LFLAGS= -ffreestanding -O2 -nostdlib -LD:\Projetos\gcc-arm-none-eabi-7-2018-q2-update\lib\gcc\arm-none-eabi\7.3.1\thumb -lsupc++
#LIBS= ../lib/usb/libuspienv.a ../lib/usb/libuspi.a
#LIBS= ../lib/usb/libcsud.a
LIBS= $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	  $(CIRCLEHOME)/lib/bt/libbluetooth.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/libcircle.a
#LIBS= $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/libcircle.a

# Location of the files
KER_SRC = ../src/kernel
KER_HEAD = ../include
CIRCLE_HEAD = $(CIRCLEHOME)/include
COMMON_SRC = ../src/common
DRIVERS_SRC = ../src/drivers
DISK_SRC = ../src/disk
OBJ_DIR = objects
KERSOURCES = $(wildcard $(KER_SRC)/*.cpp)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.cpp)
DRIVERSSOURCES = $(wildcard $(DRIVERS_SRC)/*.cpp)
DISKSOURCES = $(wildcard $(DISK_SRC)/*.cpp)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
OBJECTS = $(CIRCLEHOME)/lib/startup.o
OBJECTS += $(OBJ_DIR)/main.o $(OBJ_DIR)/kernel.o $(OBJ_DIR)/mmsjos.o
OBJECTS += $(patsubst $(COMMON_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(DRIVERS_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(DRIVERSSOURCES))
OBJECTS += $(patsubst $(DISK_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(DISKSOURCES))
HEADERS = $(wildcard $(KER_HEAD)/*.h)

IMG_NAME=mmsjos

build-hardware: build
	$(PREFIX)objdump -D $(IMG_NAME).elf > $(IMG_NAME).lst
	$(OBJCOPY) $(IMG_NAME).elf -O binary kernel.img

build: $(OBJECTS) $(HEADERS)
#	echo $(OBJECTS)''
	$(CC) -T $(CIRCLEHOME)/circle.ld -o $(IMG_NAME).elf $(LFLAGS) $(OBJECTS) $(LIBS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.cpp
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -I$(CIRCLE_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	$(CC) $(CFLAGS) -I$(KER_SRC) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.cpp
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -I$(CIRCLE_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(DRIVERS_SRC)/%.cpp
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -I$(CIRCLE_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(DISK_SRC)/%.cpp
	$(CC) $(CFLAGS) -I$(DISK_SRC) -I$(KER_HEAD) -I$(CIRCLE_HEAD) -c $< -o $@ $(CSRCFLAGS)

clean:
	rm -rf $(OBJ_DIR)
	rm $(IMG_NAME).elf
	rm $(IMG_NAME).img

run: build
	