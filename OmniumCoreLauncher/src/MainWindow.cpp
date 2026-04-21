#include "MainWindow.h"

#include "ApiClient.h"
#include "AssetPaths.h"
#include "FlowLayout.h"
#include "GameCard.h"
#include "GameDetailsDialog.h"
#include "GameScanner.h"
#include "PlatformIcons.h"
#include "SoundManager.h"
#include "Updater.h"

#include <QAction>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
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
    connect(m_updater, &Updater::upToDate, this, [this](const QString& v){
        statusBar()->showMessage(tr("OmniumCore is up to date (v%1)").arg(v), 5000);
    });
    connect(m_updater, &Updater::checkFailed, this, [this](const QString& e){
        statusBar()->showMessage(tr("Update check failed: %1").arg(e), 5000);
    });

    buildUi();

    // Auto check for updates 2s after launch.
    QTimer::singleShot(2000, this, &MainWindow::onCheckForUpdates);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("CentralWidget"));
    setCentralWidget(central);

    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ----- Sidebar -----
    auto* sidebar = new QWidget(central);
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(220);
    buildSidebar(sidebar);
    root->addWidget(sidebar);

    // ----- Main area -----
    auto* main = new QWidget(central);
    main->setObjectName(QStringLiteral("MainArea"));
    auto* mainLayout = new QVBoxLayout(main);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* header = new QWidget(main);
    header->setObjectName(QStringLiteral("Header"));
    header->setFixedHeight(72);
    buildHeader(header);
    mainLayout->addWidget(header);

    // ----- Grid -----
    m_scroll = new QScrollArea(main);
    m_scroll->setObjectName(QStringLiteral("GridScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_grid = new QWidget(m_scroll);
    m_grid->setObjectName(QStringLiteral("GridContainer"));
    m_gridLayout = new FlowLayout(m_grid, 32, 24, 24);
    m_grid->setLayout(m_gridLayout);
    m_scroll->setWidget(m_grid);

    mainLayout->addWidget(m_scroll, 1);
    root->addWidget(main, 1);

    // ----- Status bar -----
    auto* sb = statusBar();
    sb->setObjectName(QStringLiteral("StatusBar"));
    m_libraryCount = new QLabel(tr("0 games"), sb);
    sb->addPermanentWidget(m_libraryCount);
    sb->showMessage(tr("Ready. Click \"Scan Library\" to discover installed games."));

    // ----- Menu bar -----
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("Scan Library"),       this, &MainWindow::onScanLibrary);
    fileMenu->addAction(tr("Add Custom Folder…"), this, &MainWindow::onAddCustomFolder);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), this, &QWidget::close);

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("Check for Updates"), this, &MainWindow::onCheckForUpdates);
    helpMenu->addAction(tr("About"), this, [this] {
        QMessageBox::about(this, tr("About OmniumCore"),
            tr("<b>OmniumCore Launcher</b><br>Version %1<br><br>"
               "Universal Game Launcher built with C++ &amp; Qt 6.<br>"
               "© OmniumCore.").arg(QStringLiteral(OMNIUM_VERSION)));
    });
}

void MainWindow::buildSidebar(QWidget* container) {
    auto* lay = new QVBoxLayout(container);
    lay->setContentsMargins(20, 24, 20, 24);
    lay->setSpacing(16);

    auto* logo = new QLabel(container);
    logo->setObjectName(QStringLiteral("Logo"));
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const QString logoPath = AssetPaths::resolve(QStringLiteral("images/logo.svg"));
    if (!logoPath.isEmpty()) {
        QPixmap pix(logoPath);
        if (!pix.isNull()) {
            logo->setPixmap(pix.scaledToHeight(48, Qt::SmoothTransformation));
        } else {
            // SVG: render it via QIcon at the desired pixel size
            logo->setPixmap(QIcon(logoPath).pixmap(QSize(180, 48)));
        }
    } else {
        logo->setTextFormat(Qt::RichText);
        logo->setText(QStringLiteral(
            "OMNIUM<br><span style='color:#7c5cff'>CORE</span>"));
    }
    lay->addWidget(logo);

    auto* scanBtn = new QPushButton(tr("⟳  Scan Library"), container);
    scanBtn->setObjectName(QStringLiteral("PrimaryButton"));
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::onScanLibrary);
    lay->addWidget(scanBtn);

    auto* addBtn = new QPushButton(tr("＋  Add Folder"), container);
    addBtn->setObjectName(QStringLiteral("GhostButton"));
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddCustomFolder);
    lay->addWidget(addBtn);

    auto* sep = new QFrame(container);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName(QStringLiteral("SidebarDivider"));
    lay->addWidget(sep);

    auto* platformsTitle = new QLabel(tr("PLATFORMS"), container);
    platformsTitle->setObjectName(QStringLiteral("SidebarSection"));
    lay->addWidget(platformsTitle);

    const QList<Platform> all = {
        Platform::Steam, Platform::EA, Platform::Epic, Platform::Microsoft,
        Platform::Ubisoft, Platform::GOG, Platform::Amazon, Platform::Riot
    };
    for (Platform p : all) {
        auto* row = new QLabel(container);
        row->setObjectName(QStringLiteral("PlatformRow"));
        row->setText(QStringLiteral("●  %1").arg(platformToString(p)));
        row->setStyleSheet(QStringLiteral("color:%1;").arg(PlatformIcons::badgeColorFor(p)));
        lay->addWidget(row);
    }

    lay->addStretch(1);

    auto* version = new QLabel(QStringLiteral("v%1").arg(QStringLiteral(OMNIUM_VERSION)), container);
    version->setObjectName(QStringLiteral("VersionLabel"));
    lay->addWidget(version);
}

