#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QSpacerItem>

#include "imageentry.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:

    // Файл
    void menuFileOpen();
    void menuFileExit();

private:
    void loadImage();
    void activateMenu();

    void addEntryToHistory(const QString& text = "", int index = -1);
    void clearHistory();

private:
    Ui::MainWindow *ui;

    QImage *image;

    QLayout* historyLayout;
    QSpacerItem* historySpacer;
    QList<imageEntry*> historyList;
};

#endif // MAINWINDOW_H
