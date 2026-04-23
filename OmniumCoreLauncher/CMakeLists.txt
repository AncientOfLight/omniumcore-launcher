cmake_minimum_required(VERSION 3.21)

project(OmniumCoreLauncher
    VERSION 0.1.0
    DESCRIPTION "OmniumCore Universal Game Launcher"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Adjust CMAKE_PREFIX_PATH to your local Qt 6 install, e.g.
# -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2019_64"
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Multimedia Svg)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/FlowLayout.cpp
    src/FlowLayout.h
    src/GameCard.cpp
    src/GameCard.h
    src/GameDetailsView.cpp
    src/GameDetailsView.h
    src/GameScanner.cpp
    src/GameScanner.h
    src/ApiClient.cpp
    src/ApiClient.h
    src/Updater.cpp
    src/Updater.h
    src/PlatformIcons.cpp
    src/PlatformIcons.h
    src/AssetPaths.cpp
    src/AssetPaths.h
    src/SoundManager.cpp
    src/SoundManager.h
    src/models/Game.h
    resources/resources.qrc
)

if(WIN32)
    qt_add_executable(OmniumCoreLauncher WIN32 ${PROJECT_SOURCES})
else()
    qt_add_executable(OmniumCoreLauncher ${PROJECT_SOURCES})
endif()

target_link_libraries(OmniumCoreLauncher
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::Svg
)

if(WIN32)
    # Used by GameScanner for native filesystem walking and registry lookups
    target_link_libraries(OmniumCoreLauncher PRIVATE Advapi32 Shell32)
    target_compile_definitions(OmniumCoreLauncher PRIVATE
        UNICODE
        _UNICODE
        OMNIUM_VERSION="${PROJECT_VERSION}"
    )
else()
    target_compile_definitions(OmniumCoreLauncher PRIVATE
        OMNIUM_VERSION="${PROJECT_VERSION}"
    )
endif()

set_target_properties(OmniumCoreLauncher PROPERTIES
    OUTPUT_NAME "OmniumCoreLauncher"
)

install(TARGETS OmniumCoreLauncher
    RUNTIME DESTINATION bin
)

# Ship the assets folder next to the .exe so users can swap files
# (logo, sounds) without recompiling.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
        DESTINATION bin
)

# Also mirror assets next to the dev build so the running .exe finds them.
add_custom_command(TARGET OmniumCoreLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:OmniumCoreLauncher>/assets
)
