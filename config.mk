#==============================================================================#
# LIBHBM Default Config                                                        #
#==============================================================================#

# Note that if any of the below options are changed, you will have to 'make clean'
# and recompile the library.

#==============================================================================#

#==============================#
# Hardware
#==============================#

# HBM_SOUND_OUTPUT - enables sound output.
#   0 - disabled
#   1 - ASNDLib
HBM_SOUND_OUTPUT := 1

#==============================================================================#

#==============================#
# UI
#==============================#

# HBM_FORCE_ASPECT_RATIO - forces a certain aspect ratio
#   0 - 4:3 (standard)
#   1 - 16:9 (widescreen)
# HBM_FORCE_ASPECT_RATIO := 0

# HBM_FORCE_ASPECT_RATIO - forces a certain language option
#   by default this is set to English, check include/hbm.h for a list of full language options.
# HBM_LANGUAGE := 1

#==============================================================================#

#==============================#
# Verbose & debug options
#==============================#

# Verbose text
# HBM_VERBOSE := 1
# Debugging features; changes background color and displays square on cursor
# HBM_DEBUG := 1
# Enables use of IPL-style error screen
# HBM_USE_ERROR_SCREEN := 1

#==============================================================================#

# DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING !!

#==============================================================================#

# Preprocessor definitions
HBM_DEFINES :=

ifeq ($(HBM_FORCE_ASPECT_RATIO),0)
	HBM_DEFINES += HBM_FORCE_ASPECT_RATIO=0 
endif
ifeq ($(HBM_FORCE_ASPECT_RATIO),1)
	HBM_DEFINES += HBM_FORCE_ASPECT_RATIO=1 
endif

ifeq ($(HBM_UNSAVED),1)
	HBM_DEFINES += HBM_UNSAVED=1 
else
ifeq ($(HBM_UNSAVED),2)
	HBM_DEFINES += HBM_UNSAVED=2 
else
	HBM_DEFINES += HBM_UNSAVED=0 
endif
endif

ifeq ($(HBM_VERBOSE),1)
	HBM_DEFINES += HBM_VERBOSE=1 
endif
ifeq ($(HBM_DEBUG),1)
	HBM_DEFINES += HBM_DEBUG=1 
endif
ifeq ($(HBM_USE_ERROR_SCREEN),1)
	HBM_DEFINES += HBM_USE_ERROR_SCREEN=1 
endif

ifeq ($(HBM_SOUND_OUTPUT),1)
	HBM_DEFINES += HBM_SOUND_OUTPUT=1
else
	HBM_DEFINES += HBM_SOUND_OUTPUT=0
endif

export	HBM_CDEFINES	:= `$(PREFIX)pkg-config --cflags freetype2` $(foreach d,$(HBM_DEFINES),-D$(d))
export	HBM_LIBS		:= -lpngu -lfreetype `$(PREFIX)pkg-config freetype2 libpng --libs` -lasnd