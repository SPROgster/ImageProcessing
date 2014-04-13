#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QSpacerItem>

#include "imageentry.h"

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

    // Слоты панели маски
    void maskButtonClicked(bool checked);
    void maskMergeButtonClicked();
    void maskCancelButtonClicked();
    void maskSpinChanged(int value);
    void maskSliderChanged(int value);

    // Файл
    void menuFileOpen();
    void menuFileExit();

    // Редактировать
    void menuEditCurve();
    void menuEditOtsu();

private:
    void loadImage();
    void activateMenu();

    void maskValueChanged(int value);

    void addEntryToHistory(const QString& text = "", int index = -1);
    void clearHistory();

    #include "structuralElements.h"

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

    //Макса
private:
    bool masking;
    bool maskingDrawing;
    bool maskIsEmpty;

    int maskValue;
    QImage* maskCursorImage;
    QImage* maskImage;
    QCursor* maskCursor;

    QImage* maskedImage;

    //Выделение
private:
    void selectionMerging();

    QImage* selection;

private:
    QLayout* historyLayout;
    QSpacerItem* historySpacer;
    QList<imageEntry*> historyList;

    KisCurveWidget *curveWindow;
};

#endif // MAINWINDOW_H
