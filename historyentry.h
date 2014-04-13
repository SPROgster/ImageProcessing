#ifndef HISTORYENTRY_H
#define HISTORYENTRY_H

#include <QWidget>

class historyEntry : public QWidget
{
    Q_OBJECT
public:
    explicit historyEntry(QWidget *parent = 0, QImage* newImage = 0);
    ~historyEntry();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // HISTORYENTRY_H
