#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QSpacerItem>

#include "imageentry.h"

class CoinDialog;

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

    // Редактирование
    void menuEditCoinMask();

private:
    void loadImage();
    void activateMenu();

    void addEntryToHistory(const QString& text = "", int index = -1);
    void clearHistory();

#include "morphology.h"

public:
    void MoneyMask(int size = 15);

private:
    Ui::MainWindow *ui;

    QImage *image;

    QLayout* historyLayout;
    QSpacerItem* historySpacer;
    QList<imageEntry*> historyList;

    CoinDialog *coinDialog;
};

#endif // MAINWINDOW_H
