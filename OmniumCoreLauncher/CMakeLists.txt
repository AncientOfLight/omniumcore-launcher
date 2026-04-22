cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 1. Quita "OPTIONAL_COMPONENTS". Si faltan, es mejor que CMake se detenga aquí
# y no intente compilar algo que va a fallar.
find_package(Qt6 REQUIRED COMPONENTS 
    Widgets 
    Gui 
    Core 
    Concurrent 
    Network 
    Multimedia
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
# Es mejor no incluir main.cpp en el GLOB si ya lo tienes en la raíz o listarlo aparte
list(APPEND SOURCES "main.cpp") 

file(GLOB_RECURSE HEADERS "src/*.h")

qt_add_executable(OmniumCoreLauncher ${SOURCES} ${HEADERS} resources/resources.qrc)

# 2. Define la versión aquí para que ApiClient.cpp y Updater.cpp no den error C2065
target_compile_definitions(OmniumCoreLauncher PRIVATE 
    OMNIUM_VERSION="${PROJECT_VERSION}"
)

# 3. Enlaza todas las librerías necesarias
target_link_libraries(OmniumCoreLauncher PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core 
    Qt6::Concurrent 
    Qt6::Network 
    Qt6::Multimedia
)
