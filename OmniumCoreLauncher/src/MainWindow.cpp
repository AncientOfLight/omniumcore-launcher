#include "MainWindow.h"

#include "ApiClient.h"
#include "AssetPaths.h"
#include "FlowLayout.h"
#include "GameCard.h"
#include "GameDetailsView.h"
#include "GameScanner.h"
#include "PlatformIcons.h"
#include "SoundManager.h"
#include "Updater.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QFutureWatcher>

namespace omnium {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("MainWindow"));
    setWindowTitle(QStringLiteral("OmniumCore — Universal Game Launcher"));

    m_scanner = new GameScanner(this);
    m_api     = new ApiClient(this);
    m_updater = new Updater(this);
    m_sounds  = new SoundManager(this);

    connect(m_api,     &ApiClient::gameMetadataReady, this, &MainWindow::onMetadataReady);
    connect(m_api,     &ApiClient::coverReady,        this, &MainWindow::onCoverReady);
    connect(m_updater, &Updater::updateAvailable,     this, &MainWindow::onUpdateAvailable);
    connect(m_updater, &Updater::upToDate,            this, &MainWindow::onUpToDate);
    connect(m_updater, &Updater::checkFailed,         this, &MainWindow::onUpdateCheckFailed);

    buildUi();

    // Hide the native menu bar — navigation lives in the in-app Menu button.
    menuBar()->hide();

    // Auto-check for updates 2s after launch.
    QTimer::singleShot(2000, this, &MainWindow::onCheckForUpdates);
}

MainWindow::~MainWindow() = default;

