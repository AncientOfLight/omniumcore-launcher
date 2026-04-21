cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Solo pedimos lo básico para que el .exe nazca ya
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Concurrent)

file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

qt_add_executable(OmniumCoreLauncher ${SOURCES} ${HEADERS} resources/resources.qrc)

target_link_libraries(OmniumCoreLauncher PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Concurrent)
