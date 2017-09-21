#include "mainwindow.h"
#include <QApplication>

int main( int argc, char *argv[])
{
    QApplication a( argc, argv);

    GMainWindow w;
    int result = 0;
    w.show();
    if( w.start() > 0) result = a.exec();
    return( result);
}