// =====================================================================
//  Top-level layout
// =====================================================================
void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("CentralWidget"));
    setCentralWidget(central);

    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ----- Sidebar -----
    m_sidebar = new QWidget(central);
    m_sidebar->setObjectName(QStringLiteral("Sidebar"));
    m_sidebar->setFixedWidth(220);
    buildSidebar();
    root->addWidget(m_sidebar);

    // ----- Main column (header + stacked pages + status bar) -----
    auto* mainCol = new QWidget(central);
    mainCol->setObjectName(QStringLiteral("MainArea"));
    auto* mainLayout = new QVBoxLayout(mainCol);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    auto* header = new QWidget(mainCol);
    header->setObjectName(QStringLiteral("Header"));
    header->setFixedHeight(72);
    {
        // Header is built later because it needs the menu wired up first.
    }
    mainLayout->addWidget(header);
    // Build the header contents in-place using the existing widget pointer.
    {
        auto* lay = new QHBoxLayout(header);
        lay->setContentsMargins(20, 14, 28, 14);
        lay->setSpacing(14);

        // Sidebar toggle
        m_sidebarToggle = new QToolButton(header);
        m_sidebarToggle->setObjectName(QStringLiteral("SidebarToggle"));
        m_sidebarToggle->setText(QStringLiteral("☰"));
        m_sidebarToggle->setToolTip(tr("Hide / show sidebar"));
        m_sidebarToggle->setCursor(Qt::PointingHandCursor);
        m_sidebarToggle->setFixedSize(36, 36);
        connect(m_sidebarToggle, &QToolButton::clicked,
                this, &MainWindow::toggleSidebar);
        lay->addWidget(m_sidebarToggle, 0, Qt::AlignVCenter);

        // Page title (kept generic; updated per page if desired)
        auto* title = new QLabel(tr("Library"), header);
        title->setObjectName(QStringLiteral("PageTitle"));
        lay->addWidget(title);

        lay->addStretch(1);

        // Search box (only meaningful on the Library page)
        m_search = new QLineEdit(header);
        m_search->setObjectName(QStringLiteral("SearchBox"));
        m_search->setPlaceholderText(tr("Search games…"));
        m_search->setClearButtonEnabled(true);
        m_search->setFixedWidth(280);
        connect(m_search, &QLineEdit::textChanged,
                this, &MainWindow::onSearchTextChanged);
        lay->addWidget(m_search);

        // Platform filter
        m_platformFilter = new QComboBox(header);
        m_platformFilter->setObjectName(QStringLiteral("PlatformCombo"));
        m_platformFilter->addItem(tr("All Platforms"), QVariant::fromValue<int>(-1));
        for (Platform p : allPlatforms())
            m_platformFilter->addItem(platformToString(p),
                                      QVariant::fromValue<int>(int(p)));
        connect(m_platformFilter,
                &QComboBox::currentIndexChanged,
                this, &MainWindow::onPlatformFilterChanged);
        lay->addWidget(m_platformFilter);

        // Single Menu button (replaces the old File / Help menus).
        buildMainMenu();
        m_menuButton = new QToolButton(header);
        m_menuButton->setObjectName(QStringLiteral("MenuButton"));
        m_menuButton->setText(QStringLiteral("Menu  ▾"));
        m_menuButton->setPopupMode(QToolButton::InstantPopup);
        m_menuButton->setCursor(Qt::PointingHandCursor);
        m_menuButton->setMenu(m_mainMenu);
        lay->addWidget(m_menuButton);
    }

    // ----- Update banner (in-page, above the stacked content) -----
    m_updateBanner = new QFrame(mainCol);
    m_updateBanner->setObjectName(QStringLiteral("UpdateBanner"));
    m_updateBanner->setVisible(false);
    {
        auto* bl = new QHBoxLayout(m_updateBanner);
        bl->setContentsMargins(20, 10, 20, 10);
        bl->setSpacing(12);
        m_updateBannerLabel = new QLabel(m_updateBanner);
        m_updateBannerLabel->setObjectName(QStringLiteral("UpdateBannerText"));
        bl->addWidget(m_updateBannerLabel, 1);
        m_updateBannerBtn = new QPushButton(tr("Download"), m_updateBanner);
        m_updateBannerBtn->setObjectName(QStringLiteral("PrimaryButton"));
        m_updateBannerBtn->setCursor(Qt::PointingHandCursor);
        connect(m_updateBannerBtn, &QPushButton::clicked, this, [this] {
            if (!m_updateDownloadUrl.isEmpty())
                QDesktopServices::openUrl(QUrl(m_updateDownloadUrl));
        });
        bl->addWidget(m_updateBannerBtn, 0, Qt::AlignRight);
    }
    mainLayout->addWidget(m_updateBanner);

    // ----- Stacked pages -----
    m_pages = new QStackedWidget(mainCol);
    m_pages->setObjectName(QStringLiteral("PageStack"));
    m_pages->insertWidget(int(Page::Library),  buildLibraryPage());

    m_detailsView = new GameDetailsView(mainCol);
    connect(m_detailsView, &GameDetailsView::backRequested, this, [this] {
        showPage(Page::Library);
    });
    connect(m_detailsView, &GameDetailsView::launchRequested,
            this, &MainWindow::onLaunchGame);
    m_pages->insertWidget(int(Page::Details),  m_detailsView);
    m_pages->insertWidget(int(Page::Settings), buildSettingsPage());
    m_pages->insertWidget(int(Page::About),    buildAboutPage());
    mainLayout->addWidget(m_pages, 1);

    root->addWidget(mainCol, 1);

    // ----- Status bar -----
    auto* sb = statusBar();
    sb->setObjectName(QStringLiteral("StatusBar"));
    m_libraryCount = new QLabel(tr("0 games"), sb);
    sb->addPermanentWidget(m_libraryCount);
    sb->showMessage(tr("Ready. Open the Menu and choose \"Scan Library\" to discover installed games."));

    showPage(Page::Library);
}

