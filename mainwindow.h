#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "coindialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    // Файл
    void menuFileOpen();
    void menuFileExit();

    // Редактирование
    void menuEditCoinMask();

private:
    void loadImage();
    void activateMenu();

#include "morphology.h"

public:
    void MoneyMask(int size = 15);

private:
    Ui::MainWindow *ui;

    QImage *image;

    CoinDialog *coinDialog;
};

#endif // MAINWINDOW_H
