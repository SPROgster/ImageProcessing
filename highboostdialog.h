#ifndef HIGHBOOSTDIALOG_H
#define HIGHBOOSTDIALOG_H

#include <QDialog>
#include <QAbstractButton>

#include "mainwindow.h"

namespace Ui {
class HighBoostDialog;
}

class HighBoostDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HighBoostDialog(QWidget *parent = 0);
    ~HighBoostDialog();

private:
    Ui::HighBoostDialog *ui;
    MainWindow* parentWindow;

private slots:
    void spinMoved(double newValue);
    void sliderMoved(int newValue);
    void buttonPressed(QAbstractButton *button_);

private:
    int value;

};

#endif // HIGHBOOSTDIALOG_H
