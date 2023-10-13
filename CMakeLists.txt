cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

project(vatek_sdk_2)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 为我的库添加编译
message(STATUS "MyCppCommonLib value: $ENV{MyCppCommonLib}")
add_subdirectory($ENV{MyCppCommonLib} ${CMAKE_BINARY_DIR}/MyCppCommonLib)
# 这条命令要放在开头，一开始就引入头文件，这样，下面被构建的项目才能看到头文件
include_directories($ENV{MyCppCommonLib})

if(MSVC)
    option(SDK2_EN_SAMPLE "build sample app enable" ON)

elseif(UNIX AND NOT APPLE)
    option(SDK2_EN_SAMPLE "build sample app enable" ON)   
    install(
        FILES "${PROJECT_SOURCE_DIR}/build/linux/udev/50-usb-vatek.rules" 
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d/"
    )

elseif(APPLE)
    option(SDK2_EN_SAMPLE "build sample app enable" ON)

else()
    message(FATAL_ERROR "unsupported operation system")

endif()

option(SDK2_EN_STATIC_ONLY "build static library only" OFF)

add_subdirectory(api)

if(${SDK2_EN_SAMPLE})
	add_subdirectory(sample/app_stream)
	add_subdirectory(sample/app_romtool)
	add_subdirectory(sample/app_broadcast)
	add_subdirectory(sample/app_bridge)
endif()
