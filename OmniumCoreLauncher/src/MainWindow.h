#pragma once

#include <QMainWindow>
#include <QHash>

#include "models/Game.h"

class QLineEdit;
class QComboBox;
class QScrollArea;
class QStatusBar;
class QPushButton;
class QLabel;

namespace omnium {

class FlowLayout;
class GameCard;
class GameScanner;
class ApiClient;
class Updater;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onScanLibrary();
    void onAddCustomFolder();
    void onSearchTextChanged(const QString& text);
    void onPlatformFilterChanged(int index);
    void onCardClicked(const omnium::Game& game);
    void onLaunchGame(const omnium::Game& game);
    void onMetadataReady(const QString& gameId, const omnium::Game& enriched);
    void onCoverReady(const QString& gameId, const QString& localPath);
    void onCheckForUpdates();
    void onUpdateAvailable(const QString& tag, const QString& url, const QString& notes);

private:
    void buildUi();
    void buildSidebar(QWidget* container);
    void buildHeader(QWidget* container);
    void rebuildGrid();

    GameScanner*  m_scanner = nullptr;
    ApiClient*    m_api     = nullptr;
    Updater*      m_updater = nullptr;
    class SoundManager* m_sounds = nullptr;

    // UI
    QWidget*     m_grid       = nullptr;
    FlowLayout*  m_gridLayout = nullptr;
    QScrollArea* m_scroll     = nullptr;
    QLineEdit*   m_search     = nullptr;
    QComboBox*   m_platformFilter = nullptr;
    QLabel*      m_libraryCount   = nullptr;

    // Data
    QVector<Game>             m_games;
    QHash<QString, GameCard*> m_cardsById;
};

} // namespace omnium
