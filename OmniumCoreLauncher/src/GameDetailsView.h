#pragma once

#include <QWidget>

#include "models/Game.h"

class QLabel;
class QTextBrowser;
class QPushButton;
class QToolButton;
class QFrame;

namespace omnium {

// In-page details panel embedded in the main window's QStackedWidget.
// Replaces the old modal dialog: navigation stays inside a single window.
class GameDetailsView : public QWidget {
    Q_OBJECT
public:
    explicit GameDetailsView(QWidget* parent = nullptr);

    // Populate the view with a new game (e.g. when the user selects a card).
    void setGame(const Game& game);
    void setCover(const QPixmap& pix);

    const Game& game() const { return m_game; }

signals:
    void backRequested();
    void launchRequested(const omnium::Game& game);

private:
    void buildUi();
    void applyGameData();

    Game          m_game;
    QPushButton*  m_backBtn        = nullptr;
    QLabel*       m_cover          = nullptr;
    QLabel*       m_title          = nullptr;
    QLabel*       m_platform       = nullptr;
    QLabel*       m_release        = nullptr;
    QLabel*       m_genres         = nullptr;
    QLabel*       m_rating         = nullptr;
    QToolButton*  m_synopsisToggle = nullptr;
    QFrame*       m_synopsisBox    = nullptr;
    QTextBrowser* m_synopsis       = nullptr;
    QPushButton*  m_launchBtn      = nullptr;
};

} // namespace omnium
