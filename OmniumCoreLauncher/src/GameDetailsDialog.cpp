#include "GameDetailsDialog.h"
#include "PlatformIcons.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>

namespace omnium {

GameDetailsDialog::GameDetailsDialog(const Game& game, QWidget* parent)
    : QDialog(parent), m_game(game) {
    setObjectName(QStringLiteral("GameDetailsDialog"));
    setWindowTitle(game.title);
    setModal(true);
    resize(820, 560);
    buildUi();
    applyGameData();
}

void GameDetailsDialog::buildUi() {
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(20);

    // ----- Left: cover -----
    m_cover = new QLabel(this);
    m_cover->setObjectName(QStringLiteral("DetailsCover"));
    m_cover->setFixedSize(280, 380);
    m_cover->setAlignment(Qt::AlignCenter);
    m_cover->setText(QStringLiteral("No cover"));
    root->addWidget(m_cover, 0, Qt::AlignTop);

    // ----- Right: info column -----
    auto* col  = new QVBoxLayout;
    col->setSpacing(12);
    root->addLayout(col, 1);

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

    // ----- Expandable synopsis (collapsible) -----
    m_synopsisToggle = new QToolButton(this);
    m_synopsisToggle->setObjectName(QStringLiteral("SynopsisToggle"));
    m_synopsisToggle->setText(QStringLiteral("▾  Synopsis"));
    m_synopsisToggle->setCheckable(true);
    m_synopsisToggle->setChecked(true);
    m_synopsisToggle->setToolButtonStyle(Qt::ToolButtonTextOnly);
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

    // ----- Actions -----
    auto* actions = new QHBoxLayout;
    actions->addStretch(1);
    m_launchBtn = new QPushButton(QStringLiteral("▶  Play"), this);
    m_launchBtn->setObjectName(QStringLiteral("PrimaryButton"));
    m_launchBtn->setMinimumHeight(40);
    actions->addWidget(m_launchBtn);
    col->addLayout(actions);

    connect(m_launchBtn, &QPushButton::clicked, this, [this] {
        emit launchRequested(m_game);
    });
}

void GameDetailsDialog::applyGameData() {
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

void GameDetailsDialog::setCover(const QPixmap& pix) {
    if (pix.isNull()) return;
    m_cover->setPixmap(pix.scaled(m_cover->size(),
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));
}

void GameDetailsDialog::setGame(const Game& g) {
    m_game = g;
    applyGameData();
}

} // namespace omnium
