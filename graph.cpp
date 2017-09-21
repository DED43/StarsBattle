
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include <cmath>		//allows the use of pow(float/double, float/double) function
#include <cstdlib>		//allows the use of RAND_MAX macro 

#include <QPointF>
#include <QDebug>		//used for qDebug messages
#include <QDateTime> 	//
#include <list>			        //for list iterators
#include <queue>		    //for BFS queue Q


#include "graph.h"
#include "ai.h"



/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for aiNode
   *
 * -----------------------------------------------------------------------------------------------
 */
aiGNode::aiGNode( GameWidget *_gw, QString name)
{
    oName = 0;
    reset(); gw = _gw;
    init();
    setName( name);
}

aiGNode::aiGNode( GameWidget *_gw, aiid _id)
{
    oName = 0;
    reset(); gw = _gw;
    init();
    aiLoad( _id);
    updStarView();
}

void aiGNode::init()
{
    setZValue( VIEW_Z_BASE);
//    setFlag (ItemIsMovable);
    setFlag( ItemSendsGeometryChanges);
    setCacheMode( DeviceCoordinateCache);
    setAcceptHoverEvents( true);

    oName = new QGraphicsTextItem( "", this);
    if( oName){
        oName->setFont( QFont( "Times", 9, QFont::Light));
        oName->setDefaultTextColor( Qt::white);
//        oName->setDefaultTextColor (Qt::darkBlue);
        oName->setPos( 2, 2);
        oName->setZValue( VIEW_Z_NAMES);
        oName->setAcceptHoverEvents( false);
    }
}

aiGNode::~aiGNode()
{
}

void aiGNode::setName( QString name)
{
    if( oName && oName->toPlainText() != name){
        prepareGeometryChange();
        oName->setPlainText( name);
    }
}

aiid aiGNode::aiSave()
{
    save_name( getName());
    return( NodeBase::aiSave());
}
void aiGNode::aiLoad( aiid _id)
{
    NodeBase::aiLoad( _id);
    setName( load_name());
    NodeBase::aiSetObj( this);
}


QRectF aiGNode::boundingRect() const
{
    qreal adjust = 1;
    return QRectF( -r - adjust, -r - adjust, 2 * (r + adjust), 2 * (r + adjust));
}

QPainterPath aiGNode::shape() const
{
    QPainterPath path;
        path.addEllipse (-r, -r, 2*r, 2*r);
    return path;
}

void aiGNode::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    int r1 = r/2 - 2;
    painter->setPen( Qt::NoPen);
    if( clrShadow != Qt::transparent){
        painter->setBrush( clrShadow);
        painter->drawEllipse( -r1 - 9, -r1 - 9, 2*r, 2*r);
    }

    QRadialGradient gradient( -r1, -r1, r);
//    if( (gw->clickedNode == this) || (gw->markedNode == this)){
    if( isCommonStar() && gw->clickedNode == this){
        gradient.setCenter( r1, r1);
        gradient.setFocalPoint( r1, r1);
        gradient.setColorAt( 1, clrObj.lighter( 150));
        gradient.setColorAt( 0, clrObj.darker().lighter( 120));
    }else{
        gradient.setColorAt( 0, clrObj);
        gradient.setColorAt( 1, clrObj.darker());
    }
    painter->setBrush( gradient);
    painter->drawEllipse( -r, -r, 2*r, 2*r);
}

QVariant aiGNode::itemChange( GraphicsItemChange change, const QVariant &value)
{
    switch( change){
    case ItemPositionChange:
        if( scene()){
            // value is the new position.
            QPointF newPos = value.toPointF();
            qreal d = 5;
            QRectF rect = scene()->sceneRect().adjusted (d,d,-d,-d);
            if( !rect.contains( newPos)){
                // Keep the item inside the scene rect.
                newPos.setX( qBound( rect.left(), newPos.x(), rect.right()));
                newPos.setY( qBound( rect.top(), newPos.y(), rect.bottom()));
                return newPos;
            }
        }
        break;
    case ItemPositionHasChanged:
        adjustLinks();
        break;
    default:
        break;
    }

    return QGraphicsItem::itemChange( change, value);
}

/** handles the events of a click on a node */
void aiGNode::mousePressEvent( QGraphicsSceneMouseEvent *event)
{
    if( event->button()==Qt::LeftButton) {
        gw->nodeClicked( this);
        return;
    }
    if( event->button()==Qt::RightButton ){
        gw->linkCreating( this, true);
        return;
    }
    if( event->button()==Qt::MidButton){
    }

    update();
    QGraphicsItem::mousePressEvent( event);
}

void aiGNode::mouseReleaseEvent( QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent( event);
}

void aiGNode::hoverEnterEvent( QGraphicsSceneHoverEvent * event)
{
    Q_UNUSED(event);
    gw->nodeMarked( this, true);
}

void aiGNode::hoverLeaveEvent( QGraphicsSceneHoverEvent * event)
{
    Q_UNUSED(event);
    gw->nodeMarked( this, false);
}


