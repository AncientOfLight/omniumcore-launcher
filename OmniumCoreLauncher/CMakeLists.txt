cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Widgets 
    Gui 
    Core 
    Concurrent 
    Network 
    Multimedia
)

# 1. Source discovery: Scan only the 'src' directory 
# to avoid including temporary or build files.
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# 2. Entry point check: Manually add main.cpp if it's in the root directory. 
# If already in 'src', CMake will handle duplicates.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
endif()

qt_add_executable(OmniumCoreLauncher ${SOURCES} ${HEADERS} resources/resources.qrc)

target_compile_definitions(OmniumCoreLauncher PRIVATE 
    OMNIUM_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(OmniumCoreLauncher PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core 
    Qt6::Concurrent 
    Qt6::Network 
    Qt6::Multimedia
)
