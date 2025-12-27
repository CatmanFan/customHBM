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

# export	FREETYPE_CFLAGS 	:=	`$(DEVKITPRO)/portlibs/ppc/bin/powerpc-eabi-pkg-config --cflags freetype2`
# export	FREETYPE_LIBS	:=	`$(DEVKITPRO)/portlibs/ppc/bin/powerpc-eabi-pkg-config --libs freetype2`
# export	FREETYPE_INCLUDE	:=	$(PORTLIBS)/include/freetype2

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing extra header files
# ROMFS is a folder to generate app's romfs
#---------------------------------------------------------------------------------
TARGET		:=	hbm
BUILD		:=	build
INCLUDES	:=	include
SOURCES		:=	source source/classes
DATA		:=	source/png source/sfx
ROMFS       :=  source/romfs
INSTALL		:=	$(DEVKITPRO)/portlibs/wii

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -mrvl -Wall $(MACHDEP) $(INCLUDE) -I$(DEVKITPPC)/local/include `$(PREFIX)pkg-config --cflags freetype2`
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl -mrvl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
# LIBS    :=    -lfreetype `$(PREFIX)pkg-config freetype2 libpng --libs` -lasnd -lwiiuse -lbte -logc -lm
LIBS    :=    -lfreetype `$(PREFIX)pkg-config freetype2 libpng --libs` -lpngu -lasnd -lwiiuse -lbte -logc -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(LIBOGC_LIB) $(PORTLIBS) $(PORTLIBS)/include/freetype2

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
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.png) \
				$(wildcard $(dir)/*.pcm)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_SOURCES	:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES_BIN		:= $(addsuffix .o,$(BINFILES))
export OFILES 			:= $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) \
					-I$(FREETYPE_INCLUDE)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) lib $(OUTPUT).a $(TARGET).tar.bz2

dist-bin: all
	@echo packing HBM library
	@[ -d lib ] || mkdir -p lib
	@cp $(TARGET).a lib/lib$(TARGET).a
	@tar --exclude=*~ -cjf $(TARGET).tar.bz2 include lib

install: dist-bin
	@echo installing HBM library to $(DESTDIR)$(INSTALL)
	mkdir -p $(DESTDIR)$(INSTALL)
	bzip2 -cd $(TARGET).tar.bz2 | tar -xf - -C $(DESTDIR)$(INSTALL)

all:	build

#---------------------------------------------------------------------------------
%.a:
	@echo linking to lib ... $(notdir $@)
	@$(AR) -rc $@ $^
#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# REQUIRED FOR HBM (libromfs-ogc)
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
$(OUTPUT).a: $(OFILES)
#---------------------------------------------------------------------------------
$(OFILES_SOURCES) : $(HFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the extensions: png pcm
#---------------------------------------------------------------------------------
%.png.o %_png.h :	%.png
	@echo $(notdir $<)
	$(bin2o)

%.pcm.o %_pcm.h :	%.pcm
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the ROMFS data
#---------------------------------------------------------------------------------
$(ROMFS_TARGET):
	@echo ROMFS $(notdir $@)
	$(Q)tar --format=ustar -cvf romfs.tar -C $(CURDIR)/../$(ROMFS) .
	$(Q)$(OBJCOPY) --input-target binary --output-target elf32-powerpc --binary-architecture powerpc:common romfs.tar $@
	@rm -f romfs.tar

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