void aiGNode::adjustLinks()
{
    foreach( aiGLink *pLink, lLinks){
        pLink->adjust();
    }
}


aiGLink* aiGNode::findConnection( aiGNode *pNode)
{
    aiGLink *res = 0;
    foreach( aiGLink *pLink, lLinks) {
        if( pLink && pLink->isLinking( this, pNode)){
            res = pLink; break;
        }
    }
    return res;
}

bool         aiGNode::isLinkPossibleTo( aiGNode *node)
{
    bool res = false;
    if( isCommonStar() && node && node->isCommonStar()){
        aiGLink *link = findConnection( node);
        if( !link ||
                (link && !link->isDrainingLink() && !link->isBuildingLink( this, node)))
            res = true;
    }
    return res;
}



void aiGNode::updStarType(){
    mType &= ~0x00000F00;
    if( isCommonStar()){
        if( mParam > 20000000){
            mType ^= 0x00000030;
            mParam = 25000000;
            dmClear();
        }else if( mParam <= 0){
            mType ^= 0x00000050;
            dmClear();
            setVisible( false);
        }else{
            mType += GameCommon::getMassRank( mParam) * 0x00000100;
        }
    }
}

void aiGNode::stepFinit()
{
    quint32 mTypeBefore = mType;
    QString s = "";

    mParam += dmDecay + dmBL + dmDrain;
    updStarType();

    if( isCommonStar()){
        qreal m = (qreal)mParam / 1000000.0;
        s = QString().setNum( m, 'f', ( m<9.95)? 1 : 0);
    }

    if( mTypeBefore != mType)
        updStarView();

    setName( s);
}


void aiGNode::updStats()
{
    mBL = 0;
    if( isCommonStar()){
        foreach( aiGLink *pLink, lLinks){
            if( pLink && pLink->isBuildingLink()){
                if( pLink->pNode1 == this)
                    mBL += abs( pView()->stepBuildingM( pLink->prof1())) * pLink->getBuildingLinkDT();
                else if( pLink->pNode2 == this)
                    mBL += abs( pView()->stepBuildingM( pLink->prof2())) * pLink->getBuildingLinkDT();
            }
        }
    }
    if( mBL < 0 )
        qDebug("                                      --- mBL ERROR");
}



int   radiusStar[0x0010] = {0,
                             8,
                             10,
                             12,
                             14,
                             16,
                             0,0,0,0,0,0,0,0,0,0};
int   lightnessStar[0x0010] = {0,
                             100,
                             110,
                             120,
                             130,
                             140,
                             0,0,0,0,0,0,0,0,0,0};

void aiGNode::updStarView()
{
    QColor clr_name = Qt::white;
    clr_name = clr_name.darker( 170);

    prepareGeometryChange();
    if( isDarkHole()){
        r = 4;
        clrObj = Qt::white;//Qt::black;
        clrShadow = Qt::transparent;
        setName("");
    }else if( isCommonStar()){
        switch( team()){
        //Teams
        case 1:
            clrObj = Qt::green;
            break;
        case 2:
            clrObj = Qt::blue;
            break;
        default:
            clrObj = Qt::red;
            break;
        }
        clrObj = clrObj.lighter( lightnessStar[ prof()]);
        r = radiusStar[ prof()];
        clrShadow = Qt::transparent;
        if( oName) oName->setDefaultTextColor (clr_name);
    }
}



/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for aiLink
   *
 * -----------------------------------------------------------------------------------------------
 */

aiGLink::aiGLink()
{
    reset();
    init();
}

aiGLink::aiGLink( aiGNode *pN1, aiGNode *pN2)
{
    reset();
    init();
    connect( pN1, pN2);
}

aiGLink::aiGLink( aiid _id)
{
    reset();
    init();
    aiLoad( _id);
}

aiGLink::~aiGLink()
{
}

void aiGLink::init()
{
    aiSetData( 3, &length);
    aiSetData( 4, &length12);
    aiSetData( 5, &length21);

    setAcceptedMouseButtons(0);
    setZValue( VIEW_Z_LINK);
}


aiid aiGLink::aiSave()
{
    idObj1 = (pNode1)? pNode1->getCurrID() : 0;
    idObj2 = (pNode2)? pNode2->getCurrID() : 0;
    return( LinkBase::aiSave());
}

bool aiGLink::aiLoad( aiid _id)
{
    bool res = LinkBase::aiLoad( _id);
    if( res){
        aiGNode *pN1 = qgraphicsitem_cast<aiGNode *> ((aiGNode *)storage::getObj( idObj1));
        aiGNode *pN2 = qgraphicsitem_cast<aiGNode *> ((aiGNode *)storage::getObj( idObj2));
        connect( pN1, pN2);
    }
    return res;
}


void aiGLink::adjust()
{
    l.clear();
    prepareGeometryChange();
    if( pNode1 && pNode2){
        l << mapFromItem( pNode1, 0, 0);
        l << mapFromItem( pNode2, 0, 0);
    }
}

