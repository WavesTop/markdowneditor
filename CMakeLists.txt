cmake_minimum_required(VERSION 3.16)

project(MarkdownEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 解决macOS SDK中AGL框架已被废弃的问题
# 重新设置WrapOpenGL target，移除AGL依赖
if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES
        INTERFACE_LINK_LIBRARIES "-framework OpenGL"
    )
endif()

set(PROJECT_SOURCES
    main.cpp

    ui/notepad.cpp
    ui/notepad.h
    ui/codeeditor.cpp
    ui/codeeditor.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
)
