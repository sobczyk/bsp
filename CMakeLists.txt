cmake_minimum_required (VERSION 2.8)
project (LPC3250BSPS)
message ("LPC3250 Board Support Package")
message ("By default FDI3250 targets are built")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

enable_language(ASM-ATT)
set(CMAKE_ASM-ATT_FLAGS "--defsym USE_MMU=1 --defsym USE_BOARD_INIT=1")

OPTION(FDI3250 "FDI3250 dev kit target" ON)
include_directories(lpc/include)
include_directories(ip/s1l/include)
add_subdirectory (lpc)
add_subdirectory (csps/lpc32xx/)
