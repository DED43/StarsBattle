#ifndef AI_H
#define AI_H

#include <QThread>
#include <QQueue>
#include <QMap>

#include "ds.h"
#include "graph.h"


/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for aiPlayer
   *
 * -----------------------------------------------------------------------------------------------
 */
class aiPlayer: public QThread, public PlayerBase
{
    Q_OBJECT

public:
    aiPlayer();

    GamePlay                              *pGame;


    void init( int _iTeam, int _iLevel, GamePlay *_pGame);
    void preStep();
    inline void step(){ if( ready){ ready = false; working = true; start();}}

    void finit();

    volatile bool   working;
    bool                  ready;
    quint32           timer;

signals:
    void aiLink_add( quint32 dat);

protected:
    void run();

private:
    void                                     actorStep( Actor *pActor);

    int             counter_1;
};



#endif // AI_H
