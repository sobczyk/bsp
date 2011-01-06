
include_directories(include)

add_subdirectory (startup)

set(src_libfdi
	source/board.c
	source/board_i2c_eeprom_driver.c
	source/board_mlc_nand_lb_driver.c
	source/board_slc_nand_lb_driver.c
	source/libnosys_gnu.c
	source/nand_mlc_common.c
	source/nand_slc_common.c
	source/nand_support_common.c
)


ADD_LIBRARY(lpcfdiLIB STATIC ${src_libfdi})