
#include "mainwindow.h"
#include "graphicswidget.h"

#include <math.h>

#include <QMessageBox>
#include <QKeyEvent>
#include <QUrl>



GameWidget::GameWidget( QGraphicsScene *scene, GMainWindow* par) :
    QGraphicsView( scene, par), timerId(0), timerFreq(25)
{
    setScene(scene);
    setContentsMargins( 0, 0, 0, 0);

    clickedNode = 0;
    markedNode = 0;
    pLinkBegin = 0;

    objName = new GLineObjName;
    scene->addItem( objName);
    objName->setPlainText( "...");

    gameName = new QGraphicsTextItem;
    scene->addItem( gameName);
    gameName->setFont( QFont( "Times", 12, QFont::DemiBold, false));
    gameName->setDefaultTextColor( Qt::darkRed);
    gameName->setZValue( VIEW_Z_NAMES);
    gameName->setPlainText( "Level: 1");

    infoText = new GInfoText;
    scene->addItem( infoText);

    menuButton = new GMenuButton( "M");
    scene->addItem( menuButton);

    actButton = new GActionButton;
    scene->addItem( actButton);
    actButton->link = new GActionLink;
    scene->addItem( actButton->link);
    actButton->reset();

    pBckgrnd = new QImage( ":/images/pic1.jpg");

    pPlayer = new QMediaPlayer;

    paused = false;

    connect( this, SIGNAL( startGame()), this, SLOT( gameBegins()));
    connect( this, SIGNAL( gameFinished( int)), this, SLOT( finishGame( int)));

    timerFreqAverage = timerFreq;
    screenSize = SCREEN_SCALE_BASE;
    windowResized();
}


GameWidget::~GameWidget()
{
    delete pView;
    delete pBckgrnd;
    delete pPlayer;
}


void GameWidget::initView()
{
    setScaleLevel( 0);

    pView = new GamePlay( this);

    if( pPlayer){
        pPlayer->setMedia( QUrl::fromLocalFile("S:/m1.mp3"));
        pPlayer->setVolume( 50);
        pPlayer->play();
    }
}


void GameWidget::finitView()
{
    if( pView && timerId){
        gameStops();
        saveGame();
        closeGame();
    }
}


void GameWidget::startMainMenu()
{
    gameStops(); paused = true;
    mainMenu();
    paused = false;
}


int GameWidget::mainMenu( quint32 profile)
{
    int res = -1;
    if( pView){
        MainMenu mm( this, profile);
        if( mm.pContinue && mm.pNewGame){
            if( !paused && !pView->pGames[0]){
                mm.pContinue->setEnabled( false);
                mm.pNewGame->setDefault( true);
            }
        }
        if( mm.pLoadGame){
            int count = 0;
            for( int i = 1; i <= GAME_SAVE_SLOTS; ++i)
                if( pView->pGames[i]) count ++;
            if( !count)
                mm.pLoadGame->setEnabled( false);
        }
        menuButton->setVisible( false);

        while( res < 0){
            switch( mm.exec()){
            case 1:   qDebug( "GW: Continue...");
                res = 1;
                if( !paused) loadGame();
                break;
            case 2:   qDebug( "GW: New Game");
                res = 1;
                if( paused) closeGame();
                newGame();
                break;
            case 3:   qDebug( "GW: Save Game");
                {
                    SaveLoadMenu slm( 0, pView->pGames, false);
                    int _ind = slm.exec();
                    if( _ind){
                        res = 1;
                        pView->setName( slm.name);
                        saveGame( _ind);
                    }
                }
                break;
            case 4:   qDebug( "GW: Load Game");
                {
                    SaveLoadMenu slm( 0, pView->pGames, true);
                    int _ind = slm.exec();
                    if( _ind){
                        res = 1;
                        if( paused) closeGame();
                        loadGame( _ind);
                    }
                }
                break;
            case 5:   qDebug( "GW: Edit Settings");
                res = (paused)? 1 : 0;
                break;
            case 6:   qDebug( "GW: Quit Game");
                res = 0; saveGame();
                if( paused) emit clickedQuit();
                break;
            default: qDebug( "GW: Default");
                res = (paused)? 1 : 0;
            }
        }
        menuButton->setVisible( true);
    }
    if( res > 0) emit gameBegins();
    return res;
}


