#ifndef USABILITY_H
#define USABILITY_H

#include <deque>
#include <QtGui>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFileInfo;
class QTabWidget;
QT_END_NAMESPACE

class CursorScene : public QGraphicsScene
{
    std::deque<int> posX;
    std::deque<int> posY;
    static const int POS_GAIN = 40;
    static const size_t POS_MAX = 17;
    bool testing;
    QGraphicsTextItem *text;
    QGraphicsEllipseItem *circle;
    QBasicTimer timer;
    QTime start_time;
    int frames;
public:
    CursorScene (QWidget * parent = 0);
    void timerEvent (QTimerEvent* event);
    QGraphicsTextItem *getText () const
    {
        return text;
    }
    QGraphicsEllipseItem *getCircle () const
    {
        return circle;
    }
    bool getTesting () const
    {
        return testing;
    }
    void setTesting (bool f);
};

class CursorView : public QGraphicsView
{
    CursorScene *cursorScene;
public:
    CursorView (CursorScene * scene, QWidget * parent = 0);
    void paintEvent (QPaintEvent *);
    CursorScene *scene () { return cursorScene; }
};

class CursorTab : public QWidget
{
    Q_OBJECT
    CursorScene *cursorScene;
    CursorView *cursorView;

public:
    CursorTab(QWidget *parent = 0);
    void keyPressEvent (QKeyEvent *);
    CursorScene *getCursorScene () const { return cursorScene; }
    CursorView *getCursorView () const { return cursorView; }
};

class ButtonTab : public QWidget
{
    Q_OBJECT

public:
    ButtonTab(QWidget *parent = 0);
};


class PerformanceTab : public QWidget
{
    Q_OBJECT

public:
    PerformanceTab(QWidget *parent = 0);
};

class UsabilityDialog : public QDialog
{
    Q_OBJECT

    CursorTab *getCursorTab () const { return cursorTab; }
public:
    UsabilityDialog(QWidget *parent = 0);

private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    CursorTab *cursorTab;
};

#endif
