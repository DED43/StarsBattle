#ifndef DS_H
#define DS_H

#include "DS_global.h"
#include <QMap>
#include <QVector>
#include <QQueue>

#include "ds_t.h"


//! DEFINITIONS
typedef quint32 bit;               // >= quint32
typedef quint32 aiid;             // id
typedef quint32 aiidl;           // id for list

//! FILES
#define FILE_DAT64  "aiD64.dt"
#define FILE_DAT8     "aiD8.dt"


//! Storage "S64". 64MB
#define M_DAT64                  64      // block's lentgh (in elements)
#define N_DAT64_OBJ       128      // 2^8, storage capacity.       =32kB

//! Storage "S8". 64MB
#define M_DAT8                      8       // block's lentgh (in elements)
#define N_DAT8_OBJ        4096     // 2^12, storage capacity.     =128kB

//! AIID masks
#define FAIID_AIID_MASK          0x03FFFFFF         // AIID
#define FAIID_IND_MASK           0x00FFFFFF         // AIID -> IND
#define FAIID_ST_MASK              0x03000000        //
#define FAIID_FL_MASK              0xFC000000        //

//! FLAGS. Storage
#define FAIID_DAT64      0x01000000     // S64 (+) or S8 (-)
#define FAIID_DAT__        0x02000000    // reserve for storage

#define SHORT_VERSION

#define N_STOR_L1           2048  //
#define N_STOR_LL1         2048  //

typedef QList<aiid> lAIID;

/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for STORAGE
   *
 * -----------------------------------------------------------------------------------------------
 */
class DSSHARED_EXPORT storage {

public:
    storage();
    ~storage();

    static void init();
    static void finit();

    inline static quint32 getN8(){ return aiD8.getN();}
    inline static quint32 getN8del(){ return aiD8.getNdel();}
    inline static quint32 getN64(){ return aiD64.getN();}
    inline static quint32 getN64del(){ return aiD64.getNdel();}


    static aiid add( bit *pSrc, quint32 lData); //source, length (in bits)
    static aiid add( quint32 lData);
    static bool remove( aiid id);

    static bool write( bit *pSrc, aiid id, quint32 displ, quint32 lData); //write data with possibility to enlarge record. displ=0,1,2,...
    static bool writeL( bit *pSrc, aiid id, quint32 displ, quint32 lData); //write data with truncating after added data
    static quint32 read( bit *pDst, aiid id, quint32 displ, quint32 lData=0);
    static quint32 findData( aiid id, bit d, bit mask=-1);

    static quint32 indAIID( aiid id);
    static bool isValid( aiid id);
    static bool isEmpty( aiid id);
    static quint32 length( aiid id);
    static void setLength( aiid id, quint32 l);

    //! HUGE BLOCKS functionality
    static aiid  setLong( quint32 lData); //length (in bits)
    static void remLong( aiid id);
    static void clearLong( aiid id);
    static void copyLong( aiid idFrom, aiid idTo);

    //! functionality
    static bit L1[N_STOR_L1];
    static bool     read2L1( aiid id);
    static bool    write4L1_short( aiid id);
    static bool  readV2L1( quint32 ind);
    static bool writeV4L1( quint32 ind);


    //! TEXT functionality
    static QString text( aiid);
    static aiid add_text( QString, aiid _id=0);


    //! OBJ functionality
    //    linking high level structure to record
    static void     setObj( aiid id, pAIID p=0);
    static void     resetObj( aiid id);
    static pAIID  getObj( aiid id);


    //! LIST functionality.
    //    list = array of aiid-s
    //    index=1,2,3,...
    static quint32          setIDl( aiidl id, bit d, quint32 ind=0); //write <d> to <aiid> list at <ind> position
    static bit                    getIDl( aiidl id, quint32 ind); //read from <aiid> list data at <ind> position
    static void                 remIDl( aiidl id, quint32 ind); //remove from <aiid> list data at <ind> position. Replace with the last item
    static void                 remRollIDl( aiidl id, quint32 ind); //remove from <aiid> list data at <ind> position. Rolling items left
    static quint32          findIDl( aiidl id, bit d);
    static void                 populate_list( aiidl id, lAIID *pList);
    static aiidl                 save_list( aiidl id, lAIID *pList);
    static void                 del_list( aiidl idl);


    class DSSHARED_EXPORT counter {
    public:
        counter(){ idl = 0;}
        counter( aiidl _idl);
        ~counter(){ }
        void        start(){ block = idl; ind = 1; displ = 0;}
        bool        next_read( bit *p);
        bool        next_write( bit *p);
        quint32 getInc();
        void        setInc( bit _data);
        bool        isEnd();
        quint32 getInd();

    private:
        aiidl         idl;
        aiid          block;
        quint32  displ;
        quint32  ind;
    };

    class DSSHARED_EXPORT ptr {
        friend class LongDat;
    public:
        ptr(){ idBlock = 0; local_ptr = roll_ptr = 0;}
        ptr( aiid id){ init( id);}
        ~ptr(){ }

