cmake_minimum_required(VERSION 3.16)
project(OmniumCoreLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Adjust CMAKE_PREFIX_PATH to your local Qt 6 install, e.g.
# -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2019_64"
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    Network
    Multimedia
    Svg
)

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

qt_add_executable(OmniumCoreLauncher ${PROJECT_SOURCES})

target_compile_definitions(OmniumCoreLauncher PRIVATE
    OMNIUM_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(OmniumCoreLauncher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    Qt6::Multimedia
    Qt6::Svg
)

# On Windows, build as a GUI app (no console window) and embed the app icon.
if(WIN32)
    set_target_properties(OmniumCoreLauncher PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Copy the assets/ folder next to the executable after every build so that
# images, icons and sound effects are found by AssetPaths at runtime.
add_custom_command(TARGET OmniumCoreLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:OmniumCoreLauncher>/assets"
)

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS OmniumCoreLauncher
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    DESTINATION assets
)

# -----------------------------------------------------------------------------
# windeployqt — bundle every Qt DLL, plugin and runtime the launcher needs
# -----------------------------------------------------------------------------
if(WIN32)
    # Locate windeployqt.exe (ships with every Qt MSVC install)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
              "${QT6_INSTALL_PREFIX}/bin"
        DOC "Path to Qt's windeployqt utility"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        # Run windeployqt right after the build so the .exe folder is ready
        # to launch (and ready to be packaged by CPack).
        add_custom_command(TARGET OmniumCoreLauncher POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    --compiler-runtime
                    "$<TARGET_FILE:OmniumCoreLauncher>"
            COMMENT "Running windeployqt to bundle Qt runtime dependencies"
        )

        # Mirror the same step at install time so packaged builds get the DLLs.
        install(CODE "
            execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --compiler-runtime
                --dir \"\${CMAKE_INSTALL_PREFIX}\"
                \"\${CMAKE_INSTALL_PREFIX}/OmniumCoreLauncher.exe\"
            )
        ")
    else()
        message(WARNING
            "windeployqt was not found. Qt DLLs will NOT be bundled. "
            "Pass -DCMAKE_PREFIX_PATH=<Qt install path> when configuring.")
    endif()
endif()

# -----------------------------------------------------------------------------
# CPack — produce a Windows installer (.exe) via NSIS
# -----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME              "OmniumCoreLauncher")
set(CPACK_PACKAGE_VENDOR            "OmniumCore")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "OmniumCore — Universal Game Launcher")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OmniumCoreLauncher")
set(CPACK_PACKAGE_EXECUTABLES       "OmniumCoreLauncher" "OmniumCore Launcher")
set(CPACK_CREATE_DESKTOP_LINKS      "OmniumCoreLauncher")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")

    # Resolve the launcher icon for the installer / shortcuts.
    set(_OMNIUM_NSIS_ICON
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.ico")
    if(EXISTS "${_OMNIUM_NSIS_ICON}")
        set(CPACK_NSIS_MUI_ICON           "${_OMNIUM_NSIS_ICON}")
        set(CPACK_NSIS_MUI_UNIICON        "${_OMNIUM_NSIS_ICON}")
        set(CPACK_NSIS_INSTALLED_ICON_NAME
            "OmniumCoreLauncher.exe")
    endif()

    set(CPACK_NSIS_DISPLAY_NAME       "OmniumCore Launcher")
    set(CPACK_NSIS_PACKAGE_NAME       "OmniumCore Launcher")
    set(CPACK_NSIS_HELP_LINK
        "https://github.com/OmniumCore/OmniumCoreLauncher")
    set(CPACK_NSIS_URL_INFO_ABOUT
        "https://github.com/OmniumCore/OmniumCoreLauncher")
    set(CPACK_NSIS_CONTACT            "support@omniumcore.local")

    # Create the Desktop shortcut on install and remove it on uninstall.
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH        OFF)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\OmniumCore Launcher.lnk' '$INSTDIR\\\\OmniumCoreLauncher.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\OmniumCore Launcher.lnk'")
endif()

include(CPack)
