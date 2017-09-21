#ifndef GRAPH_H
#define GRAPH_H


#include <QObject>
#include <QGraphicsItem>
#include <QString>
#include <QList>

//#include <QSemaphore>
#include <QMutex>
#include <QQueue>
#include <QHash>
#include <QTextStream>
#include <QDateTime>
#include <QThread>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

#include "ds.h"
#include "graphicswidget.h"

#define VIEW_Z_BASE                    200
#define VIEW_Z_NAMES                230
#define VIEW_Z_LINK                       20

class aiGLink;
class GamePlay;
struct Actor;
class aiPlayer;


class aiGNode : public QGraphicsEllipseItem, public NodeBase
{

public:
    aiGNode( GameWidget*, QString);
    aiGNode( GameWidget*, aiid);
    ~aiGNode();

    aiGLink*          findConnection( aiGNode*); //Any existing connection
    bool                  isLinkPossibleTo( aiGNode*); //Is it possible to initiate link building
    inline void      bindLink( aiGLink *pLink){ if( !lLinks.contains( pLink)) lLinks << pLink;}
    inline bool      unbindLink( aiGLink *pLink){ return( lLinks.removeOne( pLink));}


    inline void        dmClear(){ dmDecay = 0; dmBL = 0; dmDrain = 0;}
    inline void        stepDecay( qint32 dm){ dmDecay += dm;}
    inline void        stepBL( qint32 dm){ dmBL += dm;}
    inline void        stepDrain( qint32 dm){ dmDrain += dm;}
    void                    stepFinit();
    void                    updStats();


    void                    setStar( quint32 nTeam, quint32 ind, qint32 mass) {
        mParam = mass;
        mType = nTeam | 0x00000010 | (ind * 0x00010000);
        updStarType();
        updStarView();
    }
    void                    updStarType();
    void                    updStarView();
    inline bool        isCommonStar(){ return( mType & 0x00000010);}
    inline bool        isDarkHole(){        return( mType & 0x00000020);}
    inline bool        isActive(){ return( isCommonStar() || isDarkHole());}
    inline qint32    getMass(){ return mParam;}
    inline qint32    getMassL(){ return qMax( mParam - mBL, 0);}
    inline quint32 getRank(){ return prof();}
    inline int           getRadius(){ return r;}
    inline quint32 getRadiusScaled(){ return( 1000.0 * r / gw->getScaleFactor() / gw->getScreenFactor()/ 1.1);}

    inline quint32  index(){ return( (mType & 0x00FF0000)/0x00010000);}
    inline quint32  team(){ return( mType & 0x0000000F);}
    inline quint32  nLinks(){ return( lLinks.size());}


    inline aiid                              getCurrID(){ return id_curr;}
    inline GameWidget*          getGW(){ return gw;}
    inline GamePlay*                pView(){ return( gw->getView());}
    inline QList<aiGLink*>     getLinks(){ return lLinks;}

    void                     setName( QString name);
    inline QString   getName(){ return( (oName)? oName->toPlainText() : "");}
    inline aiid          getIdName(){ return idName;}

    aiid  aiSave();
    void aiLoad( aiid _id);

    inline qint16 getZ(){ return( zValue());}

    enum{ Type = UserType + 11};
    int type() const{ return Type;}


protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange( GraphicsItemChange change, const QVariant &value);

    void mousePressEvent( QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent( QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event);

//!    aiid          idName;
//!    quint32   mType;
                                        //type (relative to Base)
                                        //   0x0000000F  #team
                                        //   0x00FF0000  #index
                                        //   0x00000010   Star
                                        //   0x00000F00    size
                                        //   0x00000020   Dark Hole
                                        //   0x00000040   toDel
    inline quint32 prof(){ return ((mType & 0x00000F00)/0x00000100);}

//!    qint32     mParam;
                                        //parameter for type (32bit)
                                        //   Mass, x1,000,000


private:
    GameWidget                 *gw;
    QGraphicsTextItem     *oName;
    QList<aiGLink*>            lLinks;

    void adjustLinks();

    qint32                                dmDecay, dmBL, dmDrain;
    qint32                                mBL;

    int                                       r;
    QColor                               clrShadow;
    QColor                               clrObj;

    inline void reset(){
        gw = 0;
        setName("");
        idName = mType = mParam = 0;
        dmDecay = dmBL = dmDrain = 0;
        mBL = 0;
        lLinks.clear();
    }
    void init();
};


class aiGLink : public QGraphicsItem, public LinkBase
{
public:
    aiGLink();
    aiGLink( aiGNode*, aiGNode*);
    aiGLink( aiid);
    ~aiGLink();