        void        init( aiid id);
        void        setDispl( quint32 displ);
        void        set( quint32 dat, quint32 displ);
        quint32 get( quint32 displ);

        void        reset( quint32 displ=0);
        void        setInc( quint32 dat);
        quint32 getInc();

    private:
        aiid              idBlock;
        quint32    *local_ptr;
        quint32    *roll_ptr;
    };

    class DSSHARED_EXPORT ptr16 {
    public:
        ptr16(){ idBlock = 0; local_ptr = roll_ptr = 0;}
        ptr16( aiid id){ init( id);}
        ~ptr16(){ }
        void           init( aiid id);
        void           set( quint16 dat, quint16 displ);
        quint16    get( quint16 displ);

        void           reset( quint16 displ=0);
        void           setInc( quint16 dat);
        quint16    getInc();

    private:
        aiid              idBlock;
        quint16    *local_ptr;
        quint16    *roll_ptr;
    };

    class DSSHARED_EXPORT SymMatrix {
    public:
        SymMatrix(){ idLengthMatrix = 0;}
        SymMatrix( quint16  _lDat){ init( _lDat);}

        void                 init( quint16  _lDat);
        void                 finit();

        void                 setDat( quint16 i, quint16 j, quint16 dat);
        quint32          getDat( quint16 i, quint16 j);

    private:
        quint16                     lDat;
        aiid                             idLengthMatrix;
        storage::ptr16         pLM;
    };

    //! VIEW-S functionality.
    //    index=1,2,3,...
    static bool           isView( quint32 ind);
    static quint32    createView();
    static quint32    newView( aiid _id);
    static quint32    setView( aiid _id);
    static aiid             getView( quint32 ind);



private:
    static quint32 count;
    static storage_t<N_DAT64_OBJ, M_DAT64, bit, aiid> aiD64;
    static storage_t<N_DAT8_OBJ, M_DAT8, bit, aiid> aiD8;

    static bit LL1[N_STOR_LL1];

    //! Array of lists
    static lAIID lHeadsID;
    // [0] - ...     ###  Reserve  ###
    // [1] - Views



    // reserve new (empty) record
    static aiid newLong();
    static aiid newShort();


    static bool isBlock( aiid id);
    static quint32 lengthBlock( aiid id);
    static aiid nextBlock( aiid id);
    static aiid bindNextBlock( aiid id, aiid next);
    static aiid unbindNextBlock( aiid id);

    static quint32 bytes2bits( quint32 l);
    static quint32 quint32bits( quint32 l);

    static bool editRecLength( aiid id, quint32 lData);
    static bool gotoPos( aiid *pId, quint32 *pDispl);
    static bit *getBlockPtr( aiid id, quint32 displ);
    static aiid rollLeft( aiid id, quint32 *pLen, quint32 displ=0); //Rolling items left
    static void delTail( aiid block);
    static aiid addLong( aiid tail);
    static aiid addShort( aiid tail);

    static bool writeT( QString, aiid);
};


class DSSHARED_EXPORT LongDat : public storage::ptr {

public:
    LongDat(){ l = 0;}
    ~LongDat(){ }

    void   init( quint32 _l);
    void   finit();
    void   copy( LongDat *pFrom);
    bool   isEnd();

private:
    quint32   l;
};






/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for DatBase
   *
 * -----------------------------------------------------------------------------------------------
 */
#define  DAT_BASE_L   8
class DSSHARED_EXPORT DatBase {

public:
    DatBase();
    ~DatBase();

    aiid     aiSave();
    bool    aiUpdate();
    bool    aiLoad( aiid _id);

    void   aiDel_fast();
    void   aiDel_id( quint32 i);
    void   aiDel_lid( quint32 i);
    aiid     getID();

    static void   aiDel_fast( aiid _id);
    static void   aiDel_id( aiid _id, quint32 i);
    static void   aiDel_lid( aiid _id, quint32 i);

    void   aiSetData( quint32 i, bit *pData);
    void   aiResetData( quint32 i);

    void   aiSetObj( pAIID ptr);
    void   aiResetObj();

protected:
    aiid    id_curr;

private:
    aiid    id_ptr;
};

class DSSHARED_EXPORT NodeBase : public DatBase {

public:
    NodeBase();
    ~NodeBase(){ }

    static void aiDel_fast( aiid id);

    void       save_name( QString str);
    QString load_name();

protected:
    aiid          idName;
    quint32   mType;
    qint32     mParam;
};
class DSSHARED_EXPORT LinkBase : public DatBase {

public:
    LinkBase();
    ~LinkBase(){ }

    static void aiDel_fast( aiid id);

protected:
    aiid           idObj1, idObj2;
    quint32    mType;
};
class DSSHARED_EXPORT GameBase : public DatBase {

public:
    GameBase();
    ~GameBase(){ }

    static void aiDel_fast( aiid id);

    void       save_name( QString str);
    QString load_name();

protected:
    aiid            idName;
    aiidl           idNodes;
    aiidl           idLinks;
    aiidl           idCmds;
    quint32    zBase;
};