QRectF aiGLink::boundingRect() const
{
    if( pNode1 && pNode2){
        qreal extra = 1;
        return l.boundingRect()
            .normalized()
            .adjusted( -extra, -extra, extra, extra);
    }
    return( QRectF());
}

void aiGLink::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if( pNode1 && pNode2){
        if( isDrainingLink()){
            QColor clr = Qt::yellow;
            clr.setAlpha (250);
            painter->setPen( QPen( clr, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPolygon( l);
        }else if( isBuildingLink()){
            QColor clr = Qt::magenta;
            clr.setAlpha (50);
            painter->setPen( QPen( clr, 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPolygon( l);
            if( length12){
//                if( !(mType & 0x000000F0)) length12 = 1000 * (pNode1->getRadius() - 1);
                clr = Qt::yellow; clr.setAlpha (150);
                QPointF p1 = mapFromItem( pNode1, 0, 0);
                QPointF p2 = mapFromItem( pNode2, 0, 0);
                p2 = p1 + (p2 - p1) * length12 / length;
                painter->setPen( QPen( clr, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->drawLine( p1, p2);
            }
            if( length21){
//                if( !(mType & 0x00000F00)) length21 = 1000 * (pNode2->getRadius() - 1);
                clr = Qt::yellow; clr.setAlpha (150);
                QPointF p1 = mapFromItem( pNode2, 0, 0);
                QPointF p2 = mapFromItem( pNode1, 0, 0);
                p2 = p1 + (p2 - p1) * length21 / length;
                painter->setPen( QPen( clr, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->drawLine( p1, p2);
            }
        }
    }
}







void aiGLink::step( GamePlay *pGame)
{
    bool change = false;
    if( pNode1 && pNode2){
        if( isDrainingLink()){ //drain
            change = true;
            qint32 m1 = pNode1->getMass();
            qint32 m2 = pNode2->getMass();
            qint32 dm = pGame->stepDraining( m2, m1);
            pNode1->stepDrain( dm);
            pNode2->stepDrain( -dm);
        }else if( isBuildingLink()){ // building link
            change = true;
            if( mType & 0x000000F0){
                pNode1->stepBL( pGame->stepBuildingM( prof1()));
                length12 += pGame->stepBuildingL( prof1());
            }
            else length12 = pNode1->getRadiusScaled();

            if( mType & 0x00000F00){
                pNode2->stepBL( pGame->stepBuildingM( prof2()));
                length21 += pGame->stepBuildingL( prof2());
            }
            else length21 = pNode2->getRadiusScaled();

            if( length <= length12 + length21){
                length12 = length21 = 0; tBL = 0;
                mType &= ~0x00000FF0; mType |= 0x00001000;
            }
            else
                setBuildingLinkDT();
        }
    }
    if( change) update();
}


bool aiGLink::setBuildingLink( aiGNode *pNfrom)
{
    bool res = false;
    if( !isToDel() && !isDrainingLink()){
        if( pNode1 == pNfrom){
            if( !(mType & 0x000000F0)){
                mType |= pNode1->getRank() * 0x00000010;
                res = true;
            }
        }else{
            if( !(mType & 0x00000F00)){
                mType |= pNode2->getRank() * 0x00000100;
                res = true;
            }
        }
        length12 = qMax( length12, pNode1->getRadiusScaled());
        length21 = qMax( length21, pNode2->getRadiusScaled());
        setBuildingLinkDT();
    }
    return res;
}

void aiGLink::setBuildingLinkDT()
{
    tBL = 1000;
    if( isBuildingLink()){
        qreal dl = pNode1->pView()->stepBuildingL( prof1()) +
                pNode1->pView()->stepBuildingL( prof2());
        if( dl) tBL = (length - length12 - length21) / dl + 1;
    }
}







/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for ViewGraph
   *
 * -----------------------------------------------------------------------------------------------
 */
GamePlay::GamePlay( GameWidget *_gw)
{
    gw = _gw;
    resetViewGraph();
    pGames.fill( 0, 1 + GAME_SAVE_SLOTS);
    if( storage::isView( GAMES_VIEW_IND)){
        storage::readV2L1( GAMES_VIEW_IND);
        for( int i = 0; i < 1 + GAME_SAVE_SLOTS; ++i)
            pGames[i] = storage::L1[i];
    }
    else
        storage::createView();
}
GamePlay::~GamePlay()
{
    if( storage::isView( GAMES_VIEW_IND)){
        for( int i = 0; i < 1 + GAME_SAVE_SLOTS; ++i)
            storage::L1[i] = pGames[i];
        storage::writeV4L1( GAMES_VIEW_IND);
    }
}

void          GamePlay::newGame( quint32 nPlayers, quint32 nStars)
{
    this->nPlayers = nPlayers;
    this->nStars     = nStars;
    qDebug( "VG: NewGame. Start. Dim = %d ", nPlayers * nStars);

    initViewGraph1();
    generateStars();
    initAI();
}

void          GamePlay::generateStars()
{
    QPointF                  center[nPlayers];

    qsrand( QTime::currentTime().msec());

    if( nPlayers == 3){
        quint32 masses[nStars];
        qreal rad = 150;
        center[0].setX( rad);
        center[0].setY( (1.0 + sqrt(3.0)) * rad);
        center[1].setX( 2.0 * rad);
        center[1].setY( rad);
        center[2].setX( 3.0 * rad);
        center[2].setY( (1.0 + sqrt(3.0)) * rad);
        qDebug( "VG: NewGame. Start. Add Stars");

        for( quint32 team = 0; team < nPlayers; ++team){
            quint32 nTotal = 0;
            quint32 n = 0;
            quint32 i=0;

            n = nStars * 3 / 10;
            for( i=0; i<n; i++) masses[i] = (500 + 1500 * qrand() / RAND_MAX) * 1000;
            nTotal += n;
            n = nStars * 3 / 10;
            for( i=0; i<n; i++) masses[nTotal+i] = (2000 + 3000 * qrand() / RAND_MAX) * 1000;
            nTotal += n;
            for( i=nTotal; i<nStars; i++) masses[i] = (5000 + 5000 * qrand() / RAND_MAX) * 1000;

            qreal dr = rad;
            dr /= nStars;
            qreal r = 0;
            qreal wr = 1.6 * dr;
            qreal phi = 0;

            idPositions.reset( team * nStars); idStateT.reset( team * nStars);

            for( i = 0; i < nStars; ++i){
                idStateT.setInc( masses[i]);
                r += dr + wr * qrand() / RAND_MAX - wr / 2.0;
                phi += 3.1415926 * 2.0 * ( 0.2 + (1.0 - 0.4) * qrand() / RAND_MAX);
                qreal x = center[team].x() + qMin( r, rad) * cos( phi);
                qreal y = center[team].y() + qMin( r, rad) * sin( phi);
                idPositions.setInc( XY2Pos( 10 * x, 10 * y));

                aiGNode *pNode = new aiGNode( gw, "");
                if( pNode){
                    pNode->setStar( team, team * nStars + i, masses[i]);
                    addNode( pNode);
                }
            }
        }
    }
}
void          GamePlay::initAI()
{
    setLengths();
    initAIPlayers();
}
void          GamePlay::setLengths()
{
    quint32 lDat = getDim();
    initLengths( lDat);

    idStateT.reset(); idPositions.reset();
    for( quint16 j = 0; j < lDat - 1; j++){
        if( idStateT.get( j)){
            quint32 x10from = 0, y10from = 0;
            Pos2XY( idPositions.get( j), &x10from, &y10from);
            for( quint16 i = j + 1; i < lDat; i++){
                if( idStateT.get( i)){
                    quint32 x10to = 0, y10to = 0;
                    Pos2XY( idPositions.get( i), &x10to, &y10to);
                    qint32 dx10 = (qint32)x10from - (qint32)x10to, dy10 = (qint32)y10from - (qint32)y10to;
                    setLength( i, j, 100 * sqrt( dx10 * dx10 + dy10 * dy10));
                }
            }
        }
    }

}
void          GamePlay::saveGame( quint32 ind)
{
    zBase += 0x00000100 * nStars + 0x00010000 * nPlayers;
    if( ind <= GAME_SAVE_SLOTS){
        aiDel_game( ind);

        lAIID      lNodesID;
        lAIID      lLinksID;
        lAIID      lCmdsID;
        idPositions.reset();

        qDebug( "    GG: Name = %s. nPlayers = %d. nStars = %d. nLinks = %d",
                mName.toLocal8Bit().constData(), nPlayers, pStars.size() - pStars.count(0), lLinks.size());

        QVector<aiGNode*>::const_iterator itNodes = pStars.begin();
        while( itNodes != pStars.end()){
            aiGNode *pNode = *itNodes;
            if( pNode){
                aiid _id = pNode->aiSave();
                qDebug( "      GG:saveNode. ID = %d. Name = %s (id = %d). Team = %d. Index = %d",
                        _id, pNode->getName().toLocal8Bit().constData(), pNode->getIdName(),
                        pNode->team(), pNode->index());
                lNodesID << _id;
                lCmdsID << idPositions.get( pNode->index());
            }
            ++itNodes;
        }

        QList<aiGLink*>::const_iterator itLinks = lLinks.begin();
        while( itLinks != lLinks.end()) {
            aiGLink *pLink = *itLinks;
            aiid _id = pLink->aiSave();
            lLinksID << _id;
            ++itLinks;
        }

        idNodes = storage::save_list( 0, &lNodesID);
        idLinks = storage::save_list( 0, &lLinksID);
        idCmds = storage::save_list( 0, &lCmdsID);
        save_name( mName);

        pGames[ind] = aiSave();
        qDebug( "      GG:saveGame. ID = %d. Name = <%s> (id = %d). idLinks = %d. idCmds = %d",
                id_curr, mName.toLocal8Bit().constData(), idName, idLinks, idCmds);
    }
    zBase &= 0x000000FF;
}

void          GamePlay::aiDel_game( quint32 ind)
{
    if( isGame( ind)){
        qDebug( "Game::aiDel_game( %d)", ind);
        GameBase::aiDel_fast( pGames[ind]);
        pGames[ind] = 0;
        qDebug( "                                               ... deleted");
    }
}

bool          GamePlay::loadGame( quint32 ind)
{
    bool res = false;
    if( isGame(ind)){
        resetViewGraph();
        res = aiLoad( pGames[ind]);
        if( res){
            nStars = (zBase / 0x00000100) & 0x000000FF;
            nPlayers = (zBase / 0x00010000) & 0x000000FF;
            zBase &= 0x000000FF;
            mName = load_name();

            initViewGraph1();
            loadNodes();
            initAI();
            loadLinks();
            qDebug( "    GG: Name = %s. nPlayers = %d. nStars = %d. nLinks = %d",
                    mName.toLocal8Bit().constData(), nPlayers, pStars.size() - pStars.count(0), lLinks.size());
        }
        else pGames[ind] = 0;
    }
    return res;
}
void          GamePlay::loadNodes()
{
    lAIID      lNodesID;
    lAIID      lCmdsID;

    storage::populate_list( idNodes, &lNodesID);
    storage::populate_list( idCmds, &lCmdsID);

    idPositions.reset();
    idStateT.reset();

    //Create Nodes
    lAIID::const_iterator itNodes = lNodesID.begin();
    lAIID::const_iterator itCmds = lCmdsID.begin();
    while( itNodes != lNodesID.end()){
        aiGNode *pNode = new aiGNode( gw, *itNodes);
        if( pNode){
            pNode->setZValue( zBase);
            quint32 index = pNode->index();
            idPositions.set( *itCmds, index);
            idStateT.set( pNode->getMass(), index);
            addNode( pNode);
        }
        ++itNodes; ++itCmds;
    }
}
void          GamePlay::loadLinks()
{
    lAIID      lLinksID;
    storage::populate_list( idLinks, &lLinksID);

    //Create Links
    lAIID::const_iterator itLinks = lLinksID.begin();
    while( itLinks != lLinksID.end()){
        aiGLink *pLink = new aiGLink( *itLinks);
        if( pLink){
            addLink( pLink);
        }
        ++itLinks;
    }
}




void         GamePlay::closeGame()
{
    finitAIPlayers();
    finitLengths();

    finitViewGraph1();
    resetViewGraph();
    qDebug("   VG: closeGame().end.        N8 = %d (DEL = %d).   N64 = %d (DEL = %d)",
           storage::getN8(), storage::getN8del(), storage::getN64(), storage::getN64del());
}


void           GamePlay::changeSpeed( bool isIncrease)
{
    quint32 speedUpNew = speedUp;
    if( isIncrease) speedUpNew *= 2;
    else speedUpNew /= 2;
    speedUpNew = qBound( (quint32)1, speedUpNew, (quint32)10);

    if( speedUpNew != speedUp){
        speedUp = speedUpNew;
        qDebug("GW: speedUp = %d ", speedUpNew);
        foreach (aiGLink *pLink, lLinks){
            pLink->setBuildingLinkDT();
        }
    }
}

#define  BOUNDARY_ADJUST_TOP         10
#define  BOUNDARY_ADJUST_DOWN    30
void                GamePlay::redrawStars( qreal zoomFactor)
{
    qreal                       w = gw->width() - BOUNDARY_ADJUST_TOP - BOUNDARY_ADJUST_DOWN;
    qreal                       h = gw->height() - BOUNDARY_ADJUST_TOP - BOUNDARY_ADJUST_DOWN;
    qreal                       radius = (w >= h)?
                qMin( w / 4, h / (2.0 + sqrt(3.0))) :
                qMin( h / 4, w / (2.0 + sqrt(3.0)));

    gw->screenSize = radius;
    QVector<aiGNode*>::const_iterator itNodes = pStars.begin();
    while( itNodes != pStars.end()){
        aiGNode *pNode = *itNodes;
        if( pNode){
            qreal                       radBase = SCREEN_SCALE_BASE;
            qreal                       wBase = radBase * 4.0;
            qreal                       hBase = radBase * (2.0 + sqrt(3.0));

            quint32 xBase = 0, yBase = 0;
            Pos2XY( idPositions.get( pNode->index()), &xBase, &yBase);
            qreal x = qreal(xBase) / 10.0;
            qreal y = qreal(yBase) / 10.0;
            if( w< h){
                y = hBase - y;
                std::swap( x, y);
                std::swap( wBase, hBase);
                y = hBase - y;
            }
            x *= radius / radBase; y *= radius / radBase;
            wBase *= radius / radBase; hBase *= radius / radBase;
            x += BOUNDARY_ADJUST_TOP + (w - wBase) / 2;
            y += BOUNDARY_ADJUST_TOP + (h - hBase) / 2;
            qDebug( "Star %d: setPos( %d, %d)", pNode->index(), (int)x, (int)y);
            pNode->setPos( QPointF( x * zoomFactor, y * zoomFactor));
            pNode->update();
        }
        ++itNodes;
    }
}

void           GamePlay::accelerate()
{
    quint32 speedUpNew = speedUp;
    int          liveStars = 0;
    for( quint32 i=0; i < nPlayers; ++i) liveStars += teamsStats[i].nActiveStars;
    liveStars = liveStars * 100 / nStars / nPlayers;

    if( liveStars < 6) speedUpNew = 10;
    else if( liveStars < 8) speedUpNew = 8;
    else if( liveStars < 10) speedUpNew = 6;
    else if( liveStars < 25) speedUpNew = 5;
    else if( liveStars < 50) speedUpNew = 4;
    else if( liveStars < 75) speedUpNew = 3;

    if( speedUpNew != speedUp){
        speedUp = speedUpNew;
        qDebug( "Accelerate: speedUp = %d ", speedUpNew);
        foreach( aiGLink *pLink, lLinks){
            pLink->setBuildingLinkDT();
        }
    }
}



void          GamePlay::initAIPlayers()
{
    aiActions.clear();
    if( nPlayers==3){
        pPlayers.fill( 0, nPlayers - 1);
        pPlayers[0] = new aiPlayer();
        pPlayers[0]->init( 1, 1, this);
        gw->connect( pPlayers[0], SIGNAL( aiLink_add(quint32)),
                gw, SLOT( aiLink_add(quint32)));

        pPlayers[1] = new aiPlayer();
        pPlayers[1]->init( 2, 5, this);
        gw->connect( pPlayers[1], SIGNAL( aiLink_add(quint32)),
                gw, SLOT( aiLink_add(quint32)));
        qDebug( "VG:initAIPlayers. Main thread ID = %i", (quint32)gw->thread()->currentThreadId());
    }
}
void          GamePlay::waitAIPlayers()
{
    if( nPlayers > 1){
        for( int i = nPlayers - 2; i >= 0; --i)
            pPlayers[i]->wait();
    }
}
void          GamePlay::finitAIPlayers()
{
    if( nPlayers > 1){
        for( int i = nPlayers - 2; i >= 0; --i){
            pPlayers[i]->finit();
            delete( pPlayers[i]);
        }
        pPlayers.clear();
    }
    aiActions.clear();
}



void GamePlay::addNode( aiGNode *pNewNode)
{
    if( pNewNode){
        qDebug( "      GG:addNode. ID = %d. Name = %s (id = %d). Team = %d. Index = %d",
                pNewNode->getCurrID(), pNewNode->getName().toLocal8Bit().constData(), pNewNode->getIdName(),
                pNewNode->team(), pNewNode->index());
        pStars[ pNewNode->index()] = pNewNode;
    }
}

void GamePlay::delNode( aiGNode *pNode)
{
    if( pNode){
        if( pStars[ pNode->index()]){
            pStars[ pNode->index()] = 0;
            // remove links
            delLinks( pNode);
        }
    }
}

void GamePlay::delLinks( aiGNode *pNode)
{
    foreach( aiGLink *pLink, pNode->getLinks()){
            gw->remLink( pLink);
    }
}

void GamePlay::addLink( aiGLink *pNewLink)
{
    lLinks << pNewLink;
    pNewLink->setLength( getLength(  pNewLink->pNode1->index(), pNewLink->pNode2->index()));
//    qDebug("addLink: length = %d", pNewLink->length);
}
void GamePlay::delLink( aiGLink *pLink)
{
    if( pLink) {
        pLink->disconnect(); //delete from local lists
        lLinks.removeOne( pLink); //delete from ViewGraph's list
    }
}


QList<quint32>  GamePlay::lActiveFriendConnections( quint32 ind)
{
    QList<quint32> res;
    aiGNode *pNode = pStars[ind];

    if( pNode && pNode->isCommonStar()){
        quint32 team = pNode->team();
        foreach( aiGLink *pLink, pNode->getLinks()){
            if( pLink && pLink->isActive()){
                aiGNode *pNode1 = pLink->linkedNode( pNode);
                if( pNode->isCommonStar() && team == pNode1->team())
                    res << (pNode1->index() % nStars);
            }
        }
    }
    return res;
}


bool          GamePlay::isConnected( quint32 iFrom, quint32 iTo)
{
    bool res = false;
    aiGNode *pNfrom = pStars[iFrom];
    aiGNode *pNto = pStars[iTo];

    if( pNfrom && pNfrom->isCommonStar() && pNto && pNto->isCommonStar() && pNfrom != pNto){
        aiGLink *pLink = pNfrom->findConnection( pNto);
        if( pLink)
            res = pLink->isBuildingLink( pNfrom, pNto) || pLink->isDrainingLink();
    }
    return res;
}


QList<quint32>       GamePlay::getLinkedStarsInd( quint32 ind)
{
    QList<quint32> list;
    aiGNode *pNode = pStars[ind];

    if( pNode && pNode->isCommonStar()) {
        foreach (aiGLink *pLink, pNode->getLinks()) {
            if( pLink && (pLink->isBuildingLink() || pLink->isDrainingLink())) {
                aiGNode *pNode1 = pLink->linkedNode( pNode);
                if( pNode1->isCommonStar())
                    list << pNode1->index();
            }
        }
    }
    return list;
}


void                   GamePlay::updStarProfile()
{
    if( counterU > 5 * 25){
        counterU = 0;

        idStateIT.reset();
        for( int i = pStars.size() - 1; i >= 0; --i){
            starsStats[i].reset();
            if( pStars[i]
                    && idStateIT.get( i)
                    && pStars[i]->getMass() > (qint32)0.4*1000000
                    )
                calcSPi( i);
        }
    }
}

void                   GamePlay::calcSPi( int ind)
{
    aiGNode *pN1 = pStars[ind];
    StarProfile sp = starsStats[ind];
    qint32 m1 = idStateNT.get( ind);

    idStateNT.reset();
    sp.ind = ind;
    for( int i = pStars.size() - 1; i >=0; --i) {
        aiGNode *pN2 = pStars[i];
        qint32 m2 = idStateNT.get( i);
        if( m2){
            if( pN1->team() != i / nStars){ //attacks and defense
                quint32 l = getLength( ind, i);
                if( m1 > m2) { //attack
                    quint32 sz = pN1->getRank();
                    qint32   dt = l / stepBuildingL( sz);
                    if( m1 + stepBuildingM( sz) * dt > m2)
                        sp.uAttack += m2 * exp( - lyambda * dt);
                }else{ //defense
                    quint32 sz = pN2->getRank();
                    qint32   dt = l / stepBuildingL( sz);
                    if( m1 < m2 + stepBuildingM( sz) * dt)
                        sp.uDefense += - m2 * exp( - lyambda * dt);
                }
            }else if( i != ind && m1 > m2){ //support
                quint32 l = getLength( ind, i);
                qint32   dt = l / stepBuildingL( pN2->getRank());
                sp.uSupport += m2 * exp( - lyambda * dt);
            }
        }
    }
    starsStats[ind] = sp;
}

void                   GamePlay::updTeamProfile( quint32 team)
{
    quint32                    nActiveStars = 0, nActiveStars1 = 0;
    quint32                    nIndepStars = 0, nIndepStars1 = 0;
    quint32                    displ = team * nStars;

    idStateT.reset(); idStateIT.reset();
    for( int i = 0; i < (int)nStars; ++i){
        qint32 m = idStateT.get( displ + i);
        idStateIT.set( m, displ + i);
        quint32 sz = getMassRank( m);
        if( sz) nActiveStars++;
        if( sz > 1) nActiveStars1++;
    }
    //from connected stars remain only massive ones
    for( int i = nStars - 1; i >= 0; --i){
        qint32 m = idStateIT.get( displ + i);
        if( m){
            int         iMax = i;
            qint32   mMax = m;
            foreach( quint32 j, lActiveFriendConnections( displ + i)){
                qint32 m1 = idStateIT.get( displ + j);
                if( m1 > mMax){
                    idStateIT.set( 0, displ + iMax);
                    mMax = m1; iMax = j;
                }else
                    idStateIT.set( 0, displ + j);
            }
            quint32 sz = getMassRank( mMax);
            if( sz) nIndepStars++;
            if( sz > 1) nIndepStars1++;
        }
    }
    teamsStats[team].nActiveStars = nActiveStars;
    teamsStats[team].nActiveStars1 = nActiveStars1;
    teamsStats[team].nIndepStars = nIndepStars;
    teamsStats[team].nIndepStars1 = nIndepStars1;
}


void       GamePlay::finishWaitingSteps( bool isWaitEndOthers)
{
    // delete Objs
    if( isWaitingSteps){
        bool isDel = true;
        if( isWaitEndOthers) waitAIPlayers();
        else for( int i = 0; i < pPlayers.size(); ++i) isDel = isDel && pPlayers[i]->isFinished();
        if( isDel){
            while( !toDelL.empty()) gw->remLink( toDelL.dequeue());
            while( !toDelN.empty()){
                aiGNode *pNode = toDelN.dequeue();
                qDebug("VG: Star (%d) is deleted", pNode->index());
                gw->remNode( pNode);
            }
            isWaitingSteps = false;
        }else qDebug( "Can't execute WaitingSteps... pPlayers[0]->isFinished() = %s. pPlayers[1]->isFinished() = %s",
                (pPlayers[0]->isFinished())? "+" : "-", (pPlayers[1]->isFinished())? "+" : "-");
    }
}


void       GamePlay::checkDataIntegrity( bool isCorrect)
{
    Q_UNUSED(isCorrect)
    quint32 lDat = getDim();
    for( quint32 i = 0; i < lDat; ++i){
        aiGNode *node = pStars[i];
        if( node){
            if( node->index() != i)
                qDebug( "Unintegrity in index in Node = %d", i);
            if( !(node->isCommonStar() || node->isDarkHole()) && !toDelN.contains( node))
                qDebug( "Unintegrity in deleting Node = %d. toDel queue", i);
            if( node->isCommonStar() && node->getMass() <= 0)
                qDebug( "Unintegrity in mass Node = %d. Active", i);
            if( toDelN.contains( node) && node->isVisible())
                qDebug( "Unintegrity in Node = %d. Visible", i);
        }
    }
    QList<aiGLink*>::iterator itLinks = lLinks.begin();
    while( itLinks != lLinks.end()) {
        aiGLink *link = *itLinks;
        if( link){
            int ind1 = (link->pNode1 && link->pNode1->isCommonStar())? link->pNode1->index() : -1;
            int ind2 = (link->pNode2 && link->pNode2->isCommonStar())? link->pNode2->index() : -1;
            if( toDelL.contains( link) && link->isVisible())
                qDebug( "Unintegrity in Link (%d, %d). Visible", ind1, ind2);
            if( (ind1 < 0 || ind2 < 0) && link->isVisible())
                qDebug( "Unintegrity in Link (%d, %d). Not full", ind1, ind2);
        }
        ++itLinks;
    }
}


int          GamePlay::step()
{
//    qDebug("VG: Step begins");
    int      winner = -10;

    QTime t;
    t.start();

    timerFreqReal = gw->getTimerFreq();
    finishWaitingSteps( false);

    //add aiPlayers links
    if( !aiActions.empty()){
//    while( !aiActions.empty()) {
        quint32 act = aiActions.dequeue();
        quint32 indN1 = (act / 0x00010000) & 0x0000FFFF;
        quint32 indN2 = act & 0x0000FFFF;
        qDebug( "add AI Link: %i -> %i", indN1, indN2);
        if( indN1 < (quint32)pStars.size() && indN2 < (quint32)pStars.size()){
            aiGNode *pN1 = pStars[indN1];
            aiGNode *pN2 = pStars[indN2];
            gw->addLink( pN1, pN2);
        }
    }

    QVector<aiGNode*>::iterator itStars = pStars.begin();
    // calculate decay
    while( itStars != pStars.end()) {
        aiGNode *pNode = *itStars;
        if( pNode && pNode->isCommonStar()){
            pNode->dmClear();
            pNode->stepDecay( stepDecay( pNode->getRank()));
        }
        ++itStars;
    }

    //calculate mass transfers, links building
    QList<aiGLink*>::iterator itLinks = lLinks.begin();
    while( itLinks != lLinks.end()) {
        aiGLink *pLink = *itLinks;
        if( pLink && pLink->isActive()) pLink->step( this);
        ++itLinks;
    }

    //update stars, eliminate stars & links
    itStars = pStars.begin();
    idStateT.reset();
    while( itStars != pStars.end()) {
        aiGNode *pNode = *itStars;
        qint32 m = 0;
        if( pNode && pNode->isCommonStar()){
            pNode->stepFinit();
            if( pNode->isDarkHole()){
                qDebug( "VG: Star (%d) became dark hole", pNode->index());
                QString s = "";
                switch( pNode->team()){
                case 1: s += "GREEN"; break;
                case 2: s += "BLUE"; break;
                default: s += "Your";
                }
                gw->infoText->addText( s + " star became Dark Hole!", 800);
                isWaitingSteps = true;
                toDelLinks( pNode);
            }else if( !pNode->isCommonStar()){
                isWaitingSteps = true;
                toDelLinks( pNode);
                toDelNode( pNode);
                qDebug( "VG: Star (%d) is prepared to delete", pNode->index());
            }else
                m = pNode->getMass();
        }
        idStateT.setInc( m);
        ++itStars;
    }

    checkDataIntegrity();
    finishWaitingSteps( false);
    checkDataIntegrity();
    if( gw->actButton->isActive()) gw->actButton->updData();

    //update stars statistics
    idStateNT.reset();
    itStars = pStars.begin();
    while( itStars != pStars.end()) {
        aiGNode *pNode = *itStars;
        qint32 mn = 0;
        if( pNode && pNode->isCommonStar()){
            pNode->updStats();
            mn = pNode->getMassL();
        }
        idStateNT.setInc( mn);
        ++itStars;
    }

    // Update status of the game
    // Conditions to stop the game
    // 1. Only one team has star(-s) with sz > 1
    quint32 pWinner1 = nPlayers;

    for( quint32 i = 0; i < nPlayers; ++i) {
        updTeamProfile( i);
        if( teamsStats[i].nActiveStars1) winner = i;
        else pWinner1--;
    }
    if( !pWinner1) winner = -1; //drawn game
    else if( pWinner1 > 1) winner = -10;

    if( winner == -10) {
        // Update counters
        counterU ++;
        accelerate();

        // AI players move
        for( int i = 0; i < pPlayers.size(); ++i)
            pPlayers[i]->preStep();

        if( t.elapsed() > 1) qDebug( "Game: step ends. dt = %d ms", t.elapsed());

        for( int i = 0; i < pPlayers.size(); ++i)
            pPlayers[i]->step();
    }

    return winner;
//    qDebug("VG: Step ends");
}







