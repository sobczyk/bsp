
include_directories(include ip/s1l/include)

add_subdirectory (source)
add_subdirectory (ip/s1l)


foreach ( var ${s1l_main_src})
	set (tmpvar ${tmpvar} ../../../ip/s1l/${var})
endforeach( var)
set (s1l_main_src ${tmpvar})
add_subdirectory (bsps)
