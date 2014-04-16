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

    ui->gammaSpin->setSingleStep(0.01);
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
        value = (double)(newValue - 100) / 10. + 1. + 1e-6;

    ui->gammaSpin->setValue(value);
}

void gammaDialog::gammaSpinMoved(double newValue)
{
    newValue += 1e-6;
    int newValueInt = (newValue * 100) / 1;
    int sliderValue = ui->gammaSlider->value();

    if (newValue < 1.)
    {
        value = newValue + 1e-6;
    }
    else
    {
        value = newValue;
        newValueInt = (newValueInt - 100) / 10 + 100;
    }

    if (newValueInt != sliderValue)
        ui->gammaSlider->setValue(newValueInt);
}

void gammaDialog::buttonPressed(QAbstractButton *button_)
{
    if (button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Apply)
            || button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Ok))
        parentWindow->gammaCorrection(value);
}