    aiGNode *pNode1, *pNode2;

    inline void connect( aiGNode *pN1, aiGNode *pN2){
        if( pN1 && pN2){
            pNode1 = pN1; pNode1->bindLink( this);
            pNode2 = pN2; pNode2->bindLink( this);
            adjust();
        }
    }
    inline void disconnect(){
        if( pNode1){ pNode1->unbindLink( this); pNode1 = 0;}
        if( pNode2){ pNode2->unbindLink( this); pNode2 = 0;}
    }

    inline bool isLinking( aiGNode *pN1, aiGNode *pN2){
        return( (pNode1==pN1 && pNode2==pN2) || (pNode1==pN2 && pNode2==pN1));}
    inline bool isLinking( aiGNode *pN){ return( (pNode1==pN) || (pNode2==pN));}
    inline aiGNode *linkedNode( aiGNode *pN){
        return( (pNode1==pN)? pNode2 : (pNode2==pN)? pNode1 : 0);}

    aiid             aiSave();
    bool            aiLoad( aiid _id);

    void adjust();

    void                    step( GamePlay *pGame);
    bool                    setBuildingLink( aiGNode *pNfrom);
    inline bool        isBuildingLink( aiGNode *pNfrom, aiGNode *pNto){
        return( (pNode1==pNfrom && pNode2==pNto && prof1()) || (pNode1==pNto && pNode2==pNfrom && prof2()));}
    inline bool        isBuildingLink(){ return( mType & 0x00000FF0);}
    inline bool        isDrainingLink(){ return( mType & 0x00001000);}
    inline bool        isActive(){              return( mType & 0x00001FF0);}
    inline bool        isToDel(){               return( mType & 0x00002000);}
    inline void        setLength( quint32 l){ length = l;}
    void                    setBuildingLinkDT();
    inline quint32 getBuildingLinkDT(){ return tBL;}
    inline void        toDel(){ mType = 0x00002000; setVisible( false);}

    inline quint32 prof1(){ return ((mType & 0x000000F0)/0x00000010);}
    inline quint32 prof2(){ return ((mType & 0x00000F00)/0x00000100);}

    enum{ Type = UserType + 12};
    int type() const{ return Type;}



protected:
    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

//!    aiid           idObj1, idObj2;
//!    quint32    mType;
                                             // type
                                             //   0x000000X0  building drain link S1->S2, profile
                                             //   0x00000X00  building drain link S2->S1, profile
                                             //   0x00001000  drain link
                                             //   0x00002000  link toDel


private:
    quint32    length;             // length between stars, x1000
    quint32    length12;        // drain link built S1->S2, x1000
    quint32    length21;        // drain link built S2->S1, x1000
    quint32    tBL;                  // time (ticks) to build link

    QPolygonF l;

    inline void reset(){
        pNode1 = pNode2 = 0; idObj1 = idObj2 = 0;
        mType = 0;
        l.clear();
        length = length12 = length21 = 0;
        tBL = 0;
    }
    void init();
};



class GamePlay : public GameCommon, public GameBase
{
    friend class GameWidget;

public:
    GamePlay( GameWidget*);
    ~GamePlay();

    inline GameWidget*  getGW(){ return gw;}

//    quint32   nPlayers;
//    quint32   nStars;
//    qreal         lyambda;
//    GameCommon       currGame;

    int                                                  counterU;
    void                                               updStarProfile();
//    QVector<StarProfile>              starsStats;
    QVector<TeamProfile>            teamsStats;


