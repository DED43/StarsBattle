#ifndef MENU_H
#define MENU_H

#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QListWidget>
#include <QGraphicsItem>

#include "ds.h"

class aiGNode;
class GamePlay;


#define     GAMES_VIEW_IND               1
#define     GAME_SAVE_SLOTS             5    // must be < 8


class MainMenu : public QDialog
{
    Q_OBJECT
public:
    MainMenu( QWidget * parent = 0, quint32 profile = 0xFFFFFF00);
    ~MainMenu() {}

    QPushButton       *pContinue;
    QPushButton       *pNewGame;
    QPushButton       *pSaveGame;
    QPushButton       *pLoadGame;
    QPushButton       *pSettings;
    QPushButton       *pQuitGame;

public slots:
    void contGame(){ done(1);}
    void newGame(){ done(2);}
    void saveGame(){ done(3);}
    void loadGame(){ done(4);}
    void editSettings(){ done(5);}
    void quitGame(){ done(6);}
};


class SaveLoadMenu : public QDialog
{
    Q_OBJECT
public:
    SaveLoadMenu( QWidget * parent, QVector<aiid> idGames, bool _isLoad = true);
    ~SaveLoadMenu() {}

    QListWidget                        *games;
    QString                                   name;

public slots:
    void                                         select( QListWidgetItem *item);

private:
    bool                                         isLoad;
    QVector<QString>               gameNames;
};


class EditName : public QDialog
{
    Q_OBJECT
public:
    EditName( QWidget * parent, QString _name);
    ~EditName() {}

    QString                                   name;

public slots:
    void                                         enableSaveButton( const QString str);
    void                                         toSave(){ name = edit->text(); done(1);}

private:
    QLineEdit                             *edit;
    QPushButton                       *save;
    QPushButton                       *cancel;
};



class GLineObjName : public QGraphicsTextItem
{
public:
    GLineObjName (QGraphicsItem *parent = 0);

    aiGNode* node;

    void           setNode( aiGNode* node);

    enum { Type = UserType + 21 };
    int type() const { return Type; }

protected:
    void keyPressEvent( QKeyEvent*);
};

#define  INFO_TEXT_RAWS       5
class GInfoText : public QGraphicsTextItem
{
public:
    GInfoText( QGraphicsItem *parent = 0);

    enum { Type = UserType + 21 };
    int type() const { return Type; }

    void clear();
    void addText( QString s, quint32 c = 0xFFFFFFFF);
    void drawText();

    void updateCounters();
    void deleteRaw( quint32 n);

    QVector<QString>    text;
    QVector<int>             counters;
    int                                 h;

};


class GMenuButton : public QGraphicsEllipseItem
{
public:
    GMenuButton( QString);

    enum { Type = UserType + 22 };
    int type() const { return Type;}

    int r;

protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString txt;
};


class GActionLink : public QGraphicsItem
{
public:
    GActionLink();

    aiGNode     *nodeFrom, *nodeTo;

    inline void   reset(){ nodeFrom = nodeTo = 0; l.clear(); setVisible( false);}
    inline void   reset( aiGNode *node){ if( nodeFrom == node || nodeTo == node) reset();}
    inline bool   isActive(){ return( nodeFrom && nodeTo && isVisible());}
    void               adjust();


    enum{ Type = UserType + 23};
    int type() const{ return Type;}

protected:
    QRectF         boundingRect() const;
    void               paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QPolygonF   l;

};

class GActionButton : public QGraphicsEllipseItem
{
public:
    GActionButton();

    inline aiGNode* nodeFrom(){ return link->nodeFrom;}
    inline aiGNode* nodeTo(){ return link->nodeTo;}
    void setText( QString txt);
    inline QString getText(){return oText->toPlainText();}
    void set( aiGNode *nodeFrom, aiGNode *nodeTo);
    inline bool isActive(){ return( link->isActive() && isVisible());}
    void updData();
    inline void reset(){ link->reset(); setVisible( false);}
    inline void reset( aiGNode *n){ link->reset( n); if( !link->isActive()) reset();}

    enum { Type = UserType + 24 };
    int type() const { return Type;}

    GActionLink   *link;
    int                       r;

protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString                          symbol;
    QGraphicsTextItem *oText;
};





#endif // MENU_H
