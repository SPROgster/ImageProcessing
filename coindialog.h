#ifndef COINDIALOG_H
#define COINDIALOG_H

#include <QDialog>

namespace Ui {
class CoinDialog;
}

class CoinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CoinDialog(QWidget *parent = 0);
    ~CoinDialog();

private:
    Ui::CoinDialog *ui;
};

#endif // COINDIALOG_H