void MainWindow::buildHeader(QWidget* container) {
    auto* lay = new QHBoxLayout(container);
    lay->setContentsMargins(28, 16, 28, 16);
    lay->setSpacing(16);

    auto* title = new QLabel(tr("Library"), container);
    title->setObjectName(QStringLiteral("PageTitle"));
    lay->addWidget(title);

    lay->addStretch(1);

    m_search = new QLineEdit(container);
    m_search->setObjectName(QStringLiteral("SearchBox"));
    m_search->setPlaceholderText(tr("Search games…"));
    m_search->setClearButtonEnabled(true);
    m_search->setFixedWidth(280);
    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    lay->addWidget(m_search);

    m_platformFilter = new QComboBox(container);
    m_platformFilter->setObjectName(QStringLiteral("PlatformCombo"));
    m_platformFilter->addItem(tr("All Platforms"), QVariant::fromValue<int>(-1));
    const QList<Platform> all = {
        Platform::Steam, Platform::EA, Platform::Epic, Platform::Microsoft,
        Platform::Ubisoft, Platform::GOG, Platform::Amazon, Platform::Riot
    };
    for (Platform p : all)
        m_platformFilter->addItem(platformToString(p), QVariant::fromValue<int>(int(p)));
    connect(m_platformFilter, &QComboBox::currentIndexChanged,
            this, &MainWindow::onPlatformFilterChanged);
    lay->addWidget(m_platformFilter);
}

void MainWindow::onScanLibrary() {
    statusBar()->showMessage(tr("Scanning installed games…"));

    auto* watcher = new QFutureWatcher<QVector<Game>>(this);
    connect(watcher, &QFutureWatcher<QVector<Game>>::finished, this, [this, watcher] {
        m_games = watcher->result();
        watcher->deleteLater();

        rebuildGrid();
        statusBar()->showMessage(tr("Scan complete — %1 games found.").arg(m_games.size()), 5000);
        m_libraryCount->setText(tr("%1 games").arg(m_games.size()));

        // Kick off metadata fetch for everything (rate-limited by the API itself).
        for (const Game& g : m_games)
            m_api->fetchMetadata(g);
    });
    watcher->setFuture(QtConcurrent::run([this] { return m_scanner->scan(); }));
}

void MainWindow::onAddCustomFolder() {
    const QString dir = QFileDialog::getExistingDirectory(this,
        tr("Select a folder containing game installs"));
    if (dir.isEmpty()) return;
    m_scanner->addExtraRoot(dir);
    statusBar()->showMessage(tr("Added folder: %1").arg(dir), 4000);
}

void MainWindow::rebuildGrid() {
    // Clear current grid widgets.
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

        // Show real cover if cached, otherwise the placeholder from assets/.
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
    auto* dlg = new GameDetailsDialog(game, this);
    if (!game.coverLocalPath.isEmpty())
        dlg->setCover(QPixmap(game.coverLocalPath));
    connect(dlg, &GameDetailsDialog::launchRequested, this, &MainWindow::onLaunchGame);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onLaunchGame(const Game& game) {
    if (game.executablePath.isEmpty()) {
        QMessageBox::warning(this, tr("Cannot launch"),
            tr("No executable was detected for %1.").arg(game.title));
        return;
    }
    if (!QProcess::startDetached(game.executablePath,
            QStringList(), QFileInfo(game.executablePath).absolutePath())) {
        QMessageBox::warning(this, tr("Launch failed"),
            tr("Could not start %1.").arg(game.title));
    } else {
        m_sounds->playLaunch();
        statusBar()->showMessage(tr("Launched %1").arg(game.title), 3000);
    }
}

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
        break;
    }
}

void MainWindow::onCoverReady(const QString& gameId, const QString& localPath) {
    for (auto& g : m_games) {
        if (g.id != gameId) continue;
        g.coverLocalPath = localPath;
        if (auto* card = m_cardsById.value(gameId, nullptr))
            card->setCover(QPixmap(localPath));
        break;
    }
}

void MainWindow::onCheckForUpdates() {
    statusBar()->showMessage(tr("Checking for updates…"), 2000);
    m_updater->checkForUpdates();
}

void MainWindow::onUpdateAvailable(const QString& tag,
                                   const QString& url,
                                   const QString& notes) {
    m_sounds->playNotification();
    QMessageBox box(this);
    box.setWindowTitle(tr("Update available"));
    box.setIcon(QMessageBox::Information);
    box.setText(tr("<b>OmniumCore %1</b> is available.").arg(tag));
    box.setInformativeText(notes.left(600));
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.button(QMessageBox::Ok)->setText(tr("Download"));
    if (box.exec() == QMessageBox::Ok && !url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

} // namespace omnium
