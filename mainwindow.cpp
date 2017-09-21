#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
//#include <QPrintDialog>

#include <ctime>

#include "mainwindow.h"
#include "graphicswidget.h"

#include "graph.h"


GMainWindow::GMainWindow( QWidget *parent)
    : QMainWindow( parent, Qt::FramelessWindowHint)
{
    storage::init();

//    setWindowState( windowState() | Qt::WindowFullScreen);
//    int w = width(), h = height();
//    if( w > 3000 || h > 2000){
//        w = qMin( width(), 3000);
//        h = qMin( height(), 2000);
//        setWindowState( windowState() ^ Qt::WindowFullScreen);
//        setWindowTitle( "Star's Battle");
//    }

//    int w = 1900, h = 1000;
    int w = 900, h = 600;
//    int w = 600, h = 900;
//    int w = 640, h = 480;
    setMinimumSize( w, h);

    scene = new QGraphicsScene( 0, 0, w, h);

    pGraphics = new GameWidget( scene, this);
    pGraphics->w = w; pGraphics->h = h;

    pGraphics->setCacheMode( QGraphicsView::CacheBackground);
//    pGraphics->setViewportUpdateMode( QGraphicsView::BoundingRectViewportUpdate);
    pGraphics->setViewportUpdateMode( QGraphicsView::FullViewportUpdate);
    pGraphics->setRenderHint( QPainter::Antialiasing);
//    pGraphics->setTransformationAnchor( QGraphicsView::AnchorUnderMouse);

    pGraphics->setDragMode( QGraphicsView::NoDrag);//NoDrag ScrollHandDrag
    pGraphics->setFocusPolicy( Qt::StrongFocus);
    pGraphics->setFocus();

    pGraphics->setMinimumSize( w, h);

    pGraphics->setSceneRect( 0, 0, w, h);

    connect( pGraphics, SIGNAL( clickedQuit()), this, SLOT( close()));

    pGraphics->initView();

    setCentralWidget( pGraphics);
    pGraphics->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff);
    pGraphics->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff);
}


void GMainWindow::closeEvent( QCloseEvent *event)
{
    pGraphics->finitView();
    event->accept();
}

GMainWindow::~GMainWindow()
{
    delete pGraphics;
    delete scene;
    qDebug("----- storage: N8 = %d (DEL = %d).   N64 = %d (DEL = %d)", storage::getN8(), storage::getN8del(), storage::getN64(), storage::getN64del());
    storage::finit();
}


int    GMainWindow::start()
{
    int res = 0;
    if( pGraphics && pGraphics->getView()){
        res = pGraphics->mainMenu( 0xFF0FFF00);
    }
    return res;
}


