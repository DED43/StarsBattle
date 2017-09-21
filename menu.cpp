#include "menu.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "graph.h"



/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
MainMenu::MainMenu( QWidget *parent, quint32 profile) : QDialog( parent, Qt::FramelessWindowHint)
{
    setFont( QFont( "Times", 16, QFont::Light, false));
    setWindowOpacity( 0.7);

    pContinue = pNewGame = pSaveGame = pLoadGame = pQuitGame = 0;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(5);

    if( profile & 0xF0000000){
        pContinue = new QPushButton( tr("Continue..."));
        mainLayout->addWidget( pContinue);
        connect( pContinue, SIGNAL(clicked()), this, SLOT(contGame()));
        pContinue->setDefault( true);
    }
    if( profile & 0x0F000000){
        pNewGame = new QPushButton( tr("New Game"));
        mainLayout->addWidget( pNewGame);
        connect( pNewGame, SIGNAL(clicked()), this, SLOT(newGame()));
        if( !(profile & 0xF0000000)) pNewGame->setDefault( true);
    }
    if( profile & 0x00F00000){
        pSaveGame = new QPushButton( tr("Save Game"));
        mainLayout->addWidget( pSaveGame);
        connect( pSaveGame, SIGNAL(clicked()), this, SLOT(saveGame()));
    }
    if( profile & 0x000F0000){
        pLoadGame = new QPushButton( tr("Load Game"));
        mainLayout->addWidget( pLoadGame);
        connect( pLoadGame, SIGNAL(clicked()), this, SLOT(loadGame()));
    }
    if( profile & 0x0000F000){
        pSettings = new QPushButton( tr("Settings"));
        mainLayout->addWidget( pSettings);
        connect( pSettings, SIGNAL(clicked()), this, SLOT(editSettings()));
    }

    pQuitGame =  new QPushButton( tr("Quit Game"));
    mainLayout->addWidget( pQuitGame);
    connect( pQuitGame, SIGNAL(clicked()), this, SLOT(quitGame()));

    setLayout( mainLayout);
}


/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
#define EMPTY_SLOT_TEXT " ..."
SaveLoadMenu::SaveLoadMenu(QWidget *parent, QVector<aiid> idGames, bool _isLoad) :
    QDialog( parent, Qt::FramelessWindowHint)
{
    setFont( QFont( "Times", 16, QFont::Light));
    setWindowOpacity( 0.7);

    isLoad = _isLoad;

    games = new QListWidget( this);
    games->setViewMode( QListView::ListMode);
    games->setMovement( QListView::Static);
//    games->setMaximumWidth(128);
//    games->setSpacing(1);

    int firstAvailableIndex = 0;
    gameNames.fill( "", GAME_SAVE_SLOTS);
    for( int i = 1; i <= GAME_SAVE_SLOTS; ++i){
        QListWidgetItem *button = new QListWidgetItem;
        button->setTextAlignment( Qt::AlignLeft);
        button->setFont( QFont( "Times", 14, QFont::Light, false));
        if( idGames[i]){
            aiid id_name = 0;
            storage::read( &id_name, idGames[i], 0, 1);
            gameNames[i-1] = storage::text( id_name);
        }
        button->setText( (gameNames[i-1].isEmpty())? EMPTY_SLOT_TEXT : gameNames[i-1]);
        if( (!isLoad) || idGames[i]){
//            button->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            if( !firstAvailableIndex) firstAvailableIndex = i;
        }
        games->insertItem( i - 1, button);
    }
    if( firstAvailableIndex) games->setCurrentRow( firstAvailableIndex - 1);
    connect( games, SIGNAL( itemClicked( QListWidgetItem *)), this, SLOT( select( QListWidgetItem *)));
    connect( games, SIGNAL( itemDoubleClicked( QListWidgetItem *)), this, SLOT( select( QListWidgetItem *)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( games);
    mainLayout->setMargin( 1);
    setLayout( mainLayout);
}


void           SaveLoadMenu::select( QListWidgetItem *item)
{
    quint32 ind = qMin( games->row( item) + 1, GAME_SAVE_SLOTS);
    if( isLoad){
        if( !gameNames[ind-1].isEmpty())
            done( ind);
    }
    else{
        EditName en( this, (gameNames[ind-1] == EMPTY_SLOT_TEXT)? "" : gameNames[ind-1]);
        switch( en.exec()){
        case 1:
            name = en.name;
            done( ind);
            break;
        default:;
        }
    }
}


EditName::EditName( QWidget *parent, QString _name) :
    QDialog( parent, Qt::FramelessWindowHint)
{
    setFont( QFont ("Times", 14, QFont::Light, false));

    setWindowOpacity( 1.0);
    name = _name;

    save = new QPushButton( tr("SAVE"));
    save->setFont( QFont ("Times", 12, QFont::Light, false));
    cancel = new QPushButton( tr("CANCEL"));
    cancel->setFont( QFont ("Times", 12, QFont::Light, false));
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setMargin(5);
    buttonsLayout->addWidget( cancel);
    buttonsLayout->addWidget( save);

    edit = new QLineEdit;
    edit->setText( name);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(5);
    mainLayout->addWidget( edit);
    mainLayout->addLayout( buttonsLayout);

    if( name == ""){
        cancel->setDefault( true);
        save->setEnabled( false);
    }else save->setDefault( true);
    setLayout( mainLayout);

    connect( edit, SIGNAL( textChanged( const QString &)), this, SLOT( enableSaveButton( const QString &)));
    connect( save, SIGNAL( clicked()), this, SLOT( toSave()));
    connect( cancel, SIGNAL( clicked()), this, SLOT( close()));
}

void                     EditName::enableSaveButton( const QString str)
{
    save->setEnabled( !str.isEmpty());
}



/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
GMenuButton::GMenuButton( QString s)
{
    txt = s;
    setZValue( VIEW_Z_NAMES);
    setAcceptHoverEvents( false);

    QFont f( "Times", 12, QFont::DemiBold);
    QFontMetrics fm( f);
    int textWidthInPixels = fm.width( txt);
    int textHeightInPixels = fm.height();

    r = qMax( textWidthInPixels, textHeightInPixels) / 2 + 3;
}

QRectF GMenuButton::boundingRect() const
{
    qreal adjust = 0;
    return QRectF( - adjust - 2 * r, - adjust, 2 * (r + adjust), 2 * (r + adjust));
}

QPainterPath GMenuButton::shape() const
{
    QPainterPath path;
        path.addEllipse( -2 * r, 0, 2 * r, 2 * r);
    return path;
}

void GMenuButton::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)
//    painter->setPen( QColor( 0xFF, 0xCF, 0xCF, 0xFF));
    painter->setPen( QPen( QColor( 0xFF, 0xCF, 0xCF, 0xBF), 2));
    painter->setBrush( Qt::darkBlue);
    painter->drawEllipse(  -2 * r, 0, 2 * r, 2 * r);
    painter->setPen( Qt::white);
    painter->setFont( QFont( "Times", 12, QFont::DemiBold));
    painter->drawText( boundingRect(), Qt::AlignCenter, txt);
}


