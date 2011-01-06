
include_directories(include)

set (s1l_main_src
	source/s1l_bootmgr.c
	source/s1l_cmds.c
	source/s1l_cmds_core.c
	source/s1l_cmds_flash.c
	source/s1l_cmds_image.c
	source/s1l_fat.c
	source/s1l_image_mngt.c
	source/s1l_line_input.c
	source/s1l_memtests.c
	source/s1l_sys.c
	source/s1l_except.asm
	source/s1l_movdat.asm
	PARENT_SCOPE
)