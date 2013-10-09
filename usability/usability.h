#ifndef USABILITY_H
#define USABILITY_H

#include <deque>
#include <QtGui>

/// @brief test results tab
class ResultsTab : public QWidget
{
    Q_OBJECT
    QListWidget *listWidget;
    public:
    /// @brief constructor
    ///
    /// @param parent widget
    ResultsTab(QWidget *parent = 0);
    void Add (const QString &s);
};

/// @brief scene data for the cursor test
class CursorScene : public QGraphicsScene
{
    Q_OBJECT
    /// @brief access to results
    ResultsTab *resultsTab;
    /// @brief duration of test
    static const int TEST_SECS = 10;
    /// @brief size of circle
    static const int RADIUS = 30;
    /// @brief used to filter circle position
    std::deque<int> posX;
    std::deque<int> posY;
    /// @brief movement/filter parameters
    static const int POS_GAIN = 40;
    static const size_t POS_MAX = 17;
    /// @brief flag that says if we are currently running a test
    bool testing;
    /// @brief test instructions
    QGraphicsTextItem *text;
    /// @brief displayed circle
    QGraphicsEllipseItem *circle;
    /// @brief used to generate circle movement events
    QBasicTimer timer;
    /// @brief for timing
    QTime start_time;
    /// @brief frame counter
    int frames;
    /// @brief performance stuff
    size_t mse_total;
    double se;
    void start_test ();
    void stop_test ();
    public:
    /// @brief constructor
    ///
    /// @param parent parent widget
    CursorScene (QWidget * parent, ResultsTab *resultsTab);
    /// @brief override
    ///
    /// @param event
    void timerEvent (QTimerEvent* event);
    /// @brief testing flag access
    ///
    /// @return testing flag
    bool getTesting () const;
    /// @brief testing flag access
    ///
    /// @param f testing flag
    void setTesting (bool f);
};

/// @brief view for cursor scene data
class CursorView : public QGraphicsView
{
    Q_OBJECT
    /// @brief scene data
    CursorScene *cursorScene;
    public:
    /// @brief constructor
    ///
    /// @param scene scene data
    /// @param parent parent widget
    CursorView (CursorScene *scene, QWidget * parent = 0);
};

/// @brief cursor test tab
class CursorTab : public QWidget
{
    Q_OBJECT
    /// @brief access to results
    ResultsTab *resultsTab;
    /// @brief scene data
    CursorScene *cursorScene;
    /// @brief scene view
    CursorView *cursorView;
    public:
    /// @brief constructor
    ///
    /// @param parent parent widget
    /// @param access to results
    CursorTab(QWidget *parent, ResultsTab *results);
    /// @brief override
    ///
    /// @param control tests
    void keyPressEvent (QKeyEvent *);
    /// @brief view access
    ///
    /// @return cursor view
    ///
    /// Required for setting focus AFTER the tab is visible.
    CursorView *getCursorView () const { return cursorView; }
};

/// @brief button test tab
class ButtonTab : public QWidget
{
    Q_OBJECT
    /// @brief access to results
    ResultsTab *resultsTab;
    QLabel *label;
    QPushButton *button;
    bool testing;
    void start_test ();
    void stop_test ();
    /// @brief for timing
    QTime start_time;
    /// @brief click counter
    int clicks;
    int width;
    int height;
    public:
    /// @brief constructor
    ///
    /// @param parent widget
    /// @param access to results
    ButtonTab(QWidget *parent, ResultsTab *results);
    /// @brief override
    ///
    /// @param control tests
    void keyPressEvent (QKeyEvent *);
    /// @brief testing flag access
    ///
    /// @return testing flag
    bool getTesting () const;
    /// @brief testing flag access
    ///
    /// @param f testing flag
    void setTesting (bool f);
    public slots:
    /// @brief button click slot
    void click ();
};

/// @brief button test tab
class ScrollTab : public QWidget
{
    Q_OBJECT
    /// @brief access to results
    ResultsTab *resultsTab;
    public:
    /// @brief constructor
    ///
    /// @param parent widget
    /// @param access to results
    ScrollTab(QWidget *parent, ResultsTab *results);
};

/// @brief button test tab
class DragTab : public QWidget
{
    Q_OBJECT
    /// @brief access to results
    ResultsTab *resultsTab;
    public:
    /// @brief constructor
    ///
    /// @param parent widget
    /// @param access to results
    DragTab(QWidget *parent, ResultsTab *results);
};

/// @brief dialog that contains usability test tabs
class UsabilityDialog : public QDialog
{
    Q_OBJECT
    public:
    /// @brief constructor
    ///
    /// @param parent widget
    UsabilityDialog(QWidget *parent = 0);
    private:
    /// @brief widget that contains the tabs
    QTabWidget *tabWidget;
    /// @brief OK/Cancel buttons
    QDialogButtonBox *buttonBox;
    /// @brief tabs
    CursorTab *cursorTab;
    ButtonTab *buttonTab;
    ScrollTab *scrollTab;
    DragTab *dragTab;
    ResultsTab *resultsTab;
};

#endif
