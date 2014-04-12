#include <QDialogButtonBox>

#include "mainwindow.h"

#include "coindialog.h"
#include "ui_coindialog.h"

CoinDialog::CoinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoinDialog)
{
    ui->setupUi(this);

    // При изменении слайдера или спинбокса меняем другой
    connect(ui->coinSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
    connect(ui->coinSpin, SIGNAL(valueChanged(int)), this, SLOT(spinChanged(int)));

    // Обработчик кнопки apply
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonPressed(QAbstractButton*)));

    value = 15;
}

CoinDialog::~CoinDialog()
{
    delete ui;
}

void CoinDialog::sliderMoved(int value_)
{
    value = value_;
    ui->coinSpin->setValue(value);
}

void CoinDialog::spinChanged(int value_)
{
    value = value_;
    ui->coinSlider->setValue(value);
}

void CoinDialog::buttonPressed(QAbstractButton *button_)
{
    if (button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Apply))
        parentWindow->MoneyMask(value);
}

void CoinDialog::setParent(MainWindow *parentWindow_)
{
    parentWindow = parentWindow_;
}

void CoinDialog::setMaxValue(QImage *image)
{
    int imageWidth = image->width();
    int imageHeigth = image->height();

    int maxValue = (imageWidth > imageHeigth) ? imageWidth : imageHeigth;

    ui->coinSlider->setMaximum(maxValue);
    ui->coinSpin->setMaximum(maxValue);
}
