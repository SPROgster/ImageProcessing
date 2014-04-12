#include "coindialog.h"
#include "ui_coindialog.h"

CoinDialog::CoinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoinDialog)
{
    ui->setupUi(this);
}

CoinDialog::~CoinDialog()
{
    delete ui;
}