    LongDat                                      idPositions, idStateT, idStateNT, idStateIT;
    QQueue<quint32>                   aiActions;
    QQueue<aiGNode*>                toDelN;
    QQueue<aiGLink*>                 toDelL;
    bool                                              isWaitingSteps;
    void                                              finishWaitingSteps( bool isWaitEndOthers = true);
    void                                              checkDataIntegrity( bool isCorrect = true);
    void                                              waitAIPlayers();

    QVector<aiid>                           pGames;
    inline bool isGame( quint32 ind){ return( ind <= GAME_SAVE_SLOTS && pGames[ind]);}

    void         newGame( quint32 nPlayers, quint32 nStars); //nPlayers, nStars
    void              generateStars();
    void              initAI();
    void         saveGame( quint32 ind = 0);
    bool         loadGame( quint32 ind = 0);
    void              loadNodes();
    void              loadLinks();
    void         closeGame();

    void         changeSpeed( bool isIncrease);
    void         accelerate();

    void                     setLengths();

    int                      step();
//    bool                                           isActiveConnection( quint32 ind1, quint32 ind2);
    QList<quint32>                         lActiveFriendConnections( quint32 ind);
    bool                                               isConnected( quint32 iFrom, quint32 iTo);
    QList<quint32>                         getLinkedStarsInd( quint32 ind);

    void aiDel_game( quint32 ind);

    void addNode( aiGNode*);
    void delNode( aiGNode*);
    void delLinks( aiGNode*);
    void addLink( aiGLink*);
    void delLink( aiGLink*);

//    inline quint32 getDim(){ return lDat;}
//    inline quint32 getDim(){ return GameCommon::getDim();}
    inline quint32 XY2Pos( quint32 x, quint32 y){ return( 0x00010000 * x + (0x0000FFFF & y));}
    inline void        Pos2XY( quint32 pos, quint32 *pX, quint32 *pY){ *pY = 0x0000FFFF & pos; *pX = 0x0000FFFF & (pos / 0x00010000);}

    void                  redrawStars( qreal zoomFactor);
//    aiGNode*        findNearest( QPoint pos, int rad = 100);

    inline void       setName( QString _name){ mName = _name;}
    inline QString getName(){ return( mName);}

    inline void resetViewGraph(){
        mName   = "";
        lLinks.clear();
        idName   = 0; idNodes  = 0; idLinks    = 0; idCmds   = 0;

        zBase       = VIEW_Z_BASE;  // 0xFF
        nPlayers = 3; nStars = 10;       // 0xFF, 0xFF
        lyambda = 1.0 / (25.0 * 30.0);
        speedUp = 2; counterU = 1000000;
//        lDat = nPlayers * nStars;

        pStars.clear(); pPlayers.clear(); starsStats.clear(); teamsStats.clear();
        toDelL.clear(); toDelN.clear(); isWaitingSteps = false;
    }
    inline void initViewGraph1(){
        quint32 lDat = getDim();
        idPositions.init( lDat); idStateT.init( lDat); idStateNT.init( lDat); idStateIT.init( lDat);
        pStars.fill( 0, lDat); starsStats.fill( StarProfile(), lDat); teamsStats.fill( TeamProfile(), nPlayers);
        toDelL.clear(); toDelN.clear(); isWaitingSteps = false;
    }
    inline void finitViewGraph1(){ idStateIT.finit(); idStateNT.finit(); idStateT.finit(); idPositions.finit();}


private:
    GameWidget                            *gw;
    QString                                        mName;

//    quint32                                       lDat;

    QVector<aiPlayer*>                pPlayers;   //AI players
    QVector<aiGNode*>               pStars;
    QList<aiGLink*>                      lLinks;

    void initAIPlayers();
    void finitAIPlayers();

    inline void toDelNode( aiGNode *n){ gw->remNode_Footprints( n); toDelN.enqueue( n);}
    inline void toDelLinks( aiGNode *n){ foreach( aiGLink *l, n->getLinks()) toDelLink( l);}
    inline void toDelLink( aiGLink *l){ l->toDel(); toDelL.enqueue( l);}

