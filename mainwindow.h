#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "kis_curve_widget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:

    // Файл
    void menuFileOpen();
    void menuFileExit();

    // Редактировать
    void menuEditCurve();
    void menuEditOtsu();

private:
    void loadImage();
    void activateMenu();

// Кривая яркости
private:
    void showCurveWindow();
    void curveChanged();

// Бинаризация методом Отцу
    long *computeHistogram();

    int  otsuThreshold();
    void otsuBinarization();

private:
    Ui::MainWindow *ui;

    QImage *image;

    KisCurveWidget *curveWindow;
};

#endif // MAINWINDOW_H
