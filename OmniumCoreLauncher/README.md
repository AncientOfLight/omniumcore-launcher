# OmniumCore Launcher

A native Windows desktop application written in **C++17** with **Qt 6 Widgets** —
a universal launcher that scans your installed games across Steam, EA, Epic,
Microsoft Store / Xbox, Ubisoft, GOG, Amazon and Riot, and presents them in a
futuristic dark "Midnight UI" grid.

## Features

- **Library grid** — `QScrollArea` + custom `FlowLayout` of game cards.
- **Game details** — expandable synopsis, release date, genres, score, platform badge.
- **Native Windows scanning** — `WinAPI` registry walk + filesystem heuristics.
- **Metadata API** — `QNetworkAccessManager` ready for RAWG (default) or IGDB.
- **Auto-update** — compares `OMNIUM_VERSION` against the latest GitHub release.
- **One-click launch** — `QProcess::startDetached` on the detected `.exe`.
- **Midnight UI** — futuristic dark theme via QSS (`resources/styles/midnight.qss`).

## Project layout

```
OmniumCoreLauncher/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── MainWindow.{h,cpp}
│   ├── FlowLayout.{h,cpp}        # Qt official FlowLayout port
│   ├── GameCard.{h,cpp}          # Card widget for the grid
│   ├── GameDetailsDialog.{h,cpp} # Expandable details panel
│   ├── GameScanner.{h,cpp}       # WinAPI + filesystem scan
│   ├── ApiClient.{h,cpp}         # RAWG / IGDB HTTP client
│   ├── Updater.{h,cpp}           # GitHub release checker
│   ├── PlatformIcons.{h,cpp}
│   └── models/Game.h
└── resources/
    ├── resources.qrc
    ├── styles/midnight.qss
    └── icons/*.svg
```

## Build (Windows, native `.exe`)

### Requirements

- **Qt 6.5+** with the `Widgets` and `Network` modules
  (e.g. `C:\Qt\6.7.0\msvc2022_64`)
- **CMake 3.21+**
- **Visual Studio 2022** (MSVC) **or** MinGW shipped with Qt
- Windows 10/11 SDK

### Configure & build

```powershell
# From the project root
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2022_64"

cmake --build build --config Release

# Bundle Qt runtime DLLs alongside the .exe
C:\Qt\6.7.0\msvc2022_64\bin\windeployqt.exe build\Release\OmniumCoreLauncher.exe
```

The native binary is `build\Release\OmniumCoreLauncher.exe`.

### Or with Qt Creator

Open `CMakeLists.txt` directly — Qt Creator will configure the kit and build.

## Configuration

| Env var             | Purpose                                                          |
| ------------------- | ---------------------------------------------------------------- |
| `OMNIUM_RAWG_KEY`   | API key for [RAWG](https://rawg.io/apidocs) (free).              |

The auto-updater is wired to a GitHub repo. Change the slug in
`MainWindow::MainWindow()` or call `m_updater->setRepository("you/your-repo")`
to point it at your release feed.

## A note on the Replit preview

This is a **native Windows GUI**. Replit's runtime is Linux without a graphical
display server, so the app cannot be rendered in the workspace preview pane.
The project compiles cleanly on Linux too (Qt 6 + g++) — the WinAPI registry
scan is conditionally compiled — but to see real windows you must build it on
Windows or run it under Wine/VcXsrv on a machine that has a desktop session.