    void                                              calcSPi( int ind);
    void                                              updTeamProfile( quint32 team);
};





























/**
 * -----------------------------------------------------------------------------------------------
   *
   This is the base class for Vertex/Link
   *
 * -----------------------------------------------------------------------------------------------
*/
/*
class aiVertex {
public:
//    aiVertex ();
    aiVertex (QString name, aiVertex* parent=0);
    aiVertex (aiid);
    ~aiVertex();

    void addLink (aiid, void*);
    bool removeLink (aiid);
    void removeLinks();

    aiid getAIID() {return id;}

    aiVertex* aiGetParent() {return pVParent;}
    aiid aiGetParentID() {return idVParent;}
    void aiSetParent (aiVertex *pParentNode=0);

    void aiSetName (QString _name) {mName=_name; save_name();}
    QString aiGetName() {return mName;}

    void aiSetX(qint16 _x) {aiX = (aiGetParent())? _x - aiGetParent()->aiGetX() : _x;}
    qint16 aiGetX() {return ((aiGetParent())? aiX + aiGetParent()->aiGetX() : aiX);}
    void aiSetY(qint16 _y) {aiY = (aiGetParent())? _y - aiGetParent()->aiGetY() : _y;}
    qint16 aiGetY() {return ((aiGetParent())? aiY + aiGetParent()->aiGetY() : aiY);}
    void aiSetZ(qint16 _z) {aiZ = (aiGetParent())? _z - aiGetParent()->aiGetZ() : _z;}
    qint16 aiGetZ() {return ((aiGetParent())? aiZ + aiGetParent()->aiGetZ() : aiZ);}

    void aiSave();
    void aiLoad();

protected:
    aiVertex *pVParent;
    aiid id, idVParent, idName;
    quint32 mType; //type
    quint32 mFlags; //flags
    qint16 aiX, aiY; //relative to parent vertex (8+8bit)
    qint16 aiZ; //relative to parent vertex (8bit)
    quint32 mParam; //parameter for type (32bit)
    QString mName;
    lAIID mLinksAIID;
    hAIID hLinks;

    void save_name() {idName = storage::add (mName, idName);}
    void load_name() {if (idName) mName = storage::text (idName);}

    void save_base (quint32*, quint32 displ=0);
    void load_base (quint32*, quint32 displ=0);
    void populate_links ();

};

class aiLink {
public:
    aiLink ();
    //aiLink (aiVertex* _parent);
    //~aiLink();

    void addVertex (aiid, void*);
    bool removeVertex (aiid);
    void removeVertices();

    aiid getAIID() {return id;}

    void aiSetName (QString _name) {mName=_name; save_name();}
    QString aiGetName() {return mName;}

    void aiSave();
    void aiLoad();

protected:
    aiid id, idName;
    quint32 aiType; //type
    quint32 aiFlags; //flags
    qint16 mZ; //relative to parent vertex (8bit)
    quint16 mParam; //parameter for type (16bit)
    QString mName;
    lAIID mVerticesID;
    hAIID hVertices;

    void save_name() {idName = storage::add (mName, idName);}
    void load_name() {if (idName) mName = storage::text (idName);}

    void save_base (quint32*, quint32 displ=0);
    void load_base (quint32*, quint32 displ=0);
    void populate_vertices ();
};
*/


/**
 * -----------------------------------------------------------------------------------------------
   *
   This is the class for NodeE/LinkE
   *
 * -----------------------------------------------------------------------------------------------
*/
/*
class aiNodeE : public QGraphicsEllipseItem, public aiVertex
{

public:
    aiNodeE (GraphicsWidget *graphicsWidget, QString name, aiNodeE* parent=0);
    //~aiVertexE();

    void bindLink (aiLinkE* pl=0);
    bool unbindLink (aiLinkE*);

    void aiSetName (QString _name);
    void setXYscene (qint16,qint16);
    void setZ (qint16);

    enum { Type = UserType + 11 };
    int type() const { return Type; }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mousePressEvent (QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent (QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent (QGraphicsSceneHoverEvent *event);


private:
    GraphicsWidget *gw;
    QGraphicsTextItem *oName;

    void adjust_childs();

};

class aiLinkE : public QGraphicsItem, public aiLink
{
public:
    aiLinkE();
    //aiLinkE(aiNodeE *parent);

    void adjust();

    enum { Type = UserType + 12 };
    int type() const { return Type; }

protected:
    QRectF boundingRect() const;
    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QPolygonF l;

};
*/


/**
 * -----------------------------------------------------------------------------------------------
   *
   This is the main class for a GraphE
   *
 * -----------------------------------------------------------------------------------------------
*/
/*
typedef QList<aiVertex*> Vertices;
//typedef map<long int,long int> imap_i;
//typedef map<int,float> imap_f;
//typedef QHash <QString, int> hash_si;

class aiGraph:  public QObject{
    Q_OBJECT

public:
    aiGraph();            //Creates a new graph.
    //void clear();          //Clears m_heads
    ~aiGraph();			//destroy object

    //bool loadGraph ();
    //bool saveGraph ();

private:
    Vertices mHeads;
};
*/

//class Graph:  public QObject {
//	Q_OBJECT

//public slots:
//	/** Slots to signals from Parser */
//	void createVertex (int i, int size, QString nodeColor,
//						QString numColor, int numSize,
//						QString label, QString lColor, int lSize,
//						QPointF p, QString nodeShape, bool signalMW
//						); //Main vertex creation call
						
//	void setFileType (int, QString, int,int, bool);
//	void removeDummyNode (int);
	
//	/** Slots to signals from GraphicsWidget and Parser*/
//    void createLink (int, int, float, QString, int, bool, bool);	//GW and Parser.
//    void createLink (int, int, float, int, bool, bool);             //GW
//    void createLink (int, int);                                     //WebCrawler
//    void nodeMovement (bool state, int type, int cW, int cH);        //Called by MW to start movement

//    void slotSetLinkVisibility( int, int, bool);
	
//	//auxiliary createVertex functions
//    void createVertex (int i, QPointF p);                            //Called by GW
//	void createVertex (int i, int canvasWidth, int canvasHeight); 	//Called by MW
//    void createVertex (QString label, int i) ;                       //Called by WebCrawler
	
//	/** Slots to signals from MainWindow */
//	void setCanvasDimensions (int w, int h);
//    void filterOrphanVertices (bool);                             //Called by MW to filter orphan vertices
	
//signals:
//	/** Signals to MainWindow */
//	void updateProgressDialog(int );
//    void graphChanged();                                //call to update MW widgets
//    void selectedVertex(int);                           //notifies MW who is the selected node

//	void signalFileType (int, QString, int,int, bool);	//notifies MW what we have loaded.
//    void statusMessage (QString message);               //updates statusbar message
		
//	/** Signals to GraphicsWidget */
//	void drawNode( int ,int,  QString, QString, int, QString, QString, int, QPointF, QString, bool, bool, bool);	//call GW to draw a node
	
//    void eraseNode (long int);                          //erase node from GW
//    void drawLink(int, int, float, bool, bool, QString, bool);	//call GW to draw an edge
//    void eraseLink(int, int);                           //emited from removeEdge() to GW to clear the edge item.
//    void setLinkVisibility ( int, int, bool);			// emitted from each Vertex
//    void setVertexVisibility(long int, bool);           //notifies GW to disable a node
//	void moveNode(int, int, int);

	
//public:
//    /** INIT AND CLEAR*/
//    Graph();            //Creates a new graph.
//    void clear();       //Clears m_graph
//	~Graph();			//destroy object

//    void setVersion (QString ver) { VERSION = ver; }
		
//    void setShowNames(bool);

//	/*FILES (READ AND WRITE)*/
//    bool loadGraph ( QString, bool,	int, int, int, int);	//Our almost universal network loader. :)
	
//	bool saveGraph( QString fileName, int fileType,
//						QString networkName, int maxWidth, int maxHeight
//				);

//    //** VERTICES */
//	int lastVertexNumber();				//Returns the number of the last vertex
//	int firstVertexNumber();			//Returns the number of the first vertex

//    int hasVertex(long int);		    //Checks if a vertex exists
//	int hasVertex(QString);				//Checks if a vertex with a label exists
//    void removeVertex (long int);	    //removes given vertex from m_graph

//    void setInitVertexSize (long int); 	//Changes the init size used in new vertices.
//    void setVertexSize(long int, int);	//Changes the size.of vertex v

	
//    void setInitVertexShape (QString); 	//Changes the init shape used in new vertices.
//    void setVertexShape( int, QString); //Changes the shape.of vertex v
//    QString shape(int);                 //returns the shape of this vertex

//    void setInitVertexColor (QString);  	//Changes the init color used in new vertices
//    void setVertexColor(long int, QString); //Changes the color of vertex v


//    void setInitVertexNumberColor (QString);    //Changes the init number color in new vertices
//    void setInitVertexNumberSize (int);         //Changes the init number size in new vertices
	
//    void setInitVertexLabelSize(int);       //Changes the init size of new vertices labels
//    void setVertexLabelSize( int, int);     //Changes the size of a vertex label
	
//    void setInitVertexLabelColor(QString);	//Changes the init color used by all new vertices' labels
//    void setVertexLabel( int, QString); 	//Changes the label.of vertex v
//    void setVertexLabelColor( int, QString);
//	QString label(int);


//	void updateVertCoords(int v, int x, int y);	 //Updates vertex v with coords x,y

//    int vertices() ;                    //Returns the sum of vertices inside m_graph

//	int edgesFrom (int i) ;				//Returns the number of edges starting from v1 (outDegree)
//	int edgesTo (int i) ;				//Returns the number of edges ending to v1 (inDegree)

//	int verticesWithOutEdges();			//Returns the sum of vertices having outEdges
//	int verticesWithInEdges();			//Returns the sum of vertices having inEdges
//	int verticesWithReciprocalEdges();		//Returns the sum of vertices having reciprocal edges


//    /** EDGES */
//    float hasLink (int v1, int v2);			//Checks if edge between v1 and v2 exists. Returns weight or -1
//    void removeLink (int v1, int v2);		//removes the edge between v1 and v2
	
 
//    void setInitLinkColor(QString);

//    void setLinkColor( long int, long int, QString);	//Changes the color of edge (s,t).
//    QString edgeColor (long int, long int); 		//Returns the edgeColor
	 
//    int totalLinks();				//Returns the sum of edges inside m_graph

//	float density();				//Returns ratio of present edges to total possible edges.

//protected:

//    void timerEvent (QTimerEvent *event);			// Called from nodeMovement when a timerEvent occurs


//private:

//	/** List of pointers to the vertices. A vertex stores all the info: links, colours, etc */
//	Vertices m_graph;

//	/** private member functions */

//	void addVertex (
//						int v1, int val, int size, QString color,
//						QString numColor, int numSize,
//						QString label, QString labelColor, int labelSize,
//						QPointF p, QString shape
//					);			// adds a vertex to m_graph
						
//    void addLink ( int, int, float, QString); 		//adds an link between v1 and v2

//	/** used in resolveClasses and createDistanceMatrix() */
//    hash_si discreteIDCs, discreteODCs, discreteCCs, discreteBCs, discreteSCs, discreteGCs, discreteECs, discretePCs, discreteICs,  discretePRCs;
	
//	int *eccentricities;
//	bool calculatedIDC, calculatedODC, calculatedCentralities, dynamicMovement;
	
//	QList<int>  triadTypeFreqs; 	//stores triad type frequencies
//	stack<int> Stack;


//    /** General & initialisation variables */

//	long int m_totalEdges, m_totalVertices, graphDiameter, initVertexSize;
//	int initVertexLabelSize, initVertexNumberSize;

//	int isolatedVertices;
//	float averGraphDistance, nonZeroDistancesCounter;
//	int outEdgesVert, inEdgesVert, reciprocalEdgesVert;
//	int timerId,  layoutType, canvasWidth, canvasHeight;
	
//	bool order, initShowLabels, initNumbersInsideNodes;
//	bool adjacencyMatrixCreated, symmetricAdjacencyMatrix, graphModified, distanceMatrixCreated;
//	bool m_undirected;

//	QString VERSION, networkName, initEdgeColor, initVertexColor, initVertexNumberColor, initVertexLabelColor, initVertexShape;
	
//	QDateTime actualDateTime;

//};

#endif

