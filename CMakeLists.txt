cmake_minimum_required(VERSION 3.1.0)

project(entity-block)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)

add_executable(entity-block
    entityblock.cpp
    main.cpp
)

target_link_libraries(entity-block Qt5::Widgets Qt5::Svg)

install(TARGETS entity-block 
    RUNTIME DESTINATION bin)
