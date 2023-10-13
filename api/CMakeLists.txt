cmake_minimum_required(VERSION 3.16)

project(libvatek_core)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(core_INC_CORE
    inc/vatek_base.h
    inc/storage_default.h
    inc/binary_rom.h
    inc/chip_define.h
    inc/device_usb.h
    inc/media_audio.h
    inc/media_video.h
    inc/output_base.h
    inc/output_modulator.h
    inc/output_rfmixer.h
    inc/stream_base.h
    inc/stream_encoder.h
    inc/calibration_define.h
    inc/halreg_define.h
    inc/halreg_mux.h
    inc/halreg_output.h
    inc/halreg_playload.h
    inc/halreg_rfmixer.h
    inc/halreg_stream.h
    inc/halreg_calibration.h
    inc/halservice_base.h
    inc/halservice_broadcast.h
    inc/halservice_rescure.h
    inc/halservice_transform.h
    inc/storage_broadcast.h
    inc/storage_config.h
    inc/storage_r2tune.h
    inc/storage_section.h
    inc/storage_transform.h
    inc/tool_8vsb.h
    inc/tool_binary_crc.h
    inc/tool_bufstream.h
    inc/tool_crc32.h
    inc/tool_dvb.h
    inc/tool_dvb_t2.h
    inc/tool_dtmb.h
    inc/tool_helpful.h
    inc/tool_isdb_t.h
    inc/tool_j83abc.h
    inc/tool_ofdm.h
    inc/tool_output.h
    inc/ui_props_api.h
    inc/ui_props_define.h
    inc/ui_props_chip.h
    inc/ui_props_encoder.h
    inc/ui_props_modulator.h
    inc/ui_props_output.h
    inc/ui_props_stream.h)

set(core_INC_CROSS
    inc/cross_bridge.h
    inc/cross_os_api.h
    inc/cross_stream.h
    inc/cross_usb_device_winusb.h)

set(core_INC_MUX
    inc/mux_define.h
    inc/mux_epg.h
    inc/mux_handle.h
    inc/mux_packet.h
    inc/mux_rule.h
    inc/mux_spec.h
    inc/mux_time.h
    inc/spec_arib.h
    inc/spec_dvb.h
    inc/spec_iso_13818.h
    inc/spec_psip.h
    inc/ui_mux_spec_arib.h
    inc/ui_mux_spec_dvb.h
    inc/ui_mux_spec_iso.h
    inc/ui_mux_spec_psip.h
    inc/ui_props_mux.h
    inc/ui_tool_arib.h
    inc/ui_tool_country.h)

set(core_INC_SERVICE
    inc/service_base.h
    inc/service_broadcast.h
    inc/service_transform.h
    inc/ui_service_base.h
    inc/ui_service_broadcast.h
    inc/ui_service_transform.h)

set(core_INC_SDK
    inc/vatek_sdk_bridge.h
    inc/vatek_sdk_broadcast.h
    inc/vatek_sdk_device.h
    inc/vatek_sdk_storage.h
    inc/vatek_sdk_transform.h
    inc/vatek_sdk_usbmux.h
    inc/vatek_sdk_usbstream.h)

set(core_SRC_CORE
    src/chip_define.cpp
    src/calibration_define.cpp
    src/output_modulator.cpp
    src/output_rfmixer.cpp
    src/storage_section.cpp
    src/stream_base.cpp
    src/stream_encoder.cpp
    src/device_usb.cpp
    src/tool_binary_crc.cpp
    src/tool_bufstream.cpp
    src/tool_crc32.cpp
    src/tool_dvb.cpp
    src/tool_dvb_t2.cpp
    src/tool_dtmb.cpp
    src/tool_ofdm.cpp
    src/tool_helpful_bitstream.cpp
    src/tool_helpful_circlebuf.cpp
    src/tool_helpful_fifo.cpp
    src/tool_helpful_mempool.cpp
    src/tool_helpful_memquery.cpp
    src/tool_helpful_objectlist.cpp
    src/tool_isdb_t.cpp
    src/tool_j83abc.cpp
    src/tool_8vsb.cpp
    src/tool_output.cpp
    src/ui_props_api.cpp)