/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
GActionLink::GActionLink()
{
    setZValue( VIEW_Z_NAMES + 3);
    setAcceptHoverEvents( false);
    reset();
}

void   GActionLink::adjust()
{
    l.clear();
    prepareGeometryChange();
    if( nodeFrom && nodeTo){
        l << mapFromItem( nodeFrom, 0, 0);
        l << mapFromItem( nodeTo, 0, 0);
    }
}


QRectF GActionLink::boundingRect() const
{
    if( nodeFrom && nodeTo){
        qreal extra = 1;
        return l.boundingRect()
            .normalized()
            .adjusted( -extra, -extra, extra, extra);
    }
    return( QRectF());
}

void GActionLink::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor clr = Qt::darkGreen;
    clr.setAlpha( 250);
    painter->setPen( QPen( clr, 2, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPolygon( l);
    if( nodeFrom && nodeTo){
        painter->setBrush( clr);
        QPointF p = mapFromItem( nodeFrom, 0, 0);
        int r = 2;
        painter->drawEllipse( p.x() - r, p.y() - r, 2 * r, 2 * r);
        p = mapFromItem( nodeTo, 0, 0);
        painter->drawEllipse( p.x() - r, p.y() - r, 2 * r, 2 * r);
    }
}



GActionButton::GActionButton()
{
    symbol = "L+";
    setZValue( VIEW_Z_NAMES);
    setAcceptHoverEvents( false);

    QFont f( "Times", 11, QFont::DemiBold);
    QFontMetrics fm( f);
    int textWidthInPixels = fm.width( symbol);
    int textHeightInPixels = fm.height();
    r = qMax( textWidthInPixels, textHeightInPixels) / 2 + 3;

    oText = new QGraphicsTextItem( "", this);
    if( oText){
        oText->setFont( QFont( "Times", 9, QFont::Light));
        oText->setDefaultTextColor( Qt::white);
        oText->setPos( -2 * r - 4, r - QFontMetrics( oText->font()).height() / 2 - 1);
        oText->setZValue( VIEW_Z_NAMES);
        oText->setAcceptHoverEvents( false);
    }
}

void GActionButton::set( aiGNode *nodeFrom, aiGNode *nodeTo)
{
    link->nodeFrom = nodeFrom; link->nodeTo = nodeTo;
    link->adjust(); link->setVisible( true); link->update();
    updData();
    setVisible( true);
}


void GActionButton::updData()
{
    GamePlay *game = link->nodeFrom->pView();
    qreal m = (qreal) -game->massBuildLink(
                link->nodeFrom->index(),
                link->nodeTo->index(),
                link->nodeFrom->getMass()) / 1000000.0;
    quint32 t = game->timeBuildLink(
                link->nodeFrom->index(),
                link->nodeTo->index(),
                link->nodeFrom->getMass()) / link->nodeFrom->getGW()->getTimerFreq();
    QString txt = "Mass: " +
            QString().setNum( m, 'f', ( m<9.95)? 2 : 1) +
            "; Time: " + QString().setNum( t) + " s.";
    setText( txt);
}

void GActionButton::setText( QString txt)
{
    if( oText && oText->toPlainText() != txt){
        oText->setPos(
                    -2 * r - QFontMetrics( oText->font()).width( txt) - 10,
                    r - QFontMetrics( oText->font()).height() / 2 - 2);
        oText->setPlainText( txt);
    }
}


QRectF GActionButton::boundingRect() const
{
    qreal adjust = 0;
    return QRectF( - adjust - 2 * r, - adjust, 2 * (r + adjust), 2 * (r + adjust));
}

QPainterPath GActionButton::shape() const
{
    QPainterPath path;
        path.addEllipse( -2 * r, 0, 2 * r, 2 * r);
    return path;
}

void GActionButton::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)
    painter->setPen( QPen( QColor( 0xFF, 0xCF, 0xCF, 0xBF), 2));
    painter->setBrush( Qt::darkGreen);
    painter->drawEllipse( -2 * r, 0, 2 * r, 2 * r);
    painter->setPen( Qt::white);
    painter->setFont( QFont( "Times", 11, QFont::DemiBold));
    painter->drawText( boundingRect(), Qt::AlignCenter, symbol);
}




