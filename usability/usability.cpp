#include <QtGui>

#include "usability.h"

UsabilityDialog::UsabilityDialog(QWidget *parent)
    : QDialog(parent)
{
    //resize (1024, 768);
    showFullScreen ();

    QFileInfo fileInfo(".");

    tabWidget = new QTabWidget;
    tabWidget->addTab(new CursorTab(), tr("Follow the Cursor"));
    tabWidget->addTab(new ButtonTab(), tr("Buttons"));
    tabWidget->addTab(new PerformanceTab(), tr("Performance"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Usability Tester"));
}

CursorTab::CursorTab(QWidget *parent)
    : QWidget(parent)
{
    QGraphicsScene *s = new QGraphicsScene;//(QRect (-1024, -768, 1024*2, 768*2));
    s->setBackgroundBrush (Qt::white);
    s->addText ("Keep the tip of the arrow inside the cursor.  Press the SPACEBAR to begin.");
    QGraphicsView *v = new QGraphicsView (s, this);
    QGraphicsEllipseItem *e = new QGraphicsEllipseItem (0, s);
    e->setRect (-50.0, -50.0, 100.0, 100.0);
    v->setRenderHints (QPainter::Antialiasing);
    v->show ();
    //QGraphicsEllipseItem e;
    //QPropertyAnimation a;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(v);
    setLayout(mainLayout);
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
