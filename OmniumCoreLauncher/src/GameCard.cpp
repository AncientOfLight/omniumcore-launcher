#include <QStyle>
#include "GameCard.h"
#include "PlatformIcons.h"

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

namespace omnium {

static constexpr int kCardW = 240;
static constexpr int kCoverH = 320;

GameCard::GameCard(const Game& game, QWidget* parent)
    : QFrame(parent), m_game(game) {
    setObjectName(QStringLiteral("GameCard"));
    setFixedSize(kCardW, kCoverH + 80);
    setCursor(Qt::PointingHandCursor);
    setProperty("class", "card");

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 160));
    setGraphicsEffect(shadow);

    rebuildUi();
}

void GameCard::rebuildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_coverLabel = new QLabel(this);
    m_coverLabel->setObjectName(QStringLiteral("CoverLabel"));
    m_coverLabel->setFixedSize(kCardW, kCoverH);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setText(QStringLiteral("Loading…"));
    root->addWidget(m_coverLabel);

    auto* meta = new QWidget(this);
    meta->setObjectName(QStringLiteral("CardMeta"));
    auto* metaLayout = new QHBoxLayout(meta);
    metaLayout->setContentsMargins(12, 8, 12, 8);
    metaLayout->setSpacing(8);

    m_titleLabel = new QLabel(m_game.title, meta);
    m_titleLabel->setObjectName(QStringLiteral("CardTitle"));
    m_titleLabel->setWordWrap(true);

    m_platformBadge = new QLabel(platformToString(m_game.platform), meta);
    m_platformBadge->setObjectName(QStringLiteral("PlatformBadge"));
    m_platformBadge->setStyleSheet(
        QStringLiteral("background:%1; color:white; padding:3px 8px; border-radius:8px; font-size:10px;")
            .arg(PlatformIcons::badgeColorFor(m_game.platform)));
    m_platformBadge->setAlignment(Qt::AlignCenter);

    metaLayout->addWidget(m_titleLabel, 1);
    metaLayout->addWidget(m_platformBadge, 0, Qt::AlignTop | Qt::AlignRight);

    root->addWidget(meta);
}

void GameCard::setGame(const Game& g) {
    m_game = g;
    if (m_titleLabel)    m_titleLabel->setText(g.title);
    if (m_platformBadge) m_platformBadge->setText(platformToString(g.platform));
}

void GameCard::setCover(const QPixmap& cover) {
    if (cover.isNull() || !m_coverLabel) return;
    m_coverLabel->setPixmap(cover.scaled(
        m_coverLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void GameCard::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton)
        emit clicked(m_game);
    QFrame::mousePressEvent(e);
}

void GameCard::mouseDoubleClickEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton)
        emit launchRequested(m_game);
    QFrame::mouseDoubleClickEvent(e);
}

void GameCard::enterEvent(QEnterEvent* e) {
    setProperty("hovered", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(e);
}

void GameCard::leaveEvent(QEvent* e) {
    setProperty("hovered", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(e);
}

} // namespace omnium
