include_directories(. board sdram examples/common examples/s1l)

set(src_kicker
	examples/kickstart/kickstart_lb_nand/kickstart_lb_nand.c
	examples/common/uart.c
	examples/common/write_protect_disable.c
	board/board_init.c
	board/clock_setup.c
	board/gpio_setup.c
	board/mem_setup.c
	board/mmu_setup.c
	sdram/ddr_lp_sdram.c
	sdram/sdram_common.c
)

set(src_s1l
	board/board_init.c
	board/mmu_setup.c
	board/clock_setup.c
	board/gpio_setup.c
	board/mem_setup.c
	sdram/ddr_lp_sdram.c
	sdram/sdram_common.c
	examples/common/write_protect_disable.c
	examples/s1l/cfg_save.c
	examples/s1l/sysapi_blkdev.c
	examples/s1l/sysapi_flash.c
	examples/s1l/sysapi_misc.c
	examples/s1l/sysapi_termio.c
	examples/s1l/sysapi_timer.c
	examples/s1l/sys_hw.c
	examples/s1l/sys_mmu_cmd_group.c
	cache_support.asm
)

set(s_entry_src
	startup_entry.asm
)

macro(ld1copy targ)
	set(s_entry
		CMakeFiles/${targ}.dir/startup_entry.asm.o
	)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/examples/buildfiles/ldscript_iram_gnu.ld ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${targ}.dir/ldscript_iram_gnu.ld)
endmacro(ld1copy)

macro(ld2copy targ)
	set(s_entry
		CMakeFiles/${targ}.dir/startup_entry.asm.o
	)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/examples/buildfiles/ldscript_iram32k_gnu.ld ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${targ}.dir/ldscript_iram32k_gnu.ld)
endmacro(ld2copy)

set (CMAKE_ASM-ATT_FLAGS "--defsym USE_MMU=1 --defsym USE_BOARD_INIT=1")
#kickstart
ld1copy(kickstart)
add_executable(kickstart ${src_kicker} ${s_entry_src})
target_link_libraries(kickstart lpcfdiLIB lpc32xxLIB lpcLIB gcc)
set_target_properties(kickstart PROPERTIES LINK_FLAGS "-static -ffreestanding -nostdinc -nostartfiles -Wl,--gc-sections -T ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/kickstart.dir/ldscript_iram_gnu.ld")
make_bin(kickstart)

#s1l
ld2copy(s1l_kick)
add_executable(s1l_kick ${src_s1l} ${s1l_main_src} ${s_entry_src})
target_link_libraries(s1l_kick lpcfdiLIB lpc32xxLIB lpcLIB gcc)
set_target_properties(s1l_kick PROPERTIES LINK_FLAGS "-static -ffreestanding -nostdinc -nostartfiles -Wl,--gc-sections -T ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/s1l_kick.dir/ldscript_iram32k_gnu.ld")
make_bin(s1l_kick)


#burners
#AFLAGS += --defsym USE_BOARD_INIT=1
#all have common sources
set ( burner_src
	examples/common/write_protect_disable.c
	examples/common/uart.c
	board/board_init.c
	board/mmu_setup.c
	board/clock_setup.c
	board/gpio_setup.c
	board/mem_setup.c
	sdram/ddr_lp_sdram.c
	sdram/sdram_common.c
	cache_support.asm
)

#set (CMAKE_ASM-ATT_FLAGS "--defsym USE_BOARD_INIT=1")
add_executable(kickstart_jtag examples/Burners/nand_lb/kickstart_jtag/kickstart_nand_lb_burn_jtag.c ${burner_src} ${s_entry_src})
target_link_libraries(kickstart_jtag lpcfdiLIB lpc32xxLIB lpcLIB gcc)
set_target_properties(kickstart_jtag PROPERTIES LINK_FLAGS "-static -ffreestanding -nostdinc -nostartfiles -Wl,--gc-sections -T ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/kickstart_jtag.dir/ldscript_iram_gnu.ld")
ld1copy(kickstart_jtag)
make_bin(kickstart_jtag)

add_executable(s1l_jtag examples/Burners/nand_lb/s1lapp_jtag/s1app_nand_lb_burn_jtag.c ${burner_src} ${s_entry_src})
target_link_libraries(s1l_jtag lpcfdiLIB lpc32xxLIB lpcLIB gcc)
set_target_properties(s1l_jtag PROPERTIES LINK_FLAGS "-static -ffreestanding -nostdinc -nostartfiles -Wl,--gc-sections -T ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/s1l_jtag.dir/ldscript_iram_gnu.ld")
ld1copy(s1l_jtag)
make_bin(s1l_jtag)


#nand erase
set (nand_src
	examples/Burners/nand_lb/nanderase/nanderase.c
	examples/common/write_protect_disable.c
	examples/common/uart.c
	cache_support.asm
)

#set (CMAKE_ASM-ATT_FLAGS "")
add_executable(nand_erase_jtag ${nand_src} ${s_entry_src})
target_link_libraries(nand_erase_jtag lpcfdiLIB lpc32xxLIB lpcLIB gcc)
set_target_properties(nand_erase_jtag PROPERTIES LINK_FLAGS "-static -ffreestanding -nostdinc -nostartfiles -Wl,--gc-sections -T ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/nand_erase_jtag.dir/ldscript_iram_gnu.ld")
ld1copy(nand_erase_jtag)
make_bin(nand_erase_jtag)

