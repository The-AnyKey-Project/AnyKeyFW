##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

include env_vars
export $(shell sed 's/=.*//' env_vars)

# Set optimization level
ifeq ($(USE_DEBUG_BUILD),1)
  USE_DEBUG_BUILD = -DUSE_DEBUG_BUILD
  OPT_LEVEL = -Og
else
  USE_DEBUG_BUILD = 
  OPT_LEVEL = -Os
endif

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = $(OPT_LEVEL) -ggdb -fomit-frame-pointer -falign-functions=16 -fdata-sections
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),1)
  USE_SMART_BUILD = yes
endif

# Enable ST-Link pins
ifeq ($(USE_STLINK),1)
  USE_STLINK = -DUSE_STLINK
else
  USE_STLINK = 
endif

# Enable Debug shell
ifeq ($(USE_CMD_SHELL),1)
  USE_CMD_SHELL = -DUSE_CMD_SHELL
else
  USE_CMD_SHELL = 
endif

# Enable USB disconnect circuit
ifeq ($(USE_USB_DISC),1)
  USE_USB_DISC = -DUSE_USB_DISC
else
  USE_USB_DISC = 
endif

# Enable USB bootloader support
ifeq ($(USE_MAPLEMINI_BOOTLOADER),1)
  USE_MAPLEMINI_BOOTLOADER = -DUSE_MAPLEMINI_BOOTLOADER
else
  USE_MAPLEMINI_BOOTLOADER = 
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = AnyKeyFW

# Target settings.
MCU  = cortex-m3

# Imported source files and paths.
CHIBIOS  := ./.3rdparty/ChibiOS_$(OS_VERSION)
CHIBIOS_CONTRIB := $(CHIBIOS)/community
U8G2     := ./.3rdparty/u8g2/csrc
CONFDIR  := ./inc/cfg/chibi
BUILDDIR := ./build
DEPDIR   := ./.dep

# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f1xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS_CONTRIB)/os/hal/hal.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/STM32/STM32F1xx/platform.mk
include $(CHIBIOS)/os/hal/boards/MAPLEMINI_STM32_F103/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional)
include $(CHIBIOS)/os/hal/lib/streams/streams.mk
include $(CHIBIOS)/os/various/shell/shell.mk
	
# u8g2 sources

U8G2_SRC = \
       ${U8G2}/u8x8_setup.c \
       ${U8G2}/u8x8_u8toa.c \
       ${U8G2}/u8g2_d_memory.c \
       ${U8G2}/u8g2_selection_list.c \
       ${U8G2}/u8g2_hvline.c \
       ${U8G2}/u8x8_message.c \
       ${U8G2}/u8g2_kerning.c \
       ${U8G2}/u8x8_input_value.c \
       ${U8G2}/u8g2_polygon.c \
       ${U8G2}/u8g2_input_value.c \
       ${U8G2}/u8x8_8x8.c \
       ${U8G2}/u8x8_debounce.c \
       ${U8G2}/u8g2_setup.c \
       ${U8G2}/u8x8_display.c \
       ${U8G2}/u8g2_box.c \
       ${U8G2}/u8x8_selection_list.c \
       ${U8G2}/u8x8_byte.c \
       ${U8G2}/u8log.c \
       ${U8G2}/u8log_u8x8.c \
       ${U8G2}/u8g2_message.c \
       ${U8G2}/u8g2_line.c \
       ${U8G2}/u8g2_intersection.c \
       ${U8G2}/u8g2_fonts.c \
       ${U8G2}/u8g2_cleardisplay.c \
       ${U8G2}/u8g2_font.c \
       ${U8G2}/u8x8_cad.c \
       ${U8G2}/u8x8_string.c \
       ${U8G2}/u8g2_ll_hvline.c \
       ${U8G2}/u8x8_fonts.c \
       ${U8G2}/u8log_u8g2.c \
       ${U8G2}/u8g2_circle.c \
       ${U8G2}/u8x8_d_ssd1306_64x48.c \
       ${U8G2}/u8g2_buffer.c \
       ${U8G2}/u8g2_bitmap.c \
       ${U8G2}/u8x8_gpio.c \
       ${U8G2}/u8g2_d_setup.c \
       ${U8G2}/u8x8_u16toa.c \

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       ${U8G2_SRC} \
       src/main.c \
       src/app/anykey.c \
       src/app/cmd_shell.c \
       src/hal/flash_storage.c \
       src/hal/glcd.c \
       src/hal/keypad.c \
       src/hal/led.c \
       src/hal/usb.c \

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC)

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC)

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DSHELL_CMD_TEST_ENABLED=0 ${USE_STLINK} ${USE_CMD_SHELL} ${USE_USB_DISC} ${USE_DEBUG_BUILD} ${USE_MAPLEMINI_BOOTLOADER}

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR = $(U8G2) $(CHIBIOS_CONTRIB)/os/various/ inc 

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =


# Define linker script file here
LDSCRIPT_FOLDER=./ld
LDSCRIPT_TMPL=STM32F103x8.ld
LDSCRIPT_GEN=$(LDSCRIPT_TMPL).generated
LDSCRIPT=$(BUILDDIR)/$(LDSCRIPT_TMPL)

$(BUILDDIR)/$(LDSCRIPT_GEN): $(LDSCRIPT_FOLDER)/$(LDSCRIPT_TMPL)
	@echo Generating linker script $@ based on template $<
	mkdir -p $(BUILDDIR)
	$(CC) -E -x c -I inc $(UDEFS) $<  > $@
	
$(LDSCRIPT): $(BUILDDIR)/$(LDSCRIPT_GEN)
	grep -v '^#' $< > $@

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk

#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#
.PHONY : all
#
# Custom rules
##############################################################################