void GameWidget::keyPressEvent(QKeyEvent *event)
{
    bool sysKey = event->modifiers() & (Qt::ControlModifier | Qt::AltModifier);
    if( !sysKey){
        QGraphicsView::keyPressEvent (event);
        if (event->isAccepted()) return;
    }

//    qDebug ("GW: keyPressEvent()");
    switch( event->key()){
    case Qt::Key_Delete:
        break;
    case Qt::Key_N:
        if( event->modifiers() & Qt::ControlModifier){
            if( pView){ qDebug ("GW: New Game");
                gameStops();
                closeGame(); thread()->msleep(100);
                newGame();
                emit gameBegins();
            }
        }
        break;
    case Qt::Key_P:
        if( event->modifiers() & Qt::ControlModifier){
            paused = !paused;
        }
        break;
    case Qt::Key_L:
        if( event->modifiers() & Qt::ControlModifier){
        }
        break;
    case Qt::Key_S:
        if( event->modifiers() & Qt::ControlModifier){
        }
        break;
    case Qt::Key_Escape:
        gameStops(); paused = true;
        mainMenu();
        paused = false;
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        break;
    case Qt::Key_Shift:
        break;
    default:
        if( objName->node && !sysKey){
            objName->setPlainText("");
            objName->setFocus();
            QGraphicsView::keyPressEvent(event);
        }
    }
}

/**
    Mouse events
*/
void GameWidget::mousePressEvent( QMouseEvent *e)
{
    if( e->button()==Qt::RightButton){
        QGraphicsItem *item= itemAt( e->pos());
        if (!markedNode && !item) {
//            addNode( e->pos(), true);
            return;
        }
    }else if( e->button() == Qt::LeftButton){
        if( itemAt( e->pos())){
            if( qgraphicsitem_cast<GMenuButton *>( itemAt( e->pos())))
                startMainMenu();
            else if( qgraphicsitem_cast<GActionButton *>( itemAt( e->pos()))){
                addLink( actButton->nodeFrom(), actButton->nodeTo());
                actButton->reset();
            }
        }else{
            nodeClicked( NULL);
        }
        if( scaleLevel){
            shiftVector = e->globalPos();
        }
    }
    QGraphicsView::mousePressEvent( e);
}


void GameWidget::mouseReleaseEvent( QMouseEvent *e)
{
    qDebug("GW: mouseReleaseEvent()");
    if( e->button() == Qt::RightButton){
        if( pLinkBegin){
            aiGNode *_node = 0;
            QList<QGraphicsItem *> lItems = items( e->pos());
            foreach( QGraphicsItem *item, lItems){
                 _node = qgraphicsitem_cast<aiGNode*>(item);
                 if( _node) break;
            }
            linkCreating( _node, false);
        }
    }else if( e->button() == Qt::LeftButton){
        if( !shiftVector.isNull()){
            shiftVector.setX(0); shiftVector.setY(0);
        }
    }
    QGraphicsView::mouseReleaseEvent( e);
}


void GameWidget::mouseMoveEvent( QMouseEvent *e)
{
    QGraphicsView::mouseMoveEvent(e);
    if( pLinkBegin){
        updateLinkCreatingLine();
        linkCreatingLine.setP2( mapToScene( e->pos()));
        updateLinkCreatingLine();
//        if( infoText){
//            QString s = "Building line...";
//            s += " Length = ";
//            s+= QString().setNum( linkCreatingLine.length(), 'f', 1);
//            infoText->setText( s, 1000);
//        }
    }else if( !shiftVector.isNull()){
        shiftVector -= e->globalPos();
        QPointF lt = sceneRect().topLeft() + shiftVector;
        moveViewSceneTo( lt);
        shiftVector = e->globalPos();
    }
}



#ifndef QT_NO_WHEELEVENT
void GameWidget::wheelEvent( QWheelEvent *event)
{
//    if( pView) pView->changeSpeed( (event->delta() > 0));
    if( event->modifiers() & Qt::ControlModifier){
        int _scaleLevel = scaleLevel + ((event->delta() > 0)? 1 : -1);
        _scaleLevel = qBound( 0, _scaleLevel, SCALE_COUNTER_MAX);
        if( scaleLevel != _scaleLevel){
            qreal coefRel = 1.0 / getScaleFactor();
            setScaleLevel( _scaleLevel);
            qreal coefAbs = getScaleFactor();
            coefRel *=  coefAbs;
            qDebug( "GW: scale k = %d", scaleLevel);

            QPoint    posAnchor = event->pos();
            QPointF posAnchorScene = mapToScene( posAnchor);
            scene()->setSceneRect( 0, 0, coefAbs * width(), coefAbs * height());
            pView->redrawStars( coefAbs);
            moveViewSceneTo( posAnchorScene * coefRel - posAnchor);
        }
    }
}
#endif