/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
GLineObjName::GLineObjName( QGraphicsItem *parent) : QGraphicsTextItem( parent)
{
    setFont( QFont( "Times", 10, QFont::DemiBold));
    setDefaultTextColor( Qt::darkRed);
    setZValue( VIEW_Z_NAMES);
    setAcceptHoverEvents( false);
    //setFlag (QGraphicsItem::ItemIsFocusable);
//    setTextInteractionFlags( Qt::TextSelectableByKeyboard | Qt::TextEditable);
    //setFlag(ItemSendsGeometryChanges);
    node = 0;
}

void GLineObjName::keyPressEvent( QKeyEvent *e)
{
    qDebug( "LineText: keyPressEvent()");
    switch (e->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        clearFocus();
        if( node){
            node->setName( toPlainText());
            node->update();
            qDebug( "LineText: Name is changed");
        }
        break;
    case Qt::Key_Escape:
        setPlainText( (node)? node->getName() : "");
        node->update();
        clearFocus();
        break;
    default:
        QGraphicsTextItem::keyPressEvent(e);
    }
}


void GLineObjName::setNode( aiGNode* node)
{
    if( !node){
        clearFocus();
        setPlainText("");
        this->node = NULL;
    }else if( node != this->node){
        clearFocus();
        this->node = node;
//        objName->setPlainText( _objNameNode->getName());
        QString s = "#"
                + QString().setNum( node->index())
                + ".   "
                + QString().setNum( node->nLinks())
                + " links.";
        setPlainText( s);
    }
}




/**
    |||||||||||||||||||||||||||||||||||||||
    |||||||||||||||||||||||||||||||||||||||
*/
GInfoText::GInfoText (QGraphicsItem *parent) : QGraphicsTextItem(parent)
{
    setFont( QFont( "Times", 8, QFont::DemiBold, false));
    setDefaultTextColor( QColor( 0xFF, 200, 200, 255));
//    setDefaultTextColor( QColor(Qt::darkRed).lighter( 150).lighter( 150).lighter( 150));
    setZValue( 50);
    setAcceptHoverEvents( false);
    h = (QFontMetrics( font()).height() + 1) * INFO_TEXT_RAWS;
    clear();
}

void GInfoText::clear()
{
    text.fill( "", INFO_TEXT_RAWS);
    counters.fill( 0, INFO_TEXT_RAWS);
    drawText();
}

void GInfoText::addText( QString s, quint32 c)
{
    for( int i = 1; i < INFO_TEXT_RAWS; ++i){
        text[i - 1] = text[i];
        counters[i - 1] = counters[i];
    }
    text[INFO_TEXT_RAWS-1] = s;
    counters[INFO_TEXT_RAWS-1] = c;
    drawText();
}

void GInfoText::drawText()
{
    QString s = "";
    for( int i = 0; i < INFO_TEXT_RAWS - 1; ++i)
        s += text[i] + "\n";
    s += text[INFO_TEXT_RAWS-1];
    prepareGeometryChange();
    setPlainText( s);
}

void GInfoText::updateCounters()
{
    bool isRedraw = false;
    for( int i = 0; i < INFO_TEXT_RAWS; ++i)
        if( counters[i])
            if( !(--counters[i])){ deleteRaw(i); isRedraw = true;}
    if( isRedraw) drawText();
}

void GInfoText::deleteRaw( quint32 n)
{
    for( int i = n; i > 0; --i){
        text[i] = text[i - 1];
        counters[i] = counters[i - 1];
    }
    text[0] = ""; counters[0] = 0;
}

