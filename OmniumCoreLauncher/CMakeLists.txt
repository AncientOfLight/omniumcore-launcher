cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscamos Qt6 y sus módulos
find_package(Qt6 REQUIRED COMPONENTS 
    Widgets 
    Gui 
    Core 
    Concurrent 
    Network 
    Multimedia
)

# Buscamos TODOS los archivos .cpp y .h dentro de src y sus subcarpetas
file(GLOB_RECURSE SOURCES "src/*.cpp" "*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "*.h")

# Creamos el ejecutable
qt_add_executable(OmniumCoreLauncher ${SOURCES} ${HEADERS} resources/resources.qrc)

# Arreglamos el error de OMNIUM_VERSION
target_compile_definitions(OmniumCoreLauncher PRIVATE 
    OMNIUM_VERSION="${PROJECT_VERSION}"
)

# Enlazamos las librerías
target_link_libraries(OmniumCoreLauncher PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core 
    Qt6::Concurrent 
    Qt6::Network 
    Qt6::Multimedia
)