// =====================================================================
//  Sidebar
// =====================================================================
void MainWindow::buildSidebar() {
    auto* outer = new QVBoxLayout(m_sidebar);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    m_sidebarBody = new QWidget(m_sidebar);
    m_sidebarBody->setObjectName(QStringLiteral("SidebarBody"));
    auto* lay = new QVBoxLayout(m_sidebarBody);
    lay->setContentsMargins(20, 24, 20, 24);
    lay->setSpacing(16);

    // Logo
    auto* logo = new QLabel(m_sidebarBody);
    logo->setObjectName(QStringLiteral("Logo"));
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const QString logoPath = AssetPaths::resolve(QStringLiteral("images/logo.svg"));
    if (!logoPath.isEmpty()) {
        QPixmap pix(logoPath);
        if (!pix.isNull())
            logo->setPixmap(pix.scaledToHeight(48, Qt::SmoothTransformation));
        else
            logo->setPixmap(QIcon(logoPath).pixmap(QSize(180, 48)));
    } else {
        logo->setTextFormat(Qt::RichText);
        logo->setText(QStringLiteral(
            "OMNIUM<br><span style='color:#7c5cff'>CORE</span>"));
    }
    lay->addWidget(logo);

    // Quick scan / add folder buttons
    auto* scanBtn = new QPushButton(tr("⟳  Scan Library"), m_sidebarBody);
    scanBtn->setObjectName(QStringLiteral("PrimaryButton"));
    scanBtn->setCursor(Qt::PointingHandCursor);
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::onScanLibrary);
    lay->addWidget(scanBtn);

    auto* addBtn = new QPushButton(tr("＋  Add Folder"), m_sidebarBody);
    addBtn->setObjectName(QStringLiteral("GhostButton"));
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddCustomFolder);
    lay->addWidget(addBtn);

    auto* sep = new QFrame(m_sidebarBody);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName(QStringLiteral("SidebarDivider"));
    lay->addWidget(sep);

    auto* platformsTitle = new QLabel(tr("PLATFORMS"), m_sidebarBody);
    platformsTitle->setObjectName(QStringLiteral("SidebarSection"));
    lay->addWidget(platformsTitle);

    // Render every known platform (including the new "Others" bucket).
    for (Platform p : allPlatforms()) {
        auto* row = new QLabel(m_sidebarBody);
        row->setObjectName(QStringLiteral("PlatformRow"));
        row->setText(QStringLiteral("●  %1").arg(platformToString(p)));
        row->setStyleSheet(QStringLiteral("color:%1;")
                               .arg(PlatformIcons::badgeColorFor(p)));
        lay->addWidget(row);
    }

    lay->addStretch(1);

    auto* version = new QLabel(QStringLiteral("v%1").arg(QStringLiteral(OMNIUM_VERSION)),
                               m_sidebarBody);
    version->setObjectName(QStringLiteral("VersionLabel"));
    lay->addWidget(version);

    outer->addWidget(m_sidebarBody, 1);
}

void MainWindow::toggleSidebar() {
    m_sidebarCollapsed = !m_sidebarCollapsed;
    if (m_sidebarCollapsed) {
        m_sidebar->setFixedWidth(0);
    } else {
        m_sidebar->setFixedWidth(220);
    }
}

// =====================================================================
//  Header / unified Menu
// =====================================================================
void MainWindow::buildHeader() { /* inlined inside buildUi() */ }

void MainWindow::buildMainMenu() {
    m_mainMenu = new QMenu(this);
    m_mainMenu->setObjectName(QStringLiteral("MainMenu"));

    m_mainMenu->addAction(tr("Settings"),          this, &MainWindow::onOpenSettings);
    m_mainMenu->addAction(tr("Scan Library"),      this, &MainWindow::onScanLibrary);
    m_mainMenu->addAction(tr("Check for Updates"), this, &MainWindow::onCheckForUpdates);
    m_mainMenu->addSeparator();
    m_mainMenu->addAction(tr("About"),             this, &MainWindow::onOpenAbout);
}

