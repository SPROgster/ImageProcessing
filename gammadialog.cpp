#include "gammadialog.h"
#include "ui_gammadialog.h"

gammaDialog::gammaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::gammaDialog)
{
    ui->setupUi(this);

    parentWindow = (MainWindow*) parent;

    connect(ui->gammaSlider, SIGNAL(sliderMoved(int)), this, SLOT(gammaSliderMoved(int)));
    connect(ui->gammaSpin, SIGNAL(valueChanged(double)), this, SLOT(gammaSpinMoved(double)));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonPressed(QAbstractButton*)));

    value = 1.0;
}

gammaDialog::~gammaDialog()
{
    delete ui;
}

void gammaDialog::gammaSliderMoved(int newValue)
{
    if (newValue < 100)
    {
        value = (double)newValue / 100. + 1e-6;
        ui->gammaSpin->setSingleStep(0.01);
    }
    else
    {
        value = (double)(newValue - 100) / 10. + 1. + 1e-6;
        ui->gammaSpin->setSingleStep(0.1);
    }

    ui->gammaSpin->setValue(value);
}

void gammaDialog::gammaSpinMoved(double newValue)
{
    int newValueInt = (newValue * 100) / 1;

    if (newValueInt < 100)
    {
        value = newValue + 1e-6;
        ui->gammaSpin->setSingleStep(0.01);
    }
    else
    {
        newValueInt = (newValueInt - 100) / 10 + 100;
        value = (double)newValueInt / 10. + 1. + 1e-6;
        ui->gammaSpin->setSingleStep(0.1);
    }

    ui->gammaSlider->setValue(newValueInt);
}

void gammaDialog::buttonPressed(QAbstractButton *button_)
{
    if (button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Apply))
        parentWindow->gammaCorrection(value);
}
