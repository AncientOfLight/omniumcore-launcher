cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Agregamos Network y Multimedia que son los que pedía el error
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Concurrent Network Multimedia)

file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

qt_add_executable(OmniumCoreLauncher ${SOURCES} ${HEADERS} resources/resources.qrc)

target_link_libraries(OmniumCoreLauncher PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Concurrent Qt6::Network Qt6::Multimedia)
