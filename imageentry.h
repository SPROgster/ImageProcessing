#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include <QImage>
#include <QLabel>
#include <QWidget>
#include <QPainter>

#define THUMBW 100
#define THUMBH 75

class imageEntry : public QWidget
{
    Q_OBJECT
public:
    explicit imageEntry(QWidget *parent = 0, QImage* newImage = 0, const QString &text = '\0');
    ~imageEntry();

    void setLabel(const QString &newText);
    void setImage(const QImage* newImage);

    void setSelected(bool isSelected);

    QImage* getImage();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage* image;
    QImage* thumbnail;
    QLabel* label;
};

#endif // IMAGEENTRY_H
