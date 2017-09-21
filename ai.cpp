
#include "ai.h"


int   timerLevel[0x0010] = {0,
                             10 * 25,
                               6 * 25,
                               4 * 25,
                               2 * 25,
                               2 * 25,
                                       0,0,0,0,0,0,0,0,0,0};

int   nActions[0x0010] = {0,
                                      1,
                                      1,
                                      2,
                                      2,
                                      3,
                                       0,0,0,0,0,0,0,0,0,0};


aiPlayer::aiPlayer()
{
    working = false; pGame = 0;
}

void          aiPlayer::init( int _iTeam, int _iLevel, GamePlay *_pGame)
{
    if( _pGame) {
        iTeam = _iTeam; iLevel = _iLevel; pGame = _pGame;
        lDat = pGame->getDim();
        qDebug("AI: player (%i) init. Begin", iTeam);

        initDat( (GameCommon*)_pGame);

        timer = timerLevel[ iLevel];
        counter_1 = 0;
        ready = false;
        qDebug("AI: player (%i) init. End", iTeam);
    }
}

void          aiPlayer::finit()
{
    working = false;
    quit(); wait();
    pGame = 0;
    finitDat();
}

void          aiPlayer::preStep()
{
    if( !(--timer)){
        if( isRunning() || pGame->isWaitingSteps){
            timer = 1;
            qDebug( "aiPlayer's step can't start... isRunning() = %s. pGame->isWaitingSteps = %s",
                    (isRunning())? "+" : "-", (pGame->isWaitingSteps)? "+" : "-");
        }else{
            teamsStats = pGame->teamsStats;
            if( teamsStats[iTeam].nActiveStars1 || teamsStats[iTeam].nActiveStars > 1) {
                timer = timerLevel[ iLevel];
                idStateT.copy( &pGame->idStateT);
                idStateNT.copy( &pGame->idStateNT);
                idStateIT.copy( &pGame->idStateIT);
                pGame->updStarProfile();
                choose_actors();
                ready = true;
            }
        }
    }
}

void          aiPlayer::run()
{
    qDebug("AI: player (%i) step. ID = %i. N = %i. N1 = %i.", iTeam, (quint32)currentThreadId(),
           teamsStats[iTeam].nActiveStars, teamsStats[iTeam].nActiveStars1);
    aiActions.clear();

    QTime t;
    t.start();

    QMap<qreal, Actor>::iterator itActors = lActors.begin();
    while( working && itActors != lActors.end()){
        Actor a = *itActors;
        actorStep( &a);

        if( aiActions.size() >= nActions[iLevel]) itActors = lActors.end();
        else ++itActors;
    }

    if( working){
        int n = nActions[iLevel];
        while( !aiActions.isEmpty() && n > 0){
            emit aiLink_add( aiActions.dequeue());
            --n;
        }
        aiActions.clear();
    }

    if( t.elapsed() > 1) qDebug("AI: player (%i) step ends. dt = %d ms", iTeam, t.elapsed());
}



