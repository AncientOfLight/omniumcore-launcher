#pragma once

#include <QDialog>

#include "models/Game.h"

class QLabel;
class QTextBrowser;
class QPushButton;
class QToolButton;
class QFrame;

namespace omnium {

// Expandable details panel: cover, title, platform, metadata (release date,
// genre, score) and a collapsible synopsis. Pure Qt Widgets — no QML.
class GameDetailsDialog : public QDialog {
    Q_OBJECT
public:
    explicit GameDetailsDialog(const Game& game, QWidget* parent = nullptr);

    void setCover(const QPixmap& pix);
    void setGame(const Game& g);

signals:
    void launchRequested(const omnium::Game& game);

private:
    void buildUi();
    void applyGameData();

    Game          m_game;
    QLabel*       m_cover         = nullptr;
    QLabel*       m_title         = nullptr;
    QLabel*       m_platform      = nullptr;
    QLabel*       m_release       = nullptr;
    QLabel*       m_genres        = nullptr;
    QLabel*       m_rating        = nullptr;
    QToolButton*  m_synopsisToggle = nullptr;
    QFrame*       m_synopsisBox    = nullptr;
    QTextBrowser* m_synopsis       = nullptr;
    QPushButton*  m_launchBtn      = nullptr;
};

} // namespace omnium
