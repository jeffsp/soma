#include <QtGui>
#include <algorithm>
#include <numeric>

#include "usability.h"

UsabilityDialog::UsabilityDialog(QWidget *parent)
    : QDialog(parent)
{
    showFullScreen ();

    QFileInfo fileInfo(".");

    tabWidget = new QTabWidget (this);
    cursorTab = new CursorTab(this);
    tabWidget->addTab(cursorTab, tr("Follow the Cursor"));
    tabWidget->addTab(new ButtonTab(this), tr("Buttons"));
    tabWidget->addTab(new PerformanceTab(this), tr("Performance"));

    cursorTab->getCursorView ()->setFocus ();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout (this);
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Usability Tester"));
}

CursorScene::CursorScene (QWidget * parent)
    : QGraphicsScene (parent)
    , testing (false)
    , text (new QGraphicsTextItem ("Keep the cursor inside of the circle.  Press SPACE to begin."))
    , circle (new QGraphicsEllipseItem (0))
{
    text->setFont (QFont ("Arial", 18, QFont::Bold));
    circle->setRect (-10.0, -10.0, 10.0, 10.0);
    addItem (text);
    addItem (circle);
    setBackgroundBrush (Qt::white);
    text->setVisible (true);
    circle->setVisible (false);
}

void CursorScene::timerEvent (QTimerEvent *)
{
    static int dist = 0;
    static int dx = 0;
    static int dy = 0;
    if (dist == 0)
    {
        dist = qrand () % 10;
        dx = (qrand () % 3) - 1;
        dy = (qrand () % 3) - 1;
    }
    else
    {
        --dist;
    }
    QPointF p = getCircle ()->pos ();
    QPointF q (p.x () + dx * POS_GAIN, p.y () + dy * POS_GAIN);
    posX.push_front (q.x ());
    posY.push_front (q.y ());
    posX.pop_back ();
    posY.pop_back ();
    float ax = 1.0 * std::accumulate (posX.begin (), posX.end (), 0) / posX.size ();
    float ay = 1.0 * std::accumulate (posY.begin (), posY.end (), 0) / posY.size ();
    getCircle ()->setPos (QPointF (ax, ay));
    ++frames;
};

void CursorScene::setTesting (bool f)
{
    testing = f;
    if (testing)
    {
        timer.start (1000/30, this);
        start_time.start ();
        frames = 0;
        text->setVisible (false);
        circle->setVisible (true);
        posX.clear ();
        posY.clear ();
        posX.assign (POS_MAX, 0.0);
        posY.assign (POS_MAX, 0.0);
        getCircle ()->setPos (QPointF (0.0, 0.0));
    }
    else
    {
        timer.stop ();
        int elapsed = start_time.elapsed ();
        float secs = elapsed / 1000.0f;
        qDebug () << frames << " frames";
        qDebug () << secs << " secs";
        if (elapsed)
            qDebug () << frames / secs << "fps";
        frames = 0;
        text->setVisible (true);
        circle->setVisible (false);
    }
}

CursorView::CursorView (CursorScene * scene, QWidget * parent)
    : QGraphicsView (scene, parent)
    , cursorScene (scene)
{
}

CursorTab::CursorTab(QWidget *parent)
    : QWidget(parent)
{
    cursorScene = new CursorScene (this);
    cursorView = new CursorView (cursorScene, this);
    cursorView->setRenderHints (QPainter::Antialiasing);
    cursorView->show ();
    QHBoxLayout *mainLayout = new QHBoxLayout (this);
    mainLayout->addWidget(cursorView);
    setLayout(mainLayout);
}

void CursorView::paintEvent (QPaintEvent *e)
{
    QGraphicsView::paintEvent (e);
}

void CursorTab::keyPressEvent (QKeyEvent *e)
{
    qDebug () << "CursorTab::keyPressEvent" << e->key ();
    switch (e->key())
    {
        case ' ':
            qDebug () << "SPACE";
            cursorScene->setTesting (!cursorScene->getTesting ());
        break;
    }
    QWidget::keyPressEvent (e);
}

ButtonTab::ButtonTab(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QPushButton *button = new QPushButton("Click Me");
    button->setFixedSize(100,30);
    mainLayout->addWidget(button);
    setLayout(mainLayout);
}

PerformanceTab::PerformanceTab(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
}