// =====================================================================
//  Page builders
// =====================================================================
QWidget* MainWindow::buildLibraryPage() {
    auto* page = new QWidget(this);
    page->setObjectName(QStringLiteral("LibraryPage"));

    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_scroll = new QScrollArea(page);
    m_scroll->setObjectName(QStringLiteral("GridScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_grid = new QWidget(m_scroll);
    m_grid->setObjectName(QStringLiteral("GridContainer"));
    m_gridLayout = new FlowLayout(m_grid, 32, 24, 24);
    m_grid->setLayout(m_gridLayout);
    m_scroll->setWidget(m_grid);

    lay->addWidget(m_scroll, 1);
    return page;
}

QWidget* MainWindow::buildSettingsPage() {
    auto* page = new QWidget(this);
    page->setObjectName(QStringLiteral("SettingsPage"));

    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 32, 40, 32);
    lay->setSpacing(20);

    auto* title = new QLabel(tr("Settings"), page);
    title->setObjectName(QStringLiteral("SectionHeading"));
    lay->addWidget(title);

    // ----- Sound -----
    auto* soundCard = new QFrame(page);
    soundCard->setObjectName(QStringLiteral("SettingsCard"));
    auto* sl = new QVBoxLayout(soundCard);
    sl->setContentsMargins(20, 16, 20, 16);
    sl->setSpacing(8);
    auto* soundTitle = new QLabel(tr("Sound"), soundCard);
    soundTitle->setObjectName(QStringLiteral("SettingsCardTitle"));
    sl->addWidget(soundTitle);
    m_muteCheckbox = new QCheckBox(tr("Mute interface sound effects"), soundCard);
    connect(m_muteCheckbox, &QCheckBox::toggled, this, [this](bool muted) {
        if (m_sounds) m_sounds->setMuted(muted);
    });
    sl->addWidget(m_muteCheckbox);
    lay->addWidget(soundCard);

    // ----- Custom scan folders -----
    auto* foldersCard = new QFrame(page);
    foldersCard->setObjectName(QStringLiteral("SettingsCard"));
    auto* fl = new QVBoxLayout(foldersCard);
    fl->setContentsMargins(20, 16, 20, 16);
    fl->setSpacing(8);
    auto* foldersTitle = new QLabel(tr("Custom scan folders"), foldersCard);
    foldersTitle->setObjectName(QStringLiteral("SettingsCardTitle"));
    fl->addWidget(foldersTitle);
    auto* foldersHint = new QLabel(
        tr("Folders added here are scanned in addition to the built-in launcher paths."),
        foldersCard);
    foldersHint->setObjectName(QStringLiteral("SettingsHint"));
    foldersHint->setWordWrap(true);
    fl->addWidget(foldersHint);

    m_customFoldersList = new QListWidget(foldersCard);
    m_customFoldersList->setObjectName(QStringLiteral("FoldersList"));
    m_customFoldersList->setMinimumHeight(140);
    fl->addWidget(m_customFoldersList);

    auto* addRow = new QHBoxLayout;
    auto* addFolderBtn = new QPushButton(tr("＋  Add folder"), foldersCard);
    addFolderBtn->setObjectName(QStringLiteral("GhostButton"));
    addFolderBtn->setCursor(Qt::PointingHandCursor);
    connect(addFolderBtn, &QPushButton::clicked, this, &MainWindow::onAddCustomFolder);
    addRow->addWidget(addFolderBtn);
    addRow->addStretch(1);
    fl->addLayout(addRow);
    lay->addWidget(foldersCard);

    // ----- API key status -----
    auto* apiCard = new QFrame(page);
    apiCard->setObjectName(QStringLiteral("SettingsCard"));
    auto* al = new QVBoxLayout(apiCard);
    al->setContentsMargins(20, 16, 20, 16);
    al->setSpacing(8);
    auto* apiTitle = new QLabel(tr("Metadata provider"), apiCard);
    apiTitle->setObjectName(QStringLiteral("SettingsCardTitle"));
    al->addWidget(apiTitle);
    const bool keySet = !qgetenv("OMNIUM_RAWG_KEY").isEmpty();
    m_apiKeyStatus = new QLabel(
        keySet ? tr("RAWG API key detected — metadata and covers will be fetched.")
               : tr("No RAWG API key detected. Set the OMNIUM_RAWG_KEY environment "
                    "variable to enable cover and synopsis lookups."),
        apiCard);
    m_apiKeyStatus->setObjectName(QStringLiteral("SettingsHint"));
    m_apiKeyStatus->setWordWrap(true);
    al->addWidget(m_apiKeyStatus);
    lay->addWidget(apiCard);

    lay->addStretch(1);
    return page;
}

QWidget* MainWindow::buildAboutPage() {
    auto* page = new QWidget(this);
    page->setObjectName(QStringLiteral("AboutPage"));

    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 32, 40, 32);
    lay->setSpacing(18);
    lay->setAlignment(Qt::AlignTop);

    auto* title = new QLabel(tr("About OmniumCore"), page);
    title->setObjectName(QStringLiteral("SectionHeading"));
    lay->addWidget(title);

    auto* card = new QFrame(page);
    card->setObjectName(QStringLiteral("SettingsCard"));
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(24, 20, 24, 20);
    cl->setSpacing(10);

    auto* logo = new QLabel(card);
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const QString logoPath = AssetPaths::resolve(QStringLiteral("images/logo.svg"));
    if (!logoPath.isEmpty())
        logo->setPixmap(QIcon(logoPath).pixmap(QSize(220, 60)));
    cl->addWidget(logo);

    auto* version = new QLabel(
        tr("Version <b>%1</b>").arg(QStringLiteral(OMNIUM_VERSION)), card);
    version->setObjectName(QStringLiteral("SettingsCardTitle"));
    cl->addWidget(version);

    auto* description = new QLabel(
        tr("Universal Game Launcher built with C++ and Qt 6.<br>"
           "Aggregates Steam, EA, Epic, Microsoft Store, Ubisoft, GOG, "
           "Amazon, Riot and any other Windows installs into a single, "
           "futuristic library.<br><br>© OmniumCore. All rights reserved."),
        card);
    description->setObjectName(QStringLiteral("SettingsHint"));
    description->setTextFormat(Qt::RichText);
    description->setWordWrap(true);
    cl->addWidget(description);

    lay->addWidget(card);
    return page;
}

