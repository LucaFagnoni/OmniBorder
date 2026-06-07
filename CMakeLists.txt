cmake_minimum_required(VERSION 3.20)
project(OmniBorder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(wxWidgets CONFIG REQUIRED)

set(SOURCES
    src/app.cpp
    src/main.cpp
    src/main_ui.cpp
    src/window_manager.cpp
    src/monitor_manager.cpp
    src/hotkey_manager.cpp
    src/config_manager.cpp
    src/language_manager.cpp
    src/tray_icon.cpp
    src/logger.cpp
    resources.rc
)

set(HEADERS
    src/types.h
    src/main.h
    src/window_manager.h
    src/monitor_manager.h
    src/hotkey_manager.h
    src/config_manager.h
    src/language_manager.h
    src/tray_icon.h
    src/logger.h
)

add_executable(OmniBorder WIN32 ${SOURCES} ${HEADERS})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/languages" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(OmniBorder PRIVATE wx::core wx::base)

target_compile_definitions(OmniBorder PRIVATE
    UNICODE
    _UNICODE
)

if(MSVC)
    target_compile_definitions(OmniBorder PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
    
    add_compile_options(/utf-8)
    
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "src" FILES ${SOURCES} ${HEADERS})

set_target_properties(OmniBorder PROPERTIES
    OUTPUT_NAME "OmniBorder"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)