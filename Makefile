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
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing extra header files
# ROMFS is a folder to generate app's romfs
#---------------------------------------------------------------------------------
TARGET		:=	libhbm
INSTALL		:=	$(DEVKITPRO)/portlibs/wii

BUILD		:=	build
INCLUDES	:=	include
SOURCES		:=	source source/classes
DATA		:=	source/files/png source/files/sfx

ifneq ($(BUILD),$(notdir $(CURDIR)))
include	$(CURDIR)/config.mk
else
include	$(CURDIR)/../config.mk
endif

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -Wall $(MACHDEP) $(INCLUDE) $(HBM_CDEFINES)
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl -mrvl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS    :=    $(HBM_LIBS) -lwiiuse -lbte -logc -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(LIBOGC_LIB) $(PORTLIBS)
FREETYPE_INCLUDE ?= $(PORTLIBS_PATH)/ppc/include/freetype2

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
					$(wildcard $(dir)/*.pcm) $(wildcard $(dir)/*.lang) \
					$(wildcard $(dir)/*.ttf)))

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

install: build
	@echo installing HBM library to $(DESTDIR)$(INSTALL)
	@rm -fr $(DESTDIR)$(INSTALL)/lib/libhbm.a $(DESTDIR)$(INSTALL)/include/hbm.h $(DESTDIR)$(INSTALL)/include/hbm
	@cp $(TARGET).a $(DESTDIR)$(INSTALL)/lib/libhbm.a
	@cp include/hbm.h $(DESTDIR)$(INSTALL)/include/hbm.h
	@cp -r include/hbm $(DESTDIR)$(INSTALL)/include

uninstall:
	@echo uninstalling HBM library from $(DESTDIR)$(INSTALL)
	@rm -fr $(DESTDIR)$(INSTALL)/lib/libhbm.a $(DESTDIR)$(INSTALL)/include/hbm.h $(DESTDIR)$(INSTALL)/include/hbm

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
# This rule links in binary data with these extensions: png pcm lang ttf romfs
#---------------------------------------------------------------------------------
%.png.o %_png.h : %.png
	@echo $(notdir $<)
	$(bin2o)
	
%.pcm.o %_pcm.h : %.pcm
	@echo $(notdir $<)
	$(bin2o)
	
%.lang.o %_lang.h : %.lang
	@echo $(notdir $<)
	$(bin2o)
	
%.ttf.o %_ttf.h : %.ttf
	@echo $(notdir $<)
	$(bin2o)
	
$(ROMFS_TARGET):
	@echo ROMFS $(notdir $@)
	$(Q)tar --format=ustar -cvf romfs.tar -C $(CURDIR)/../$(ROMFS) .
	$(Q)$(OBJCOPY) --input-target binary --output-target elf32-powerpc --binary-architecture powerpc:common romfs.tar $@
	@rm -f romfs.tar
#---------------------------------------------------------------------------------

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------