void GameWidget::moveViewSceneTo( QPointF lt)
{
    QPointF       wh = QPointF( width(), height());
    QRectF         viewNewRect = QRectF( lt, lt + wh);
    QPointF       pL = scene()->sceneRect().bottomRight() - wh;
//    qDebug( "                     posPL = (%f, %f)", pL.x(), pL.y());
    viewNewRect.moveTo(
                qBound( 0.0, viewNewRect.left(), pL.x()),
                qBound( 0.0, viewNewRect.top(), pL.y()));
//    qDebug( "           posLeftTop = (%f, %f)", viewNewRect.x(), viewNewRect.y());
    setSceneRect( viewNewRect);
    centerOn( sceneRect().x() + width() / 2, sceneRect().y() + height() / 2);

    windowResized();
}


void GameWidget::drawBackground( QPainter *painter, const QRectF &rect)
{
    if( pBckgrnd){
        QRectF rect1 = rect;
        rect1.moveTo( 100, 50);
        painter->drawImage( rect, *pBckgrnd, rect1);
    }else{
        // Fill
        QLinearGradient gradient( rect.topLeft(), rect.bottomRight());
        gradient.setColorAt( 0, Qt::darkGray);
        gradient.setColorAt( 1, Qt::darkGray);
        painter->fillRect( rect, gradient);
        painter->setBrush( Qt::NoBrush);
        painter->drawRect( rect);
    }
}


void GameWidget::drawForeground( QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)
    if( pLinkBegin){
        painter->setPen( QColor(Qt::red).lighter(180));
        painter->drawLine( linkCreatingLine);
    }
}


void GameWidget::resizeEvent( QResizeEvent *e)
{
    Q_UNUSED(e)
    QGraphicsView::resizeEvent( e);
    windowResized();
    scaleLevel = 0;
    qreal coefAbs = getScaleFactor();
    scene()->setSceneRect( 0, 0, coefAbs * width(), coefAbs * height());
    pView->redrawStars( coefAbs);
    moveViewSceneTo( QPointF(0,0));
}


//aiGNode* GameWidget::addNode( QPointF p, bool isMarked)
//{
//    QString _name = "N: ";
//    _name += QString().setNum( pView->nNodes() + 1);
//    aiGNode *pNode = new aiGNode( this, _name);
//    scene()->addItem( pNode);
//    pNode->setPos( p);
//    pView->addNode( pNode);
//    if( isMarked) nodeMarked( pNode, true);
//    return pNode;
//}


void           GameWidget::addLink( aiGNode* pN1, aiGNode* pN2)
{
    aiGLink *pLink = 0;
    if( pN1 && pN1->isCommonStar() && pN2 && pN2->isCommonStar()){
        pLink = pN1->findConnection( pN2); //try to find among existing links
        if( !pLink){
            pLink = new aiGLink( pN1, pN2);
            scene()->addItem( pLink);
            pView->addLink( pLink);
        }
        if( pLink && pLink->setBuildingLink( pN1)){
            qreal m = (qreal) -pView->massBuildLink(
                        pN1->index(), pN2->index(), pN1->getMass()) / 1000000.0;
            quint32 t = pView->timeBuildLink(
                        pN1->index(), pN2->index(), pN1->getMass()) / getTimerFreq();
            QString txt = "Mass: " +
                    QString().setNum( m, 'f', ( m<9.95)? 2 : 1) +
                    "; Time: " + QString().setNum( t) + " s.";
            QString s = "";
            int duration = 400;
            switch( pN1->team() * 10 + pN2->team()){
            case 01: s += "You attacked GREEN. "; break;
            case 02: s += "You attacked BLUE. "; break;
            case 10: s += "You were attacked by GREEN. "; break;
            case 20: s += "You were attacked by BLUE. "; break;
            case 12: s += "GREEN attacked BLUE. " ; duration = 200; break;
            case 21: s += "BLUE attacked GREEN. " ; duration = 200; break;
            default:;
            }
            if( !s.isEmpty()){
                s += txt;
                infoText->addText( s, duration);
            }
        }
    }
}


