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

# 1. Aquí está el cambio: solo buscamos dentro de la carpeta 'src' 
# para no agarrar archivos basura de carpetas temporales.
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# 2. Si tu main.cpp está afuera de 'src', lo añadimos manualmente. 
# Si ya está dentro de 'src', CMake lo ignorará si es un duplicado.
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
