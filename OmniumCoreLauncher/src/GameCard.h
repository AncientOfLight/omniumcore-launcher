#pragma once

#include <QFrame>
#include <QPixmap>

#include "models/Game.h"

class QLabel;
class QMouseEvent;

namespace omnium {

// Visual card displayed inside the FlowLayout grid.
// Emits clicked() when the user activates it.
class GameCard : public QFrame {
    Q_OBJECT
public:
    explicit GameCard(const Game& game, QWidget* parent = nullptr);

    const Game& game() const { return m_game; }

    void setCover(const QPixmap& cover);
    void setGame(const Game& g);

signals:
    void clicked(const omnium::Game& game);
    void launchRequested(const omnium::Game& game);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    void rebuildUi();

    Game    m_game;
    QLabel* m_coverLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_platformBadge = nullptr;
};

} // namespace omnium