void           GameWidget::remNode( aiGNode *pNode)
{
    pView->delNode( pNode);
    remNode_Footprints( pNode);
//    scene()->removeItem( pNode);
    delete pNode;
}
void           GameWidget::remNode_Footprints( aiGNode *node)
{
    bool _updInfoNode = false;
    if( clickedNode == node){ clickedNode = 0; _updInfoNode = true;}
    if( markedNode == node){ markedNode = 0; _updInfoNode = true;}
    if( _updInfoNode) updInfoName();
    actButton->reset( node);
}

void           GameWidget::remLink( aiGLink *pLink)
{
    pView->delLink( pLink);
//    scene()->removeItem( pLink);
    delete pLink;
}


void           GameWidget::nodeClicked( aiGNode *pN)
{
    if( clickedNode && !clickedNode->isCommonStar()) clickedNode = 0;
    if( pN && !pN->isCommonStar()) pN = 0;

    if( actButton->isActive()){
        if( pN == actButton->nodeTo()){
            addLink( actButton->nodeFrom(), actButton->nodeTo());
            pN = 0;
        }
        actButton->reset();
    }
    if( clickedNode != pN){
        if( clickedNode && clickedNode->team() == 0 && pN){
            aiGNode *pN0 = clickedNode;
            clickedNode = 0; pN0->update();
            if( pN0->isLinkPossibleTo( pN))
                actButton->set( pN0, pN);
        }else{
            if( clickedNode) clickedNode->update();
            clickedNode = pN;
            if( clickedNode) clickedNode->update();
        }
    }else{
        clickedNode = 0;
    }
    updInfoName();
}

void           GameWidget::nodeMarked( aiGNode *pN, bool fSet)
{
    markedNode = (fSet) ? pN : NULL;
    if( pN) pN->update();
    updInfoName();
}


void           GameWidget::linkCreating( aiGNode *pN, bool fStart)
{
    if( fStart){
        pLinkBegin = pN;
        linkCreatingLine.setP1( pN->scenePos());
        linkCreatingLine.setP2( pN->scenePos());
        setCursor( Qt::CrossCursor);
        updateLinkCreatingLine();
    }else if( pN && (pLinkBegin!=pN)){
        addLink( pLinkBegin, pN);
        nodeMarked( pN, true);
        pLinkBegin->update();
        pLinkBegin = 0;
        pN->update();
        unsetCursor();
    }else{
        updateLinkCreatingLine();
        pLinkBegin->update();
        pLinkBegin = 0;
        unsetCursor();
    }
}


void            GameWidget::updateLinkCreatingLine()
{
    QRectF rect = QRectF( linkCreatingLine.p1(), linkCreatingLine.p2()).normalized();
    scene()->invalidate( rect, QGraphicsScene::ForegroundLayer);
}


void            GameWidget::updInfoName()
{
    objName->setNode( (markedNode)? markedNode : clickedNode);
}


void           GameWidget::windowResized()
{
    if( gameName){ gameName->setPos( mapToScene( 10, 10)); gameName->update();}
    if( infoText){
        infoText->setPos( sceneRect().left() + 5, sceneRect().bottom() - infoText->h - 5);
        infoText->update();
    }
    if( menuButton){
        menuButton->setPos( sceneRect().right() - 10, sceneRect().top() + 10);
        menuButton->update();
    }
    if( actButton){
        actButton->setPos( sceneRect().right() - 10, sceneRect().top() + 10 + 2 * menuButton->r + 20);
        if( actButton->link) actButton->link->adjust();
        actButton->update();
    }

    if( objName){ objName->setPos( mapToScene( 250, 2)); objName->update();}
}


void           GameWidget::newGame()
{
    qDebug( "GW: newGame()");
    if( pView){
        pView->newGame( 3, 10);
        addObjectsToScene();
        pView->redrawStars( 1.0);
        if( infoText) infoText->addText( "New Game begins...", 200);
    }
}

bool           GameWidget::loadGame( int n)
{
    qDebug( "GW: loadGame( %i)", n);
    bool res = false;
    if ( pView){
        res = pView->loadGame( n);
        if( res){
            addObjectsToScene();
            pView->redrawStars( 1.0);
            if( infoText) infoText->addText( "Game <" + pView->getName() +"> loaded...", 100);
        }
    }
    return res;
}

