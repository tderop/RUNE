#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# ROMFS is the directory containing data to be added to RomFS, relative to the Makefile (Optional)
#
# NO_ICON: if set to anything, do not use icon.
# NO_NACP: if set to anything, no .nacp file is generated.
# APP_TITLE is the name of the app stored in the .nacp file (Optional)
# APP_AUTHOR is the author of the app stored in the .nacp file (Optional)
# APP_VERSION is the version of the app stored in the .nacp file (Optional)
# APP_TITLEID is the titleID of the app stored in the .nacp file (Optional)
# ICON is the filename of the icon (.jpg), relative to the project folder.
# CONFIG_JSON is the filename of the NPDM config file (.json), relative to the project folder.
#---------------------------------------------------------------------------------
APP_TITLE   := RUNE
# APP_TITLEID := 0001000000000016
APP_VERSION := 0.0.0
APP_AUTHOR  := tderop
APP_ICON    := $(CURDIR)/RUNE.jpg

TARGET      := $(notdir $(CURDIR))

BUILD      := build
SOURCES    := src \
              src/Events \
              src/Input \
              src/Utils \
              src/Save \
              src/Screens \
              src/Application \
              src/Editor \
              src/Editor/Menus \
              src/Editor/Menus/Inventory \
              src/Editor/Widgets \
              src/Editor/Widgets/Generic \
              src/Editor/Widgets/Unique \
              src/Screens/Editor \
              src/Json

# Preserve source paths so same-named files in different Aether directories stay distinct.
AETHER_SOURCE_DIR := $(if $(TOPDIR),$(TOPDIR),$(CURDIR))/external/Aether/source
AETHER_SOURCE_FILES := $(shell find "$(AETHER_SOURCE_DIR)" -type f -name "*.cpp")
AETHER_OBJECT_FILES := $(patsubst $(AETHER_SOURCE_DIR)/%.cpp,aether/%.o,$(AETHER_SOURCE_FILES))

# DATA       := data
INCLUDES   := include src \
              external/Aether/include \
              libs/json/include \
              libs/stb/include

OUTPUTDIR  := output
ROMFS      := romfs

# requires rsync
ROMFS_FILTER ?= 0
ROMFS_BLACKLIST ?= img/thumb sound
ROMFS_STAGING := $(TOPDIR)/$(BUILD)/romfs_staging

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH       := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS     := -g -Wall -O3 -ffunction-sections \
              $(ARCH) $(DEFINES)
CFLAGS     += $(INCLUDE) -D__SWITCH__

CXXFLAGS   := $(CFLAGS) -fno-rtti -fno-exceptions -Wno-unused-function
CXXFLAGS   += -std=gnu++17 -Wno-overloaded-virtual -Wno-unused-variable -Wno-unused-but-set-variable

ASFLAGS    := -g $(ARCH)
LDFLAGS    = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) $(LIBPATHS) -Wl,-Map,$(notdir $*.map)

LIBS := -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_gfx \
        -lEGL -lglapi -lGLESv2 -ldrm_nouveau \
        -lm \
        -lfreetype -lpng -ljpeg -lwebp \
        -lharfbuzz -lbz2 -lz \
        -lstdc++ \
        -lSDL2_mixer -lvorbisidec -lmodplug -lmpg123 -lopusfile -logg -lopus \
        -lnx \

#---------------------------------------------------------------------------------
# list of directories containing libraries
#---------------------------------------------------------------------------------
LIBDIRS := $(PORTLIBS) $(LIBNX) $(CURDIR)/libs/spdlog

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT := $(CURDIR)/$(OUTPUTDIR)/$(TARGET)
export TOPDIR := $(CURDIR)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)
export AETHER_OBJECT_FILES

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES_BIN := $(addsuffix .o,$(BINFILES))
export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o) $(AETHER_OBJECT_FILES)
export OFILES := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(CONFIG_JSON)),)
    jsons := $(wildcard *.json)
    ifneq (,$(findstring $(TARGET).json,$(jsons)))
        export APP_JSON := $(TOPDIR)/$(TARGET).json
    else
        ifneq (,$(findstring config.json,$(jsons)))
            export APP_JSON := $(TOPDIR)/config.json
        endif
    endif
else
    export APP_JSON := $(TOPDIR)/$(CONFIG_JSON)
endif

ifeq ($(strip $(ICON)),)
    icons := $(wildcard *.jpg)
    ifneq (,$(findstring $(TARGET).jpg,$(icons)))
        export APP_ICON := $(TOPDIR)/$(TARGET).jpg
    else
        ifneq (,$(findstring icon.jpg,$(icons)))
            export APP_ICON := $(TOPDIR)/icon.jpg
        endif
    endif
else
    export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
    export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
    export NROFLAGS += --nacp=$(OUTPUT).nacp
endif

ifneq ($(APP_TITLEID),)
    export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
    ifeq ($(ROMFS_FILTER),1)
        export NROFLAGS += --romfsdir=$(ROMFS_STAGING)
    else
        export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
    endif
endif

.PHONY: clean all romfs-stage

all: $(OUTPUTDIR) $(BUILD)

$(OUTPUTDIR):
	@[ -d $@ ] || mkdir -p $@

# Stage assets before the recursive build; do not modify the original RomFS.
romfs-stage:
ifeq ($(ROMFS_FILTER),1)
ifneq ($(ROMFS),)
	@echo "Preparing filtered RomFS..."
	@mkdir -p "$(ROMFS_STAGING)"
	@rsync -a --delete $(foreach path,$(ROMFS_BLACKLIST),--exclude='/$(path)') "$(CURDIR)/$(ROMFS)/" "$(ROMFS_STAGING)/"
endif
endif

$(BUILD): romfs-stage
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
ifeq ($(strip $(APP_JSON)),)
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf
else
	@rm -fr $(BUILD) $(TARGET).nsp $(TARGET).nso $(TARGET).npdm $(TARGET).elf
endif

#---------------------------------------------------------------------------------
else
.PHONY: all FORCE_ROMFS

DEPENDS := $(OFILES:.o=.d)

ifeq ($(strip $(APP_JSON)),)

all: $(OUTPUT).nro

# RomFS contents and blacklist can change without an ELF rebuild.
# Force NRO repackaging, but do not recompile source files.
FORCE_ROMFS:
$(OUTPUT).nro: FORCE_ROMFS

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro: $(OUTPUT).elf
endif

else

all: $(OUTPUT).nsp

$(OUTPUT).nsp: $(OUTPUT).nso $(OUTPUT).npdm
$(OUTPUT).nso: $(OUTPUT).elf

endif

$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC): $(HFILES_BIN)

aether/%.o: $(TOPDIR)/external/Aether/source/%.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -MT $@ -c $< -o $@

%.bin.o %_bin.h : %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
#---------------------------------------------------------------------------------