void          aiPlayer::actorStep( Actor *pActor)
{
    qDebug("AI: Actor #%i. IND = %i. iY = %i. m = %1.1f. mn = %1.1f. mnMax = %1.1f. mnCrytY = %1.1f. dmnDrain = %i. mnCrytDrain = %1.1f",
           pActor->rank, pActor->ind, pActor->iY, (pActor->m/1000000.0), (pActor->mn/1000000.0),
           (pActor->mnMax/1000000.0), (pActor->mnCrytY/1000000.0), pActor->dmnDrain, (pActor->mnCrytDrain/1000000.0));
    QString s = "Steps: 101";

    QQueue<int>                    steps;
    quint32                               a1 = 0, s1 = 0, s2 = 0;
    qint32 indA[6] =       {-1, -1, -1, -1, -1, -1};
    qint32 indS[6] =        {-1, -1, -1, -1, -1, -1};
    quint32 actDH[3] =  { 0,  0,  0};

    int                                       iStep = 100;
    while( working && iStep){
        switch( iStep){
        case 100: //   Defense Variants
            if( pActor->dmnDrain < 0) { //we are under attack!
                if( pActor->dmnDrain + 2 * pGame->stepDraining(5) < 0){ //severe attack. next actor's move
                    qDebug("we are under severe attack!");
                    escapeSupporting( pActor);
                }
                else if( pActor->dmnDrain + pGame->stepDraining(3) > 0 //under weak attack. Attack
                         && pActor->mnMax - pActor->mn < 3*1000000){
                    choicesA2( pActor, indA, 0x00001000 | 0x00000100);
                    a1 = a_1( pActor->ind, indA, 0);
                    if( !a1){
                        sN1_choices( pActor->ind, indS);
                        s1 = s_1( pActor->ind, indS, 3);
                        if( !s1) s1 = s_1( pActor->ind, indS, 1002);
                        if( s1 && nActions[iLevel] > 1){
                            choicesA2( pActor, indA, 0x00001000 | 0x00000200);
                            a1 = a_1( pActor->ind, indA, 0);
                        }
                    }
                }
                else
                    steps.enqueue(102);
            }
            else{
                if( pActor->mn > 1*1000000)
                    steps.enqueue(150);
            }
            break;
        case 101: //   Defense. Attack + Support
            steps.enqueue(161); steps.enqueue(162);                    // +A1
            steps.enqueue(111);
            steps.enqueue(114);                                                        // if !A1 then +S1
            steps.enqueue(105);
            break;
        case 102: //   Defense. Support only
            steps.enqueue(111);
            steps.enqueue(112);                                                        // +S1
            steps.enqueue(105);
            break;
        case 105: //   Defense. init & additional steps for Level>2
            if( iLevel > 2){
                steps.enqueue(113);                                                    // +S2
                if( iLevel == 5) {
                }
            }
            break;
        case 111: //   #1. Defense.   Init
            sN1_choices( pActor->ind, indS);
            break;
        case 112: //   #1. Defense.   Support main
            s1 = s_1( pActor->ind, indS, 3);
            if( !s1) s1 = s_1( pActor->ind, indS, 1002);
            break;
        case 113: //   #1. Defense.   Support extra
            s2 = s_1( pActor->ind, indS);
            break;
        case 114: //   #1. Attack.   Support if no Attack
            if( !a1) steps.enqueue(112);
            break;

        case 150: //   #1. Attack Variants
            if( (pActor->mn + pActor->mnDrain2 > 17*1000000 || pActor->mn + pActor->mnDrain5 > 19*1000000)
                    && protectFromDH2( pActor, actDH)) {
                qDebug("     Protecting from becoming Dark Hole");
                a1 = actDH[0];
                if( iLevel > 2 && actDH[1] > 0) a1 = actDH[1];
                if( iLevel == 5) s1 = actDH[2];
            }
            else if( pActor->mn > 6*1000000){
                if( pActor->dmnDrainNegative < 0){
                    qDebug("     Drain positive, but we are still under attack...");
                    choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000100);
                    a1 = a_1( pActor->ind, indA, 0);
                    if( !a1){
                        if( pActor->rank == 1) s1_choices( pActor->ind, indS);
                        else sN1_choices( pActor->ind, indS);
                        s1 = s_1( pActor->ind, indS, 3);
                        if( !s1) s1 = s_1( pActor->ind, indS, 1002);
                        if( s1 && nActions[iLevel] > 1){
                            choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000200);
                            a1 = a_1( pActor->ind, indA, 0);
                        }
                    }
                }else{
                    if( pActor->mn < 12*1000000){
                        qDebug("     We are small and will try to get bigger...");
                        choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000100);
                        a1 = a_1( pActor->ind, indA, 0);
                        if( !a1){
                            choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000200);
                            a1 = a_1( pActor->ind, indA, 0);
                            if( !a1){
                                sN1_choices( pActor->ind, indS);
                                s1 = s_1( pActor->ind, indS, 3);
                                if( !s1) s1 = s_1( pActor->ind, indS, 1002);
                                if( !s1 && !pActor->dmnDrain){
                                    escapeSupporting( pActor);
                                }
                            }
                        }
                    }else{
                        qDebug("     We are big enough. Let's play...");
                        if( pActor->dmnDrain > 2 * pGame->stepDraining(5)
                                && pActor->mn + pActor->mnDrain2 > 17*1000000){
                            qDebug("        DANGEROUS. Do nothing");
                        }else if( pActor->dmnDrain > abs( pGame->stepBuildingM(5) + pGame->stepDecay(5))){
                            choicesA2( pActor, indA, 0x00000001);
                            a1 = a_1( pActor->ind, indA, 0);
                            qDebug("        Sufficient drain rate. Attack the biggest possible");
                        }else{
                            qDebug("        Attack...");
                            choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000100);
                            a1 = a_1( pActor->ind, indA, 0);
                            if( !a1){
                                choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000200);
                                a1 = a_1( pActor->ind, indA, 0);
                            }
                        }
                    }

                }
            }else{
                choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000100);
                a1 = a_1( pActor->ind, indA, 0);
                if( !a1){
                    choicesA2( pActor, indA, 0x00020000 | 0x00001000 | 0x00000200);
                    a1 = a_1( pActor->ind, indA, 0);
                    if( !a1 && !pActor->dmnDrain){
                        escapeSupporting( pActor);
                    }
                }
            }
            break;
        case 151: //   Attack. Attack + Support
            steps.enqueue(161); steps.enqueue(163);                    // +A1
            steps.enqueue(111);
            steps.enqueue(114);                                                        // if !A1 then +S1
            steps.enqueue(105);
            break;
        case 161: //   #1. Attack. Steps
            choicesA1( pActor, indA, pActor->mnCrytDrain);
            break;
        case 162: //   #1. Attack. Steps
            a1 = a_1( pActor->ind, indA, 3);
            if( !a1) a1 = a_1( pActor->ind, indA, 1002);
            break;
        case 163: //   #1. Attack. Steps
            a1 = a_1( pActor->ind, indA, 1);
            break;

        default:
            steps.clear();
        }

        if( steps.isEmpty()) iStep = 0;
        else{ iStep = steps.dequeue(); s+= " -> " + QString().setNum( iStep);}
    }

//    qDebug("%s", s.toStdString().c_str());
    if( a1) {
        aiActions.enqueue( a1);
//        qDebug("add AI attack Link: %i -> %i", a1 / 0x00010000, a1 & 0x0000FFFF);
    }
    if( s1) {
        aiActions.enqueue( s1);
//        qDebug("add AI support Link: %i -> %i", s1 / 0x00010000, s1 & 0x0000FFFF);
    }
    if( s2) {
        aiActions.enqueue( s2);
//        qDebug("add AI extra support Link: %i -> %i", s2 / 0x00010000, s2 & 0x0000FFFF);
    }
}



