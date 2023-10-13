cmake_minimum_required(VERSION 3.16)

project(app_broadcast)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(cli_SRC
    "app_broadcast.cpp")

set(cli_SRC_COM
    "../../api/src/tool_printf.cpp"
    "../../api/src/tool_stream_file.cpp"
    "../../api/src/tool_stream_udp.cpp"
    "../../api/src/tool_tspacket.cpp")

set(cli_INC_COM
    tool_printf.h
    tool_stream.h
    tool_tspacket.h)

if(MSVC)
	if(NOT TARGET libvatek_core)
   	 	message(FATAL_ERROR "app_broadcast need build with libvatek_core")
	endif()

	add_executable(app_broadcast ${cli_SRC} ${cli_SRC_COM} ${cli_SRC_INTERNAL})
	target_include_directories(app_broadcast PRIVATE "inc" "../../api/inc")
	target_link_libraries(app_broadcast libvatek_core)

elseif(UNIX)
	if(NOT TARGET vatek_core)
    		message(FATAL_ERROR "app_broadcast need build with vatek_core")
	endif()

	add_executable(app_broadcast ${cli_SRC} ${cli_SRC_COM} ${cli_SRC_INTERNAL})
	target_include_directories(app_broadcast PRIVATE "inc" "../../api/inc")
	target_link_libraries(app_broadcast vatek_core)    
    
endif()

if(MSVC)
    set_property(TARGET app_broadcast PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(app_broadcast PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
    target_compile_options(app_broadcast PRIVATE /wd26495 PRIVATE /wd26812 PRIVATE /wd26498)

    source_group("src\\" FILES ${cli_SRC})

elseif(UNIX)
    SET_TARGET_PROPERTIES(app_broadcast PROPERTIES INSTALL_RPATH "$ORIGIN/lib:$ORIGIN/")
endif()

set_target_properties(app_broadcast PROPERTIES 
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