/**
 * -----------------------------------------------------------------------------------------------
   *
    This is the class for GAME
   *
 * -----------------------------------------------------------------------------------------------
 */

struct DSSHARED_EXPORT StarProfile {
    StarProfile(){ reset();}
    quint32 ind;
    qreal   uAttack;
    qreal   uDefense;
    qreal   uSupport;

    void reset(){ ind = 0; uAttack = 0; uDefense = 0; uSupport = 0;}
    inline qreal u( qreal a1, qreal a2){ return( uAttack + a1 * uDefense + a2 * uSupport);}
};
struct DSSHARED_EXPORT TeamProfile {
    TeamProfile(){ reset();}
    quint32                    nActiveStars, nActiveStars1;
    quint32                    nIndepStars, nIndepStars1;

    void reset(){ nActiveStars = 0; nActiveStars1 = 0; nIndepStars = 0; nIndepStars1 = 0;}
};
class DSSHARED_EXPORT GameCommon {

public:
    GameCommon(){ timerFreqReal = 25;}
    virtual ~GameCommon(){}

    quint32                           nPlayers;
    quint32                           nStars;

    qreal                                lyambda;
    quint32                           speedUp;
    qreal                                timerFreqReal;

    QVector<StarProfile>              starsStats;

    quint32                     getDim();

    static quint32         getMassRank( qint32 mass);
    static quint32         getMaxMassRank();
    static qint32            getLowBound( quint32 massRank);

    qint32                       stepDecay( quint32 rank);
    qint32                       stepBuildingM( quint32 rank);
    quint32                     stepBuildingL( quint32 rank);
    qint32                       stepDraining( quint32 rank);
    qint32                       stepDraining( qint32 mFrom, qint32 mTo);
    static qint32           massDrained( qint32 mFrom, qint32 mTo, quint32 time = 2); // time in minutes (speedUp=1)
    quint32                    getMassDrainedTime( quint32 time = 2);
    void                           initLengths( quint32 dim);
    void                           finitLengths();
    void                           setLength( quint32 iFrom, quint32 iTo, quint32 length);
    quint32                    getLength( quint32 iFrom, quint32 iTo);
    qint32                       timeBuildLink( quint32 iFrom, quint32 iTo, qint32 mFrom);
    qint32                       massBuildLink( quint32 iFrom, quint32 iTo, qint32 mFrom);


    virtual   bool                                               isConnected( quint32 iFrom, quint32 iTo) = 0;
    virtual   QList<quint32>                         getLinkedStarsInd( quint32 ind) = 0;


private:
    storage::SymMatrix      mxLength;

};



#define   AI_MAX_ACTORS   3

struct DSSHARED_EXPORT Actor {
    Actor(){ reset();}

    quint32     ind;
    quint32     rank;
    qint32       m;
    quint32     iY;
    qint32       mnMax;
    qint32       mnCrytY;
    qint32       mn;
    qint32       mnCrytBL;
    qint32       dmnDrain, dmnDrainNegative, mnDrain2, mnDrain5;
    qint32       mnCrytDrain;

    QMap<qint32, qint32>   mnDrainTriggers;

    void           reset();
    qint32       dmnDrainChange( qint32 mBegin, qint32 mEnd);
};

class DSSHARED_EXPORT PlayerBase {

public:
    PlayerBase() {}

    int                                                iTeam;
    int                                                iLevel;

    QVector<TeamProfile>           teamsStats;

    void initDat( GameCommon  *gc);
    void finitDat();


protected:
    GameCommon  *gc;
    quint32    nPlayers, nStars, lDat;
    LongDat   idStateT, idStateNT, idStateIT;

    QMap<qreal, Actor>        lActors;

    qreal         alphaU1, alphaU2;
    void           choose_actors();

    void          choicesA1( Actor *pActor, qint32 *pInd, qint32 mFinalMin = 0);
    void          choicesA2( Actor *pActor, qint32 *pInd, quint32 flags = 0);
    //                                                                                            0x0000000x max mass flow (default)
    //                                                                                            0x00000001 max mass
    //                                                                                            0x00000x00 timeHoryzonType. auto select (default)
    //                                                                                            0x00000100 timeHoryzonType = 2
    //                                                                                            0x00000200 timeHoryzonType = 5
    //                                                                                            0x00001000 positive net balance of mass flow
    //                                                                                            0x00010000 mnFromEnd > pActor->mnCrytY
    //                                                                                            0x00020000 mnFromEnd > pActor->mnCrytDrain
    quint32   a_1( quint32 ind, qint32 *pInd, quint32 type = 0);
    void          s1_choices( quint32 ind, qint32 *pInd);
    void          sN1_choices( quint32 ind, qint32 *pInd);
    quint32   s_1( quint32 ind, qint32 *pInd, quint32 type = 0);
    bool          protectFromDH( Actor *pActor, quint32 *pInd);
    bool          protectFromDH2( Actor *pActor, quint32 *pInd);
    void          escapeSupporting( Actor *pActor);

    QQueue<quint32>                   aiActions;
};




#endif // DS_H
