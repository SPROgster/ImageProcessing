#ifndef COINVIEWDIALOG_H
#define COINVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class coinViewDialog;
}

class coinViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit coinViewDialog(QWidget *parent = 0);
    ~coinViewDialog();

    QImage *selectComponents(const QImage *origin, int &colorNumber);
    QImage *imageGradient(const QImage *origin);
    void loadCoins(const QImage* coins);
    void replaceColor(QImage *image, const QRgb colorToReplace, const QRgb newColor);
private:
    Ui::coinViewDialog *ui;
};

#endif // COINVIEWDIALOG_H
