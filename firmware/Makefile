######################################
# target
######################################
TARGET = bms_operating_system


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O3

#######################################
# paths
#######################################

# Build path
BUILD_DIR = build

.PRECIOUS: $(BUILD_DIR)/%.o

#######################################
# includes
#######################################

# AS includes
AS_INCLUDES =  \
-ICore/Inc

# C includes
C_INCLUDES_DIRS =  \
	Core/Inc \
	Core/Inc/logging \
	Core/Inc/tcp_server \
	Core/Inc/bsm \
	Drivers/STM32L4xx_HAL_Driver/Inc \
	Drivers/STM32L4xx_HAL_Driver/Inc/Legacy \
	Drivers/CMSIS/Include \
	Drivers/CMSIS/Device/ST/STM32L4xx/Include \
	Middlewares/FreeRTOS/Source/include \
	Middlewares/FreeRTOS/Source/CMSIS_RTOS \
	Middlewares/FreeRTOS/Source/portable/GCC/ARM_CM4F \
	Middlewares/FreeRTOS_Plus_TCP/source/include \
	Middlewares/FreeRTOS_Plus_TCP/source/portable/Compiler/GCC \
	Middlewares/bq76940/include \
	Middlewares/enc28j60/include \

C_INCLUDES = $(addprefix -I, $(C_INCLUDES_DIRS))


######################################
# source
######################################

SUBDIR = Core Middlewares Drivers

C_SOURCES = $(sort $(foreach fd, $(SUBDIR), $(shell find $(fd) -name '*.c')))
CPP_SOURCES = $(sort $(foreach fd, $(SUBDIR), $(shell find $(fd) -name '*.cpp')))
ASM_SOURCES = startup_stm32l476xx.s


#######################################
# binaries
#######################################

PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CPP = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CPP = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb -specs=nano.specs -specs=nosys.specs $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L476xx

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
endif



#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32L476RGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage -u _printf_float

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CPP) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR) 
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.elf: $(OBJECTS) | Makefile
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@	

#######################################
# clean up
#######################################

clean:
	-rm -fR $(BUILD_DIR)

#######################################
# flash
#######################################

flash:
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x8000000
  
#######################################
# format
#######################################

format:
	clang-format -i -style=file --verbose $(sort $(foreach fd, $(SUBDIR), $(shell find $(fd) -name "*.h" -or -name "*.cpp" -or -name "*.c" -or -name "*.hpp")))
  

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
