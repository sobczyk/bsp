

set (src_lpclib
source/lpc_api.c
source/lpc_fat16.c
source/lpc_helvr10.c
source/lpc_nandflash_params.c
source/lpc_swim.c
source/lpc_x5x7.c
source/lpc_arm922t_cp15_driver.c
source/lpc_fat16_private.c
source/lpc_lbecc.c
source/lpc_rom8x16.c
source/lpc_swim_font.c
source/lpc_x6x13.c
source/lpc_bmp.c
source/lpc_fonts.c
source/lpc_lcd_params.c
source/lpc_rom8x8.c
source/lpc_swim_image.c
source/lpc_colors.c
source/lpc_heap.c
source/lpc_line_parser.c
source/lpc_string.c
source/lpc_winfreesystem14x16.c
)

ADD_LIBRARY(lpcLIB STATIC ${src_lpclib})