cmake_minimum_required(VERSION 3.16)

project(app_bridge)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(cli_SRC
    "app_bridge.cpp")

set(cli_SRC_COM
    "../../api/src/tool_printf.cpp")

set(cli_INC_COM
    tool_printf.h)

if(MSVC)
	if(NOT TARGET libvatek_core)
   	 	message(FATAL_ERROR "app_bridge need build with libvatek_core")
	endif()

	add_executable(app_bridge ${cli_SRC} ${cli_SRC_COM} ${cli_SRC_INTERNAL})
	target_include_directories(app_bridge PRIVATE "inc" "../../api/inc")
	target_link_libraries(app_bridge libvatek_core)

elseif(UNIX)
	if(NOT TARGET vatek_core)
    		message(FATAL_ERROR "app_bridge need build with vatek_core")
	endif()

	add_executable(app_bridge ${cli_SRC} ${cli_SRC_COM} ${cli_SRC_INTERNAL})
	target_include_directories(app_bridge PRIVATE "inc" "../../api/inc")
	target_link_libraries(app_bridge vatek_core)    
    
endif()

if(MSVC)
    set_property(TARGET app_bridge PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(app_bridge PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
    target_compile_options(app_bridge PRIVATE /wd26495 PRIVATE /wd26812 PRIVATE /wd26498)

    source_group("src\\" FILES ${cli_SRC})

elseif(UNIX)
    SET_TARGET_PROPERTIES(app_bridge PROPERTIES INSTALL_RPATH "$ORIGIN/lib:$ORIGIN/")
endif()

set_target_properties(app_bridge PROPERTIES 
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
