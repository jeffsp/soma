#include <QtGui>
#include <algorithm>
#include <numeric>

#include "usability.h"

/// @brief constructor
///
/// @param parent parent widget
UsabilityDialog::UsabilityDialog(QWidget *parent)
    : QDialog(parent)
{
    // seed the random number generator
    qsrand (QTime::currentTime ().msec ());
    // take up whole screen
    showFullScreen ();
    // allocate tabs
    tabWidget = new QTabWidget (this);
    cursorTab = new CursorTab(this);
    buttonTab = new ButtonTab(this);
    resultsTab = new ResultsTab(this);
    // add them to the tab widget
    tabWidget->addTab(cursorTab, tr("Follow the Cursor"));
    tabWidget->addTab(buttonTab, tr("Buttons"));
    tabWidget->addTab(resultsTab, tr("Results"));
    // set focus to client area.  this has to be done after the cursor view is constructed.
    cursorTab->getCursorView ()->setFocus ();
    // add the OK/Cancel buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    // set the layout
    QVBoxLayout *mainLayout = new QVBoxLayout (this);
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    // set the title
    setWindowTitle(tr("Usability Tester"));
}

/// @brief constructor
///
/// @param parent parent widget
CursorScene::CursorScene (QWidget * parent)
    : QGraphicsScene (parent)
    , testing (false)
    , text (new QGraphicsTextItem ("Try to keep the cursor inside of the circle.  Press SPACE to begin."))
    , circle (new QGraphicsEllipseItem (0))
{
    // set the text font
    text->setFont (QFont ("Arial", 18, QFont::Bold));
    // center the instruction text
    QRectF r = text->boundingRect ();
    text->setPos (-r.width () / 2, -r.height () / 2);
    // add it
    addItem (text);
    // make a circle
    circle->setRect (-RADIUS, -RADIUS, RADIUS, RADIUS);
    addItem (circle);
    // set background of client area
    setBackgroundBrush (Qt::white);
    // initially show only the text
    text->setVisible (true);
    circle->setVisible (false);
}

/// @brief used to move the circle at set intervals
void CursorScene::timerEvent (QTimerEvent *)
{
    // check to see if the test is over yet
    if (start_time.elapsed () / 1000 >= TEST_SECS)
    {
        setTesting (false);
        return;
    }
    // move along a line 'dist' intervals
    static int dist = 0;
    static int dx = 0;
    static int dy = 0;
    if (dist == 0)
    {
        // we're at the end of the line, get a new one
        dist = qrand () % 10;
        // get a new direction
        dx = (qrand () % 3) - 1;
        dy = (qrand () % 3) - 1;
    }
    else
    {
        // count down one interval along the line
        --dist;
    }
    // get the new position
    QPointF p = circle->pos ();
    QPointF q (p.x () + dx * POS_GAIN, p.y () + dy * POS_GAIN);
    // remember the position
    posX.push_front (q.x ());
    posY.push_front (q.y ());
    // forget old positions
    posX.pop_back ();
    posY.pop_back ();
    // spatial filtering
    float ax = 1.0 * std::accumulate (posX.begin (), posX.end (), 0) / posX.size ();
    float ay = 1.0 * std::accumulate (posY.begin (), posY.end (), 0) / posY.size ();
    // set the new position
    circle->setPos (QPointF (ax, ay));
    // frame counter
    ++frames;
};

/// @brief access function
///
/// @return testing flag
bool CursorScene::getTesting () const
{
    return testing;
}

/// @brief access function
///
/// @param f testing flag
void CursorScene::setTesting (bool f)
{
    testing = f;
    if (testing)
    {
        // start a new test
        timer.start (1000/30, this);
        start_time.start ();
        frames = 0;
        text->setVisible (false);
        circle->setVisible (true);
        posX.clear ();
        posY.clear ();
        posX.assign (POS_MAX, 0.0);
        posY.assign (POS_MAX, 0.0);
    }
    else
    {
        // end the test
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

/// @brief constructor
///
/// @param scene scene data
/// @param parent parent widget
CursorView::CursorView (CursorScene * scene, QWidget * parent)
    : QGraphicsView (scene, parent)
    , cursorScene (scene)
{
}

/// @brief constructor
///
/// @param parent parent widget
CursorTab::CursorTab(QWidget *parent)
    : QWidget(parent)
{
    // setup the scene and view
    cursorScene = new CursorScene (this);
    cursorView = new CursorView (cursorScene, this);
    cursorView->setRenderHints (QPainter::Antialiasing);
    cursorView->show ();
    // setup the layout
    QHBoxLayout *mainLayout = new QHBoxLayout (this);
    mainLayout->addWidget(cursorView);
    setLayout(mainLayout);
}

/// @brief control the cursor test with the keyboard
///
/// @param e keyboard event
void CursorTab::keyPressEvent (QKeyEvent *e)
{
    switch (e->key())
    {
        // on SPACEBAR, start the test
        case ' ':
        cursorScene->setTesting (!cursorScene->getTesting ());
        break;
    }
    // call through to base class
    QWidget::keyPressEvent (e);
}

/// @brief constructor
///
/// @param parent parent widget
ButtonTab::ButtonTab(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QPushButton *button = new QPushButton("Click Me");
    button->setFixedSize(100,30);
    mainLayout->addWidget(button);
    setLayout(mainLayout);
}

/// @brief constructor
///
/// @param parent parent widget
ResultsTab::ResultsTab(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
}
