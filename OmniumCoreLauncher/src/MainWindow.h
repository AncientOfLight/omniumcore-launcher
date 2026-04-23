#pragma once

#include <QMainWindow>
#include <QHash>

#include "models/Game.h"

class QLineEdit;
class QComboBox;
class QScrollArea;
class QStackedWidget;
class QPushButton;
class QToolButton;
class QLabel;
class QMenu;
class QFrame;
class QCheckBox;
class QListWidget;

namespace omnium {

class FlowLayout;
class GameCard;
class GameScanner;
class ApiClient;
class Updater;
class SoundManager;
class GameDetailsView;

// Top-level pages displayed inside the main window's QStackedWidget.
// Everything stays inside a single window — no popups, no dialogs.
enum class Page : int {
    Library  = 0,
    Details  = 1,
    Settings = 2,
    About    = 3
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // ---- Menu actions ----
    void onOpenSettings();
    void onOpenAbout();
    void onScanLibrary();
    void onCheckForUpdates();

    // ---- Library ----
    void onAddCustomFolder();
    void onSearchTextChanged(const QString& text);
    void onPlatformFilterChanged(int index);
    void onCardClicked(const omnium::Game& game);
    void onLaunchGame(const omnium::Game& game);

    // ---- Async results ----
    void onMetadataReady(const QString& gameId, const omnium::Game& enriched);
    void onCoverReady(const QString& gameId, const QString& localPath);
    void onUpdateAvailable(const QString& tag, const QString& url, const QString& notes);
    void onUpToDate(const QString& currentVersion);
    void onUpdateCheckFailed(const QString& error);

    // ---- Sidebar collapse ----
    void toggleSidebar();

private:
    void buildUi();
    void buildSidebar();
    void buildHeader();
    void buildMainMenu();
    QWidget* buildLibraryPage();
    QWidget* buildSettingsPage();
    QWidget* buildAboutPage();

    void rebuildGrid();
    void showPage(Page page);
    void refreshUpdateBanner(const QString& text, bool actionable, const QString& url);

    // Core services ----
    GameScanner*  m_scanner = nullptr;
    ApiClient*    m_api     = nullptr;
    Updater*      m_updater = nullptr;
    SoundManager* m_sounds  = nullptr;

    // Layout ----
    QWidget*        m_sidebar      = nullptr;
    QWidget*        m_sidebarBody  = nullptr; // inner content (hidden when collapsed)
    QToolButton*    m_sidebarToggle = nullptr;
    bool            m_sidebarCollapsed = false;

    QStackedWidget* m_pages       = nullptr;
    QFrame*         m_updateBanner = nullptr;
    QLabel*         m_updateBannerLabel = nullptr;
    QPushButton*    m_updateBannerBtn   = nullptr;
    QString         m_updateDownloadUrl;

    // Library page widgets ----
    QWidget*     m_grid           = nullptr;
    FlowLayout*  m_gridLayout     = nullptr;
    QScrollArea* m_scroll         = nullptr;
    QLineEdit*   m_search         = nullptr;
    QComboBox*   m_platformFilter = nullptr;
    QLabel*      m_libraryCount   = nullptr;

    // Header widgets ----
    QToolButton* m_menuButton = nullptr;
    QMenu*       m_mainMenu   = nullptr;

    // Details page ----
    GameDetailsView* m_detailsView = nullptr;

    // Settings page widgets ----
    QCheckBox*   m_muteCheckbox     = nullptr;
    QListWidget* m_customFoldersList = nullptr;
    QLabel*      m_apiKeyStatus     = nullptr;

    // Data ----
    QVector<Game>             m_games;
    QHash<QString, GameCard*> m_cardsById;
};

} // namespace omnium