set(core_SRC_CROSS
    src/cross_bridge.cpp
    src/cross_bridge_board.cpp
    src/cross_os_socket.cpp
    src/cross_stream.cpp
    src/cross_stream_file.cpp
    src/cross_stream_test.cpp
    src/cross_stream_udp.cpp
    src/cross_stream_mux.cpp
    src/cross_usb_device_winusb.cpp
)

set(core_SRC_DEVICE
    src/cross_device_bridge.cpp
    src/cross_device_tool.cpp
    src/cross_device_usb.cpp
    src/mux_usbstream.cpp)

set(core_SRC_MUX
    src/mux_epg.cpp
    src/mux_hadnel_playload.cpp
    src/mux_handle_broadcast.cpp
    src/mux_handle_channel.cpp
    src/mux_handle_core.cpp
    src/mux_handle_raw.cpp
    src/mux_packet_base.cpp
    src/mux_packet_pes.cpp
    src/mux_rule.cpp
    src/mux_spec.cpp
    src/mux_time.cpp
    src/mux_tool_tick.cpp)

set(core_SRC_SERVICE
    src/service_base.cpp
    src/service_broadcast.cpp
    src/service_transform.cpp)

set(core_SRC_SDK
    src/cross_os_api.cpp
    src/vatek_sdk_base.cpp
    src/vatek_sdk_bridge.cpp
    src/vatek_sdk_broadcast.cpp
    src/vatek_sdk_device.cpp
    src/vatek_sdk_storage.cpp
    src/vatek_sdk_transform.cpp
    src/vatek_sdk_usbmux.cpp
    src/vatek_sdk_usbstream.cpp)

set(core_SRC_OS
    src/win_os_bridge.cpp
    src/win_os_common.cpp
    src/win_os_event_api.cpp
    src/win_os_mutex_api.cpp
    src/win_os_process.cpp
    src/win_os_smem_api.cpp
    src/win_os_sys_api.cpp
    src/win_os_thread_api.cpp
    src/win_os_uart_api.cpp
)

if($(core_SRC_OS))
    message(FATAL_ERROR "OS not supported [supported : win|linux]")
endif()

set(sdk2_SOURCES
    ${core_SRC_CORE}
    ${core_SRC_CROSS}
    ${core_SRC_DEVICE}
    ${core_SRC_MUX}
    ${core_SRC_OS}
    ${core_SRC_SDK}
    ${core_SRC_SERVICE})

set(sdk2_HEADERS
    inc/bridge_base.h
    inc/bridge_demod.h
    inc/bridge_device.h
    inc/bridge_rfmixer.h
    inc/bridge_source.h
    inc/bridge_usb.h
    inc/ui_bridge_base.h
    inc/ui_bridge_source.h

    ${core_INC_CORE}
    ${core_INC_CROSS}
    ${core_INC_MUX}
    ${core_INC_SDK}
    ${core_INC_SERVICE})

if(MSVC)
    source_group("src" FILES ${core_SRC_SDK})
endif()

include_directories("inc")

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)

    # libvatek_core #
	
	if(NOT ${SDK2_EN_STATIC_ONLY})
        set(sdk2_RCFILE ${CMAKE_BINARY_DIR}/libvatek_core.rc)
        configure_file(./cmake/core.rc.in ${sdk2_RCFILE})

		add_library(libvatek_core SHARED ${sdk2_SOURCES} ${sdk2_HEADERS} ${sdk2_RCFILE})
		target_compile_definitions(libvatek_core PRIVATE -D_VA2_DLL_)
		target_compile_options(libvatek_core PRIVATE /wd26495 PRIVATE /wd26812 PRIVATE /wd26498)
		set_property(TARGET libvatek_core PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		set_target_properties(libvatek_core PROPERTIES LINK_FLAGS "/LTCG /SUBSYSTEM:WINDOWS")
		set_target_properties(libvatek_core PROPERTIES 
							  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
							  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
							  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	endif()

    # libvatek_core_static #
    add_library(libvatek_core_static STATIC ${sdk2_SOURCES} ${sdk2_HEADERS})
    target_compile_options(libvatek_core_static PRIVATE /wd26495 PRIVATE /wd26812 PRIVATE /wd26498)
    target_compile_definitions(libvatek_core_static PRIVATE -D_VA2_STATIC_)
    set_property(TARGET libvatek_core_static PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    set_target_properties(libvatek_core_static PROPERTIES
                          ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                          LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    
endif()
