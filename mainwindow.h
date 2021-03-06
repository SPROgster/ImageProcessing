#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define DeleteIfNotNull(x) if (x != 0) { delete x; x = 0; }

#include <QList>
#include <QMainWindow>
#include <QSpacerItem>

#include "config.h"
#include "imageentry.h"
#include "progressiveCut.h"

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

    // Слоты progressive Cut
    void buttonForegroundClicked();
    void buttonBackgroundClicked();
    void buttonCreateGraphClicked();

    // Слоты панели маски
    void maskButtonClicked(bool checked);
    void maskMergeButtonClicked();
    void maskCancelButtonClicked();
    void maskSpinChanged(int value);
    void maskSliderChanged(int value);

    // Файл
    void menuFileOpen();
    void menuFileExit();

private:
    void loadImage();
    void activateMenu();

    void maskValueChanged(int value);

    void addEntryToHistory(const QString& text = "", int index = -1);
    void clearHistory();

    #include "structuralElements.h"

private:
    Ui::MainWindow *ui;

    QImage *image;

    //Макса
private:
    bool masking;
    bool maskingDrawing;
    bool maskIsEmpty;

    int maskValue;
    QImage* maskImage;
    QImage* maskImageAlpha;
    QCursor* maskCursor;

    QImage* maskedImage;
    //Выделение
private:
    void selectionMerging();
    void selectionPreview();

    QImage* selection;
    QImage* selectionBuffer;
    QImage* selectionAlpha;

    // Гистограмма
    long* computeHistogramRGB();

private:
    QLayout* historyLayout;
    QSpacerItem* historySpacer;
    QList<imageEntry*> historyList;

    // Progressive Cut
private:
    ProgressiveCut* progressiveCut;

    bool graphCreated;
    bool segmentBackgroundNew;
    bool segmentForegroundNew;

    friend class ProgressiveCut;
};

#endif // MAINWINDOW_H