void MainWindow::showPage(Page page) {
    if (!m_pages) return;
    m_pages->setCurrentIndex(int(page));

    // Hide library-only controls when not on the library page.
    const bool onLibrary = (page == Page::Library);
    if (m_search)         m_search->setVisible(onLibrary);
    if (m_platformFilter) m_platformFilter->setVisible(onLibrary);
}

// =====================================================================
//  Menu actions
// =====================================================================
void MainWindow::onOpenSettings() { showPage(Page::Settings); }
void MainWindow::onOpenAbout()    { showPage(Page::About); }

void MainWindow::onScanLibrary() {
    showPage(Page::Library);
    statusBar()->showMessage(tr("Scanning installed games…"));

    auto* watcher = new QFutureWatcher<QVector<Game>>(this);
    connect(watcher, &QFutureWatcher<QVector<Game>>::finished, this, [this, watcher] {
        m_games = watcher->result();
        watcher->deleteLater();

        rebuildGrid();
        statusBar()->showMessage(tr("Scan complete — %1 games found.")
                                     .arg(m_games.size()), 5000);
        m_libraryCount->setText(tr("%1 games").arg(m_games.size()));

        // Kick off metadata fetch for everything.
        for (const Game& g : m_games)
            m_api->fetchMetadata(g);
    });
    watcher->setFuture(QtConcurrent::run([this] { return m_scanner->scan(); }));
}

void MainWindow::onCheckForUpdates() {
    statusBar()->showMessage(tr("Checking for updates…"), 2000);
    m_updater->checkForUpdates();
}

// =====================================================================
//  Library-specific slots
// =====================================================================
void MainWindow::onAddCustomFolder() {
    const QString dir = QFileDialog::getExistingDirectory(this,
        tr("Select a folder containing game installs"));
    if (dir.isEmpty()) return;
    m_scanner->addExtraRoot(dir);
    if (m_customFoldersList)
        m_customFoldersList->addItem(dir);
    statusBar()->showMessage(tr("Added folder: %1").arg(dir), 4000);
}

void MainWindow::rebuildGrid() {
    QLayoutItem* item;
    while ((item = m_gridLayout->takeAt(0))) {
        if (auto* w = item->widget()) w->deleteLater();
        delete item;
    }
    m_cardsById.clear();

    const QString search = m_search ? m_search->text().trimmed().toLower() : QString();
    const int platformFilter = m_platformFilter
        ? m_platformFilter->currentData().toInt()
        : -1;

    for (const Game& g : std::as_const(m_games)) {
        if (!search.isEmpty() && !g.title.toLower().contains(search))
            continue;
        if (platformFilter >= 0 && int(g.platform) != platformFilter)
            continue;

        auto* card = new GameCard(g, m_grid);
        connect(card, &GameCard::clicked,         this, &MainWindow::onCardClicked);
        connect(card, &GameCard::launchRequested, this, &MainWindow::onLaunchGame);

        if (!g.coverLocalPath.isEmpty()) {
            card->setCover(QPixmap(g.coverLocalPath));
        } else {
            const QString ph = AssetPaths::resolve(
                QStringLiteral("images/cover_placeholder.svg"));
            if (!ph.isEmpty())
                card->setCover(QIcon(ph).pixmap(QSize(240, 320)));
        }

        m_gridLayout->addWidget(card);
        m_cardsById.insert(g.id, card);
    }
}

