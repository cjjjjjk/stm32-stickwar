# Relative location of the TouchGFX framework from root of application
touchgfx_path := ../Middlewares/ST/touchgfx

# Location of the TouchGFX Environment
touchgfx_env := ../../../../TouchGFX/4.26.1/env

# Optional additional compiler flags
user_cflags := -DUSE_BPP=16

# Any Additional components
# e.g. components := 

# Any Additional include paths
# e.g. include_paths := 

# Additional compiler/linker flags
# e.g. board_cflags :=
#      board_lflags :=

include $(touchgfx_path)/config/gcc/app.mk