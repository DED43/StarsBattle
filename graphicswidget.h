#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H


#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtMultimedia>

#include "ds.h"
#include "menu.h"


#define     SCALE_COUNTER_MAX       20
#define     SCALE_FACTOR_MAX          2.0

#define     SCREEN_SCALE_BASE    150.0


class aiGNode;
class aiGLink;
class GamePlay;
class GMainWindow;


 class GameWidget : public QGraphicsView
{
    Q_OBJECT
public:
    GameWidget( QGraphicsScene*, GMainWindow*);
    ~GameWidget();

    aiGNode       *clickedNode;
    aiGNode       *markedNode;
    aiGNode       *pLinkBegin;

    QGraphicsTextItem  *gameName;
    GLineObjName          *objName;
    GInfoText                     *infoText;
    GMenuButton            *menuButton;
    GActionButton           *actButton;

    inline void              setScaleLevel( int level){ scaleLevel = level; setScaleFactor();}
    inline qreal            getScaleFactor(){ return scaleFactor;}
    inline qreal            getScreenFactor(){ return( screenSize / SCREEN_SCALE_BASE);}
    qreal                        screenSize;
    int                             w, h;

    void           initView();
    int              mainMenu( quint32 profile = 0xFFFFF000);
    void           finitView();
    inline GamePlay *getView(){ return pView;}

//    aiGNode* addNode( QPointF, bool);
    void           addLink( aiGNode* pN1, aiGNode* pN2);
    void           remNode( aiGNode*);
    void           remNode_Footprints( aiGNode*);
    void           remLink( aiGLink*);

    void newGame();
    void saveGame( int n = 0);
    bool loadGame( int n = 0);
    void closeGame();

    void gameStops();
    inline qreal getTimerFreq(){ return timerFreqAverage;}

public slots:
    void nodeClicked( aiGNode*);
    void nodeMarked( aiGNode*, bool);
    void linkCreating( aiGNode*, bool);

    void gameBegins();
    void aiLink_add( quint32);
    void finishGame( int state);

    void startMainMenu();


signals:
    void userRightClicked( QPointF, bool);
    void startGame();
    void beginAIStep0();
    void beginAIStep1();
    void gameFinished( int state);
    void clickedQuit();


protected:
    void keyPressEvent( QKeyEvent*);
    void timerEvent( QTimerEvent*);

    void mousePressEvent( QMouseEvent*);
    void mouseReleaseEvent( QMouseEvent*);
    void mouseMoveEvent( QMouseEvent*);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent*);
#endif
    void drawBackground( QPainter*, const QRectF&);
    void drawForeground( QPainter*, const QRectF&);

    void resizeEvent( QResizeEvent*);

private:
    GamePlay          *pView;
    QImage                  *pBckgrnd;

    int                 timerId;
    int                 timerFreq;
    qreal            timerFreqAverage;
    int                 timerCounter;
    QTime         timerTest;

    bool              paused;

    void addObjectsToScene();

    QLineF        linkCreatingLine;
    void             updateLinkCreatingLine();
    QPoint        shiftVector;
    void             moveViewSceneTo( QPointF lt = QPointF( 0, 0));

    int                             scaleLevel;
    qreal                        scaleFactor;
    inline void              setScaleFactor(){ scaleFactor = pow( SCALE_FACTOR_MAX, (qreal)scaleLevel / SCALE_COUNTER_MAX);}

    void updInfoName();
    void windowResized();

    QMediaPlayer *pPlayer;

};




#endif
