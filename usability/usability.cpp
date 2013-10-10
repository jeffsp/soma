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
    resultsTab = new ResultsTab(this);
    cursorTab = new CursorTab(this, resultsTab);
    buttonTab = new ButtonTab(this, resultsTab);
    scrollTab = new ScrollTab(this, resultsTab);
    dragTab = new DragTab(this, resultsTab);
    // add them to the tab widget
    tabWidget->addTab(cursorTab, tr("Follow the Cursor"));
    tabWidget->addTab(buttonTab, tr("Buttons"));
    tabWidget->addTab(scrollTab, tr("Scroll"));
    tabWidget->addTab(dragTab, tr("Drag"));
    tabWidget->addTab(resultsTab, tr("Results"));
    // set focus to client area.  this has to be done after the cursor view is constructed.
    cursorTab->getCursorView ()->setFocus ();
    // add the OK/Cancel buttons
    //buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
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
CursorScene::CursorScene (QWidget * parent, ResultsTab *resultsTab)
    : QGraphicsScene (parent)
    , resultsTab (resultsTab)
    , testing (false)
    , text (new QGraphicsTextItem ("Try to keep the cursor inside of the circle.  Press SPACE to begin."))
    , circle (new QGraphicsEllipseItem (0))
{
    // set the text font
    text->setFont (QFont ("Arial", 18, QFont::Bold));
    // center the instruction text
    QRectF r = text->boundingRect ();
    text->setPos (-r.width () / 2, -r.height () / 2 + 100);
    // add it
    addItem (text);
    // make a circle
    circle->setRect (-RADIUS, -RADIUS, RADIUS, RADIUS);
    addItem (circle);
    // set background of client area
    setBackgroundBrush (Qt::white);
    // show the text
    text->setVisible (true);
    circle->setVisible (true);
    // start circle in the center
    circle->setPos (QPointF (0, 0));
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
    {
        // get the current circle center in scene coordinates
        QPointF p = circle->boundingRect ().center ();
        // get position in viewport coordinates
        QPoint v = views ().first ()->mapFromScene (p);
        // get position in global coordinates
        QPoint g = views ().first ()->viewport ()->mapToGlobal (v);
        // get the current cursor position in pixels
        QPoint q = QCursor::pos ();
        // update error rate
        ++mse_total;
        float dx = g.x () - q.x() ;
        float dy = g.y () - q.y ();
        float d = sqrt (dx * dx + dy * dy);
        se += d * d;
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
        start_test ();
    else
        stop_test ();
}

void CursorScene::start_test ()
{
    // start a new test
    timer.start (1000/30, this);
    start_time.start ();
    // reset frame counter
    frames = 0;
    // reset performance counter
    mse_total = 0;
    se = 0.0;
    // show the text
    text->setVisible (false);
    circle->setVisible (true);
    // start circle in the center
    circle->setPos (QPointF (0, 0));
    // clear position buffers
    posX.clear ();
    posY.clear ();
    posX.assign (POS_MAX, 0.0);
    posY.assign (POS_MAX, 0.0);
}

void CursorScene::stop_test ()
{
    // end the test
    timer.stop ();
    // get fps
    int elapsed = start_time.elapsed ();
    float secs = elapsed / 1000.0f;
    QString s;
    s.sprintf ("CursorTest:frames %d", frames);
    qDebug () << s;
    s.sprintf ("CursorTest:seconds %f", secs);
    qDebug () << s;
    float fps = 0.0;
    if (elapsed)
        fps = frames / secs;
    s.sprintf ("CursorTest:fps %f", fps);
    qDebug () << s;
    // get performance
    float rmse = sqrt (se / mse_total);
    s.sprintf ("CursorTest:rmse %f", rmse);
    resultsTab->Add (s); qDebug () << s;
    // show the text
    text->setVisible (true);
    circle->setVisible (true);
    // start circle in the center
    circle->setPos (QPointF (0, 0));
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
/// @param access to results
CursorTab::CursorTab(QWidget *parent, ResultsTab *resultsTab)
    : QWidget(parent)
    , resultsTab (resultsTab)
{
    // setup the scene and view
    cursorScene = new CursorScene (this, resultsTab);
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
/// @param access to results
ButtonTab::ButtonTab(QWidget *parent, ResultsTab *resultsTab)
    : QWidget(parent)
    , resultsTab (resultsTab)
    , testing (false)
{
    button = new QPushButton("Click Me");
    label = new QLabel("Click the buttons as fast as you can.  Press SPACE to begin.");
    label->setVisible (true);
    label->setFont (QFont ("Arial", 18, QFont::Bold));
    button->setFixedSize(80,30);
    button->setVisible (false);
    label->setParent (this);
    button->setParent (this);
    QRect r = parent->geometry ();
    width = r.right ();
    height = r.bottom ();
    label->move (width / 2, height / 2);
    button->move (width / 2, height / 2);
    button->setContextMenuPolicy (Qt::CustomContextMenu);
    connect(button, SIGNAL(clicked()), this, SLOT(click()));
    connect(button, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(click()));
}

/// @brief control the button test with the keyboard
///
/// @param e keyboard event
void ButtonTab::keyPressEvent (QKeyEvent *e)
{
    switch (e->key())
    {
        // on SPACEBAR, start the test
        case ' ':
        setTesting (!getTesting ());
        break;
    }
    // call through to base class
    QWidget::keyPressEvent (e);
}

/// @brief access function
///
/// @return testing flag
bool ButtonTab::getTesting () const
{
    return testing;
}

/// @brief access function
///
/// @param f testing flag
void ButtonTab::setTesting (bool f)
{
    testing = f;
    if (testing)
        start_test ();
    else
        stop_test ();
}

void ButtonTab::click ()
{
    ++clicks;
    if (clicks == 10)
        setTesting (false);
    button->move (qrand () % width, qrand () % height);
    setFocus ();
}

void ButtonTab::start_test ()
{
    label->setVisible (false);
    button->setVisible (true);
    start_time.start ();
    clicks = 0;
}

void ButtonTab::stop_test ()
{
    label->setVisible (true);
    button->setVisible (false);
    int elapsed = start_time.elapsed ();
    float secs = elapsed / 1000.0f;
    QString s;
    s.sprintf ("ButtonTest:clicks %d", clicks);
    qDebug () << s;
    s.sprintf ("ButtonTest:seconds %f", secs);
    qDebug () << s;
    float cps = 0.0;
    if (elapsed)
        cps = clicks / secs;
    s.sprintf ("ButtonTest:clicks/sec %f", cps);
    // report it
    resultsTab->Add (s); qDebug () << s;
}

/// @brief constructor
///
/// @param parent parent widget
ScrollScene::ScrollScene (QWidget * parent, ResultsTab *resultsTab)
    : QGraphicsScene (parent)
    , resultsTab (resultsTab)
    , testing (false)
    , text1 (new QGraphicsTextItem ("Scroll until the red circle is centered.  Press SPACE to begin."))
    , text2 (new QGraphicsTextItem ("Scroll until the red circle is centered.  Press SPACE to begin."))
{
    // set the text font
    text1->setFont (QFont ("Arial", 18, QFont::Bold));
    text2->setFont (QFont ("Arial", 18, QFont::Bold));
    QRectF r = text1->boundingRect ();
    const size_t NCIRCLES = 100;
    text1->setPos (-r.width () / 2, -4 * RADIUS);
    text2->setPos (-r.width () / 2, NCIRCLES * RADIUS * 2);
    // add it
    addItem (text1);
    addItem (text2);
    // set background of client area
    setBackgroundBrush (Qt::white);
    circles.resize (NCIRCLES);
    for (size_t i = 0; i < circles.size (); ++i)
    {
        QGraphicsEllipseItem *c = new QGraphicsEllipseItem (0);
        c->setRect (-RADIUS, -RADIUS, RADIUS, RADIUS);
        int y = i * RADIUS * 2;
        c->setPos (0, y);
        addItem (c);
        circles[i] = c;
    }
}

/// @brief access function
///
/// @return testing flag
bool ScrollScene::getTesting () const
{
    return testing;
}

/// @brief access function
///
/// @param f testing flag
void ScrollScene::setTesting (bool f)
{
    testing = f;
    if (testing)
        start_test ();
    else
        stop_test ();
}

void ScrollScene::start_test ()
{
    // start a new test
    start_time.start ();
    // reset frame counter
    scrolls = 0;
}

void ScrollScene::stop_test ()
{
    // get fps
    int elapsed = start_time.elapsed ();
    float secs = elapsed / 1000.0f;
    QString s;
    s.sprintf ("ScrollTest:scrolls %d", scrolls);
    qDebug () << s;
    s.sprintf ("ScrollTest:seconds %f", secs);
    qDebug () << s;
    float sps = 0.0;
    if (elapsed)
        sps = scrolls / secs;
    s.sprintf ("ScrollTest:scrolls/sec %f", sps);
    // report results
    resultsTab->Add (s); qDebug () << s;
}

/// @brief constructor
///
/// @param scene scene data
/// @param parent parent widget
ScrollView::ScrollView (ScrollScene * scene, QWidget * parent)
    : QGraphicsView (scene, parent)
    , scrollScene (scene)
{
}

/// @brief constructor
///
/// @param parent parent widget
/// @param access to results
ScrollTab::ScrollTab(QWidget *parent, ResultsTab *resultsTab)
    : QWidget(parent)
    , resultsTab (resultsTab)
{
    // setup the scene and view
    scrollScene = new ScrollScene (this, resultsTab);
    scrollView = new ScrollView (scrollScene, this);
    scrollView->setRenderHints (QPainter::Antialiasing);
    scrollView->show ();
    // setup the layout
    QHBoxLayout *mainLayout = new QHBoxLayout (this);
    mainLayout->addWidget(scrollView);
    setLayout(mainLayout);
    if (qrand () % 2)
        scrollView->verticalScrollBar()->setValue(scrollView->verticalScrollBar()->maximum ());
    else
        scrollView->verticalScrollBar()->setValue(scrollView->verticalScrollBar()->minimum ());
}

/// @brief control the scroll test with the keyboard
///
/// @param e keyboard event
void ScrollTab::keyPressEvent (QKeyEvent *e)
{
    switch (e->key())
    {
        // on SPACEBAR, start the test
        case ' ':
        scrollScene->setTesting (!scrollScene->getTesting ());
        break;
    }
    // call through to base class
    QWidget::keyPressEvent (e);
}

/// @brief constructor
///
/// @param parent parent widget
/// @param access to results
DragTab::DragTab(QWidget *parent, ResultsTab *resultsTab)
    : QWidget(parent)
    , resultsTab (resultsTab)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
}

/// @brief constructor
///
/// @param parent parent widget
ResultsTab::ResultsTab(QWidget *parent)
    : QWidget(parent)
    , listWidget (new QListWidget)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(listWidget);
    setLayout(mainLayout);
}

void ResultsTab::Add (const QString &s)
{
    listWidget->addItem (new QListWidgetItem (s, listWidget));
}
