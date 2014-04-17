#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QSpacerItem>

#include "imageentry.h"

class CoinDialog;

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

    // Редактирование
    void menuEditCoinMask();

private:
    void loadImage();
    void activateMenu();

    void maskValueChanged(int value);

    void addEntryToHistory(const QString& text = "", int index = -1);
    void clearHistory();

#include "structuralElements.h"
#include "morphology.h"

public:
    void MoneyMask(int size = 15);

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

private:
    QLayout* historyLayout;
    QSpacerItem* historySpacer;
    QList<imageEntry*> historyList;

    CoinDialog *coinDialog;
};

#endif // MAINWINDOW_H
