PROJECT=anykey-hidraw-test	
PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

IDIR = -I $(PROJECT_ROOT)../../software/inc/ -I $(PROJECT_ROOT)../../software/.3rdparty/u8g2/csrc 
CC=gcc

CFLAGS +=$(IDIR) -DHIDRAW_TEST
CFLAGS_BUILD = -O3
CFLAGS_DEBUG = -O0 -g -DDEBUG

ifeq ($(BUILD_MODE),debug)
	CFLAGS += $(CFLAGS_DEBUG)
else ifeq ($(BUILD_MODE),run)
	CFLAGS += $CFLAGS_BUILD)
else
	CFLAGS += $(CFLAGS_BUILD)
endif

OBJS = main.o

all:	$(PROJECT)

$(PROJECT):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr $(PROJECT) $(OBJS)