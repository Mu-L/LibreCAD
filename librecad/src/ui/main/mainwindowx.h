#ifndef MAINWINDOWX_H
#define MAINWINDOWX_H

#include <QMainWindow>

/**
 * an eXtension of QMainWindow;
 * It is intended to be generic,
 * for use with other projects.
 */
class QToolBar;

class MainWindowX : public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindowX(QWidget* parent = nullptr);
    void sortWidgetsByTitle(QList<QDockWidget*>& list);
    void sortWidgetsByTitle(QList<QToolBar*>& list);
    void sortWidgetsByGroupAndTitle(QList<QToolBar*>& list);
public slots:
    void toggleRightDockArea(bool state);
    void toggleLeftDockArea(bool state);
    void toggleTopDockArea(bool state);
    void toggleBottomDockArea(bool state);
    void toggleFloatingDockwidgets(bool state);
};

#endif // MAINWINDOWX_H
