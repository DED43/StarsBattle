#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QGraphicsScene>
#include <QTreeWidget>


#include <math.h>
#include "graphicswidget.h"
#include "graph.h"

#include "ds.h"


static const QString VERSION = "1.0";

using namespace std;


class GMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    GMainWindow(QWidget *parent = 0);
    ~GMainWindow();

    GameWidget *pGraphics;
    int start();

public slots:

protected:
    void closeEvent( QCloseEvent*);


signals:


private:
    QGraphicsScene *scene;

};


#endif // MAINWINDOW_H