void           GameWidget::saveGame( int n)
{
    qDebug( "GW: saveGame( %i)", n);
    if( pView){
        pView->saveGame( n);
        if( infoText) infoText->addText( "Game <" + pView->getName() +"> saved...", 100);
    }
}

void           GameWidget::closeGame()
{
    qDebug( "GW: closeGame()");
    if( pView){
        clickedNode = markedNode = 0;
        updInfoName();
        if( pLinkBegin) linkCreating( 0, false);
        foreach( QGraphicsItem *item, items()){
             if( aiGLink *_link = qgraphicsitem_cast<aiGLink*>(item))
                 remLink( _link);
        }
        foreach( QGraphicsItem *item, items()){
             if( aiGNode *_node = qgraphicsitem_cast<aiGNode*>(item))
                 remNode( _node);
        }
        if( scaleLevel){
            setScaleLevel( 0);
            scene()->setSceneRect( 0, 0, width(), height());
            moveViewSceneTo();
        }
        qDebug ("                                 Remains %d items on scene()", scene()->items().size());
        pView->closeGame();
        update();
    }
}

void           GameWidget::gameBegins()
{
    qDebug( "GW: gameBegins()");
    paused = false;
    if( !timerId) timerId = startTimer( 1000 / timerFreq);
    timerCounter = 0; timerTest.start();
}
void           GameWidget::gameStops()
{
    killTimer( timerId); timerId = 0;
    if( timerCounter > 10)
        timerFreqAverage = (qreal)timerCounter / timerTest.elapsed() * 1000;
    qDebug( "GW: gameStops().     Frequency: %f Hz", timerFreqAverage);
    if( pView){
        if( infoText) infoText->clear();
        pView->finishWaitingSteps( true);
        if( pView->isWaitingSteps) qDebug( "GW: gameStops(). Remain undeleted items");
    }
}

void           GameWidget::timerEvent( QTimerEvent *event)
{
    Q_UNUSED(event)
    int gameState = -10;
    if( pView && timerId && !paused)
        if( timerId == event->timerId()){
            if( ++timerCounter > 5) timerFreqAverage =
                    0.9 * timerFreqAverage +
                    0.1 * (qreal)timerCounter / timerTest.elapsed() * 1000;
            if( timerCounter > 1000){
//                qDebug( "     Frequency: %f Hz", timerFreqAverage);
                timerCounter = 0; timerTest.start();
            }
            gameState = pView->step();
            if( gameState != -10){
                gameStops();
                emit gameFinished( gameState);
            }else{
                if( infoText) infoText->updateCounters();
            }
        }
}

void           GameWidget::finishGame( int state)
{
    QString s = "";
    switch( state){
    case -1:
        s = "DRAW! \n";
        break;
    case 0:
        s = "RED player wins! \n";
        break;
    case 1:
        s = "GREEN player wins! \n";
        break;
    case 2:
        s = "BLUE player wins! \n";
        break;
    default:;
    }

    closeGame();
    pView->aiDel_game(0);

    {
        QMessageBox msgBox;
        msgBox.setWindowFlags( Qt::FramelessWindowHint);
        msgBox.setFont( QFont( "Times", 14, QFont::Light, false));
        msgBox.setWindowOpacity( 0.67);

        msgBox.setText( "GAME OVER!");
        msgBox.setInformativeText( s);
        msgBox.setStandardButtons( QMessageBox::Ok);
        msgBox.setDefaultButton( QMessageBox::Ok);
        msgBox.exec();
    }

    if( !mainMenu( 0x0F0FFF00))
        emit clickedQuit();
}


void          GameWidget::addObjectsToScene()
{
    QVector<aiGNode*>::const_iterator itNodes = pView->pStars.begin();
    while( itNodes != pView->pStars.end()){
        scene()->addItem( *itNodes);
        ++itNodes;
    }
    QList<aiGLink*>::const_iterator itLinks = pView->lLinks.begin();
    while( itLinks != pView->lLinks.end()){
        scene()->addItem( *itLinks);
        ++itLinks;
    }
}


void          GameWidget::aiLink_add( quint32 dat)
{
    if( pView && timerId)
        pView->aiActions.enqueue( dat);
}