void MainWindow::onSearchTextChanged(const QString&) { rebuildGrid(); }
void MainWindow::onPlatformFilterChanged(int)        { rebuildGrid(); }

void MainWindow::onCardClicked(const Game& game) {
    m_sounds->playClick();
    if (!m_detailsView) return;
    m_detailsView->setGame(game);
    if (!game.coverLocalPath.isEmpty()) {
        m_detailsView->setCover(QPixmap(game.coverLocalPath));
    } else {
        const QString ph = AssetPaths::resolve(
            QStringLiteral("images/cover_placeholder.svg"));
        if (!ph.isEmpty())
            m_detailsView->setCover(QIcon(ph).pixmap(QSize(320, 440)));
    }
    showPage(Page::Details);
}

void MainWindow::onLaunchGame(const Game& game) {
    if (game.executablePath.isEmpty()) {
        statusBar()->showMessage(
            tr("No executable detected for %1.").arg(game.title), 5000);
        return;
    }
    if (!QProcess::startDetached(game.executablePath,
            QStringList(), QFileInfo(game.executablePath).absolutePath())) {
        statusBar()->showMessage(tr("Could not start %1.").arg(game.title), 5000);
    } else {
        m_sounds->playLaunch();
        statusBar()->showMessage(tr("Launched %1").arg(game.title), 3000);
    }
}

// =====================================================================
//  Async network results
// =====================================================================
void MainWindow::onMetadataReady(const QString& gameId, const Game& enriched) {
    for (auto& g : m_games) {
        if (g.id != gameId) continue;
        g.coverUrl    = enriched.coverUrl;
        g.synopsis    = enriched.synopsis;
        g.releaseDate = enriched.releaseDate;
        g.genres      = enriched.genres;
        g.rating      = enriched.rating;
        if (auto* card = m_cardsById.value(gameId, nullptr))
            card->setGame(g);
        // If the user is currently viewing this game, refresh the details page.
        if (m_detailsView && m_pages
            && m_pages->currentIndex() == int(Page::Details)
            && m_detailsView->game().id == gameId) {
            m_detailsView->setGame(g);
        }
        break;
    }
}

void MainWindow::onCoverReady(const QString& gameId, const QString& localPath) {
    for (auto& g : m_games) {
        if (g.id != gameId) continue;
        g.coverLocalPath = localPath;
        if (auto* card = m_cardsById.value(gameId, nullptr))
            card->setCover(QPixmap(localPath));
        if (m_detailsView && m_pages
            && m_pages->currentIndex() == int(Page::Details)
            && m_detailsView->game().id == gameId) {
            m_detailsView->setCover(QPixmap(localPath));
        }
        break;
    }
}

// =====================================================================
//  Update banner
// =====================================================================
void MainWindow::refreshUpdateBanner(const QString& text, bool actionable,
                                     const QString& url) {
    m_updateDownloadUrl = url;
    if (m_updateBannerLabel) m_updateBannerLabel->setText(text);
    if (m_updateBannerBtn)   m_updateBannerBtn->setVisible(actionable);
    if (m_updateBanner)      m_updateBanner->setVisible(true);
}

void MainWindow::onUpdateAvailable(const QString& tag,
                                   const QString& url,
                                   const QString& notes) {
    Q_UNUSED(notes);
    m_sounds->playNotification();
    refreshUpdateBanner(
        tr("OmniumCore %1 is available.").arg(tag),
        !url.isEmpty(),
        url);
}

void MainWindow::onUpToDate(const QString& currentVersion) {
    if (m_updateBanner) m_updateBanner->setVisible(false);
    statusBar()->showMessage(
        tr("OmniumCore is up to date (v%1)").arg(currentVersion), 5000);
}

void MainWindow::onUpdateCheckFailed(const QString& error) {
    statusBar()->showMessage(tr("Update check failed: %1").arg(error), 5000);
}

} // namespace omnium
