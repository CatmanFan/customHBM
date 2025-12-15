#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC)
endif

include $(DEVKITPPC)/wii_rules
# include $(DEVKITPRO)/libogc2/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	$(shell find source -type d)
INCLUDES	:=	$(shell find source -type d)
DATA		:=	data

#---------------------------------------------------------------------------------
# HBM
# DATA is a list of directories containing data files
# ROMFS is a folder to generate app's romfs
#---------------------------------------------------------------------------------
SOURCES		+=	hbm/source hbm/source/utils/i18n
INCLUDES	+=	hbm/source hbm/source/utils/i18n
DATA		+=	hbm/bin hbm/bin/sfx
ROMFS       :=  romfs

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

# -mrvl
CFLAGS	= -g -O2 -mrvl -Wall $(MACHDEP) $(INCLUDE) -I$(DEVKITPPC)/local/include `freetype-config --cflags`
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
# LIBS    :=    -lwiisprite -lpng -lz -lfreetype -lfat
# LIBS    :=    -lgrrlib -lpngu `$(PREFIX)pkg-config freetype2 libpng libjpeg --libs` -lfat
LIBS    :=    -lwiisprite -lpng -lz -lfreetype -lgrrlib -lpngu `$(PREFIX)pkg-config freetype2 libpng libjpeg --libs` -lfat
LIBS    +=    -lasnd
LIBS    +=    -lwiiuse -lbte -logc -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin) \
				$(wildcard $(dir)/*.jpg) \
				$(wildcard $(dir)/*.png) \
				$(wildcard $(dir)/*.ttf) \
				$(wildcard $(dir)/*.pcm) \
				$(wildcard $(dir)/*.lang)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) -I$(PORTLIBS)/include/freetype2

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:= -L$(LIBOGC_LIB) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#---------------------------------------------------------------------------------
run:
	psoload $(TARGET).dol

#---------------------------------------------------------------------------------
reload:
	psoload -r $(TARGET).dol


#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# LIBROMFS-OGC
#---------------------------------------------------------------------------------
include $(PORTLIBS_PATH)/wii/share/romfs-ogc.mk
CFLAGS		+=	$(ROMFS_CFLAGS)
CXXFLAGS	+=	$(ROMFS_CFLAGS)
LIBS		+=	$(ROMFS_LIBS)
OFILES		+=	$(ROMFS_TARGET)
#---------------------------------------------------------------------------------

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

$(OFILES_SOURCES) : $(HFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the extensions: jpg png ttf pcm lang bin
#---------------------------------------------------------------------------------
%.jpg.o %_jpg.h	:	%.jpg
	@echo $(notdir $<)
	$(bin2o)

%.png.o %_png.h :	%.png
	@echo $(notdir $<)
	$(bin2o)

%.ttf.o %_ttf.h	:	%.ttf
	@echo $(notdir $<)
	$(bin2o)

%.pcm.o %_pcm.h :	%.pcm
	@echo $(notdir $<)
	$(bin2o)

%.lang.o :	%.lang
	@echo $(notdir $<)
	$(bin2o)
	
%.bin.o %_bin.h	:	%.bin
	@echo $(notdir $<)
	@$(bin2o)

$(ROMFS_TARGET):
	@echo ROMFS $(notdir $@)
	$(Q)tar --format=ustar -cvf romfs.tar -C $(CURDIR)/../$(ROMFS) .
	$(Q)$(OBJCOPY) --input-target binary --output-target elf32-powerpc --binary-architecture powerpc:common romfs.tar $@
	@rm -f romfs.tar

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
