# Don't use normal gcc, use the arm cross compiler
PREFIX = arm-none-eabi-
CC = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy

SDKHOME = D:/Projetos/Raspberrypi/MMSJOS
CIRCLEHOME = $(SDKHOME)/lib/CircleLib
SCREETFTHOME = $(SDKHOME)/lib/ScreenTFTLib

CFLAGS= -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fpic -ffreestanding -fno-exceptions
CSRCFLAGS= -O2 -Wall -Wextra -fno-rtti
LFLAGS= -ffreestanding -O2 -nostdlib -LD:\Projetos\gcc-arm-none-eabi-7-2018-q2-update\lib\gcc\arm-none-eabi\7.3.1\thumb -lsupc++

# Location of the files
KER_SRC = ../src
MMSJ_HEAD = $(SDKHOME)/include
CIRCLE_HEAD = $(CIRCLEHOME)/include
CIRCLE_ADDON_HEAD = $(CIRCLEHOME)/addon
SCREENTFT_HEAD = $(SCREETFTHOME)/include
DRIVERS_HEAD = ../include
OBJ_DIR = objects
KERSOURCES = $(wildcard $(KER_SRC)/*.cpp)
OBJECTS = $(patsubst $(KER_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(KERSOURCES))
HEADERS = $(wildcard $(KER_HEAD)/*.h)

IMG_NAME=module_hm10

build-hardware: build

build: $(OBJECTS) $(HEADERS)
#	echo $(OBJECTS)''
	$(CC) -shared -o $(IMG_NAME).dll $(LFLAGS) $(OBJECTS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.cpp
	$(CC) $(CFLAGS) -I$(DRIVERS_HEAD) -I$(MMSJ_HEAD) -I$(CIRCLE_HEAD) -I$(CIRCLE_ADDON_HEAD) -I$(SCREENTFT_HEAD) -c $< -o $@ $(CSRCFLAGS)

clean:
	rm -rf $(OBJ_DIR)
	rm $(IMG_NAME).dll

run: build
	