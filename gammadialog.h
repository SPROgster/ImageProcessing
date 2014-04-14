#ifndef GAMMADIALOG_H
#define GAMMADIALOG_H

#include <QDialog>
#include <QAbstractButton>

#include "mainwindow.h"

namespace Ui {
class gammaDialog;
}

class gammaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit gammaDialog(QWidget *parent = 0);
    ~gammaDialog();

private slots:
    void gammaSliderMoved(int newValue);
    void gammaSpinMoved(double newValue);

    void buttonPressed(QAbstractButton* button_);

private:
    Ui::gammaDialog *ui;

    MainWindow* parentWindow;

    double value;
};

#endif // GAMMADIALOG_H
