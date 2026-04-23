#include "GameDetailsView.h"
#include "PlatformIcons.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>

namespace omnium {

GameDetailsView::GameDetailsView(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("GameDetailsView"));
    buildUi();
}

void GameDetailsView::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);

    // ----- Top bar with the Back button -----
    auto* topBar = new QHBoxLayout;
    topBar->setSpacing(12);

    m_backBtn = new QPushButton(QStringLiteral("←  Back to library"), this);
    m_backBtn->setObjectName(QStringLiteral("GhostButton"));
    m_backBtn->setCursor(Qt::PointingHandCursor);
    connect(m_backBtn, &QPushButton::clicked, this, &GameDetailsView::backRequested);
    topBar->addWidget(m_backBtn, 0, Qt::AlignLeft);
    topBar->addStretch(1);
    root->addLayout(topBar);

    // ----- Body: cover on the left, info column on the right -----
    auto* body = new QHBoxLayout;
    body->setSpacing(24);

    m_cover = new QLabel(this);
    m_cover->setObjectName(QStringLiteral("DetailsCover"));
    m_cover->setFixedSize(320, 440);
    m_cover->setAlignment(Qt::AlignCenter);
    m_cover->setText(QStringLiteral("No cover"));
    body->addWidget(m_cover, 0, Qt::AlignTop);

    auto* col = new QVBoxLayout;
    col->setSpacing(14);
    body->addLayout(col, 1);

    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("DetailsTitle"));
    m_title->setWordWrap(true);
    col->addWidget(m_title);

    auto* badgeRow = new QHBoxLayout;
    badgeRow->setSpacing(8);
    m_platform = new QLabel(this);
    m_platform->setObjectName(QStringLiteral("PlatformBadge"));
    m_platform->setAlignment(Qt::AlignCenter);
    badgeRow->addWidget(m_platform, 0, Qt::AlignLeft);
    badgeRow->addStretch(1);
    col->addLayout(badgeRow);

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(8);

    auto makeKey = [this](const QString& s) {
        auto* l = new QLabel(s, this);
        l->setObjectName(QStringLiteral("MetaKey"));
        return l;
    };
    auto makeVal = [this]() {
        auto* l = new QLabel(this);
        l->setObjectName(QStringLiteral("MetaValue"));
        l->setWordWrap(true);
        return l;
    };

    m_release = makeVal();
    m_genres  = makeVal();
    m_rating  = makeVal();

    grid->addWidget(makeKey(QStringLiteral("Release date")), 0, 0);
    grid->addWidget(m_release,                                0, 1);
    grid->addWidget(makeKey(QStringLiteral("Genres")),        1, 0);
    grid->addWidget(m_genres,                                 1, 1);
    grid->addWidget(makeKey(QStringLiteral("Rating")),        2, 0);
    grid->addWidget(m_rating,                                 2, 1);
    col->addLayout(grid);

    // ----- Collapsible synopsis -----
    m_synopsisToggle = new QToolButton(this);
    m_synopsisToggle->setObjectName(QStringLiteral("SynopsisToggle"));
    m_synopsisToggle->setText(QStringLiteral("▾  Synopsis"));
    m_synopsisToggle->setCheckable(true);
    m_synopsisToggle->setChecked(true);
    m_synopsisToggle->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_synopsisToggle->setCursor(Qt::PointingHandCursor);
    col->addWidget(m_synopsisToggle, 0, Qt::AlignLeft);

    m_synopsisBox = new QFrame(this);
    m_synopsisBox->setObjectName(QStringLiteral("SynopsisBox"));
    auto* sbLay = new QVBoxLayout(m_synopsisBox);
    sbLay->setContentsMargins(0, 0, 0, 0);
    m_synopsis = new QTextBrowser(m_synopsisBox);
    m_synopsis->setObjectName(QStringLiteral("SynopsisText"));
    m_synopsis->setOpenExternalLinks(true);
    sbLay->addWidget(m_synopsis);
    col->addWidget(m_synopsisBox, 1);

    connect(m_synopsisToggle, &QToolButton::toggled, this, [this](bool open) {
        m_synopsisBox->setVisible(open);
        m_synopsisToggle->setText(open ? QStringLiteral("▾  Synopsis")
                                       : QStringLiteral("▸  Synopsis"));
    });

    // ----- Action bar -----
    auto* actions = new QHBoxLayout;
    actions->addStretch(1);
    m_launchBtn = new QPushButton(QStringLiteral("▶  Play"), this);
    m_launchBtn->setObjectName(QStringLiteral("PrimaryButton"));
    m_launchBtn->setMinimumHeight(40);
    m_launchBtn->setCursor(Qt::PointingHandCursor);
    actions->addWidget(m_launchBtn);
    col->addLayout(actions);

    connect(m_launchBtn, &QPushButton::clicked, this, [this] {
        emit launchRequested(m_game);
    });

    root->addLayout(body, 1);
}

void GameDetailsView::setGame(const Game& g) {
    m_game = g;
    applyGameData();
}

void GameDetailsView::applyGameData() {
    m_title->setText(m_game.title);
    m_platform->setText(QStringLiteral("  %1  ").arg(platformToString(m_game.platform)));
    m_platform->setStyleSheet(
        QStringLiteral("background:%1; color:white; padding:4px 12px; border-radius:10px;")
            .arg(PlatformIcons::badgeColorFor(m_game.platform)));

    m_release->setText(m_game.releaseDate.isValid()
                       ? m_game.releaseDate.toString(QStringLiteral("MMMM d, yyyy"))
                       : QStringLiteral("Unknown"));
    m_genres->setText(m_game.genres.isEmpty()
                      ? QStringLiteral("—")
                      : m_game.genres.join(QStringLiteral(", ")));
    m_rating->setText(m_game.rating > 0
                      ? QStringLiteral("★ %1 / 10").arg(m_game.rating, 0, 'f', 1)
                      : QStringLiteral("Not rated"));
    m_synopsis->setPlainText(m_game.synopsis.isEmpty()
                             ? QStringLiteral("No synopsis available yet. Connect a RAWG API "
                                              "key (OMNIUM_RAWG_KEY) to fetch metadata.")
                             : m_game.synopsis);
}

void GameDetailsView::setCover(const QPixmap& pix) {
    if (pix.isNull() || !m_cover) return;
    m_cover->setPixmap(pix.scaled(m_cover->size(),
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));
}

} // namespace omnium
