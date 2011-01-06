# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER_FLAGS -mcpu=arm926ej-s -Wall -Os -mno-sched-prolog -fno-hosted -mno-thumb-interwork)

# specify the cross compiler
SET(CMAKE_C_COMPILER   arm-linux-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-g++)
SET(CMAKE_ASM-ATT_COMPILER arm-linux-as)
INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)

macro(make_bin target)
ADD_CUSTOM_COMMAND(
	TARGET ${target} POST_BUILD
   COMMAND arm-linux-objcopy -I elf32-littlearm -O binary --strip-all --verbose ${EXECUTABLE_OUTPUT_PATH}/${target} ${EXECUTABLE_OUTPUT_PATH}/${target}.bin
   COMMENT "Copying ${target} to binary"
   )
endmacro(make_bin)

# srec -O srec --strip-all --verbose kickstart_nand_large_block_gnu.elf kickstart_nand_large_block_gnu.srec


# specify the cross compiler
CMAKE_FORCE_C_COMPILER(arm-linux-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-linux-g++ GNU)


# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH  /opt/eldk-2007-01-19/ppc_74xx /home/alex/eldk-ppc74xx-inst)

# search for programs in the build host directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
