#ifndef COINDIALOG_H
#define COINDIALOG_H

#include <QDialog>
#include <QAbstractButton>

#include "mainwindow.h"

namespace Ui {
class CoinDialog;
}

class CoinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CoinDialog(QWidget *parent = 0);
    ~CoinDialog();

private slots:
    void sliderMoved(int value_);
    void spinChanged(int value_);

    void buttonPressed(QAbstractButton* button_);

private:
    Ui::CoinDialog *ui;

    MainWindow *parentWindow;

public:
    void setParent(MainWindow *parentWindow_);
    void setMaxValue(QImage* image);

    int value;
};

#endif // COINDIALOG_H
