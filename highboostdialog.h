#ifndef HIGHBOOSTDIALOG_H
#define HIGHBOOSTDIALOG_H

#include <QDialog>

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
};

#endif // HIGHBOOSTDIALOG_H
