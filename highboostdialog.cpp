#include "highboostdialog.h"
#include "ui_highboostdialog.h"

HighBoostDialog::HighBoostDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HighBoostDialog)
{
    ui->setupUi(this);

    value = 0;
    parentWindow = (MainWindow*)parent;

    connect(ui->coeffSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
    connect(ui->coeffSpin, SIGNAL(valueChanged(double)), this, SLOT(spinMoved(double)));

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonPressed(QAbstractButton*)));
}

HighBoostDialog::~HighBoostDialog()
{
    delete ui;
}

void HighBoostDialog::spinMoved(double newValue)
{
    newValue += 1e-6;
    value = newValue * 10;
    int sliderValue = ui->coeffSlider->value();

    if (sliderValue != value)
        ui->coeffSlider->setValue(value);
}

void HighBoostDialog::sliderMoved(int newValue)
{
    value = newValue;

    ui->coeffSpin->setValue((double)value / 10. + 1e-6);
}

void HighBoostDialog::buttonPressed(QAbstractButton *button_)
{
    bool type = ui->type8radio->isChecked();

    if (button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Apply)
            || button_ == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Ok))
        parentWindow->highBoostFiltering(value, type);
}
