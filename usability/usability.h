#ifndef USABILITY_H
#define USABILITY_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFileInfo;
class QTabWidget;
QT_END_NAMESPACE

class CursorTab : public QWidget
{
    Q_OBJECT

public:
    CursorTab(QWidget *parent = 0);
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

public:
    UsabilityDialog(QWidget *parent = 0);

private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
};

#endif
