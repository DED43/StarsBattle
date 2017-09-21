#ifndef DST_H
#define DST_H

#include <QFile>
#include <QDataStream>
#include <QString>

// Storage
#define N_INITIAL   1           // first element

// flags
//#define FSTOR_MASK     0xff000000

#define FSTOR_AIID_MASK     0x03ffffff        //  AIID -> ID
#define FSTOR_IND_MASK      0x00ffffff        //  AIID -> IND
#define FSTOR_FL_MASK         0xfc000000     //  AIID -> FLAGS
#define FSTOR_SET                    0x08000000
#define FSTOR_HEAD               0x04000000

// file
#define AI_FILE_END       0x12345678

typedef void* pAIID;


//  TnObj<=2^24 storage capacity
template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid> class storage_t {

public:
    storage_t();

    inline quint32 newHead()
    {
        quint32 ind = newBlock();
        if (ind)
            fSet[ind] |= FSTOR_HEAD;
        return ind;
    }
    quint32             newBlock();
    Taiid                   bindNextBlock( Taiid curr, Taiid next); // return previous next block aiid
    Taiid                   delBlock( Taiid id); // returns next Block

    quint32             readBlock( Tbit *pDst, quint32 ind, Tbit displ=0, quint32 length=TnBits);
    quint32             writeBlock( Tbit *pSrc, quint32 ind, Tbit displ=0, quint32 length=TnBits);
    quint32             clearBlock( quint32 ind, Tbit displ=0, quint32 length=TnBits);

    quint32             findData (Taiid id, Tbit d, Tbit mask=-1);

    quint32             newLongBlock( quint32); // length in blocks
    void                    delLongBlock( Taiid id);
    void                    clearLongBlock( Taiid id);


    inline bool        Block_isInRange( quint32 ind){ return( ind<TnObj && ind>=N_INITIAL);}
    inline bool        Block_isHead( quint32 ind){ return( fSet[ind] & FSTOR_HEAD);}
    inline bool        Block_isSet( quint32 ind){ return( fSet[ind] & FSTOR_SET);}
    inline Taiid       nextBlock( quint32 ind){ return( fSet[ind] & FSTOR_AIID_MASK);}

    inline bool        isFreeSpace(quint32 n){ return( nDel+TnObj-nSet - n > 0);}
    inline quint32 bits2Blocks(Tbit length){ return( (length+TnBits-1)/TnBits);} // transform bits of length l to blocks

    inline bool        isHead(quint32 ind){ return( isValid(ind) && Block_isHead(ind));}
    inline bool        isValid(quint32 ind){ return( Block_isInRange(ind) && Block_isSet(ind));}
    inline quint32 length( quint32 ind){ return( lSet[ind]);} // returns length
    inline void        setLength( quint32 ind, quint32 length){ lSet[ind] = length;} // set length

    inline quint32 getN(){ return nSet;}
    inline quint32 getNdel(){ return nDel;}
    inline Tbit      *getPtr( quint32 ind, Tbit displ=0){ return( store + ind * TnBits + displ);}
    inline void       setByPtr( Tbit *ptr, Tbit d=0){ *ptr = d;}
    void                   rollLeft( quint32 ind, quint32 displ, quint32 len, Tbit last=0);

    bool init( const QString &_name);
    void clear();
    void finit();
    bool save_all();
    bool read_all();


    inline pAIID getObj (quint32 ind){ return( (isHead(ind))? pObj[ind] : 0);}
    inline void    setObj (quint32 ind, pAIID _pObj){ if( isHead(ind)) pObj[ind] = _pObj;}

private:
    Tbit           store [TnObj*TnBits];          // data storage
    Taiid         fSet [TnObj];                           // Flags
    quint32   lSet [TnObj];                           // Length (if Head),

    quint32   qDel [TnObj];

    pAIID       pObj [TnObj];                   //

    quint32   nSet;
    quint32   nDel;

    QFile      *pFile;

};


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
storage_t<TnObj, TnBits, Tbit, Taiid>
::storage_t()
{
}

template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
void storage_t<TnObj, TnBits, Tbit, Taiid>
::clear()
{
    Taiid *pf = fSet;
    for (int i = TnObj; i!=0; --i) *(pf++) = 0;
    quint32 *pl = lSet;
    for (int i = TnObj; i!=0; --i) *(pl++) = 0;
    Tbit *pp = store;
    for (int i = 10*TnBits; i!=0; --i) *(pp++) = 0;

    nSet = N_INITIAL;
    nDel = N_INITIAL;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::newBlock()
{
    quint32 ind = 0;

    if( nDel > N_INITIAL){
        ind = qDel[--nDel];
    }
    else if( nSet < TnObj){
        ind = nSet++;
    }
    if( ind){
        lSet[ind] = 0;
        fSet[ind] = FSTOR_SET;
        pObj[ind] = 0;
    }
    return ind;
}
template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
Taiid storage_t<TnObj, TnBits, Tbit, Taiid>
::delBlock( Taiid id)
{
    Taiid idNext = 0;
    quint32 ind = (quint32) (id & FSTOR_IND_MASK);
    if( isValid( ind)){
        idNext = nextBlock(ind);
        fSet[ind] = 0;
        lSet[ind] = 0;
        qDel[nDel++] = ind;
    }
    return idNext;
}

template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::newLongBlock( quint32 l) // Length (in blocks) fixed
{
    quint32 res = 0;
    if( nSet + l < TnObj)
    {
        res = nSet;
        for( int i = l; i > 0; --i){
            lSet[nSet] = 0;
            pObj[nSet] = 0;
            fSet[nSet] = FSTOR_SET;
            if( i != 1) fSet[nSet] |= nSet + 1;
            ++nSet;
        }
        lSet[res] = TnBits * l;
        fSet[res] |= FSTOR_HEAD;
    }
    return res;
}
template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
void storage_t<TnObj, TnBits, Tbit, Taiid>
::delLongBlock( Taiid id)
{
    quint32 ind = (quint32) (id & FSTOR_IND_MASK);
    if( isHead( ind)) {
        quint32 l = lSet[ind] / TnBits;
        bool isLast = ( ind + l == nSet)? true: false;
        for( int i = l; i > 0; --i){
            lSet[ind] = 0;
            pObj[ind] = 0;
            fSet[ind] = 0;
            if( !isLast) qDel[nDel++] = ind;
            ++ind;
        }
        if( isLast)
            nSet -= l;
    }
}
template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
void storage_t<TnObj, TnBits, Tbit, Taiid>
::clearLongBlock( Taiid id)
{
    quint32 ind = (quint32) (id & FSTOR_IND_MASK);
    if( isHead( ind)){
        quint32 l = lSet[ind];
        Tbit *pDat = store + (Tbit)ind * TnBits;
        while( l--) *(pDat++) = 0;
    }
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
Taiid storage_t<TnObj, TnBits, Tbit, Taiid>
::bindNextBlock( Taiid curr, Taiid next)
{
    quint32 ind = (quint32) (curr & FSTOR_IND_MASK);
    if( Block_isInRange( ind))
    {
        curr = fSet[ind] & FSTOR_AIID_MASK;
        fSet[ind] &= FSTOR_FL_MASK;
        fSet[ind] += next & FSTOR_AIID_MASK;
    }
    else curr = 0;
    return curr;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::readBlock (Tbit *pDst, quint32 ind, Tbit displ, quint32 length)
{
    Tbit *pSrc = store + (Tbit)ind * TnBits + displ;
    quint32 res = 0;

    if (displ < TnBits){
        if (displ+length > TnBits)
            length = TnBits - displ;
        res = length;
        if (length > 0){
            while (length--) *(pDst++) = *(pSrc++);
        }
    }
    return res;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::writeBlock (Tbit *pSrc, quint32 ind, Tbit displ, quint32 length)
{
    Tbit *pDst = store + (Tbit)ind * TnBits + displ;
    quint32 res = 0;

    if (displ < TnBits){
        if (displ+length > TnBits)
            length = TnBits - displ;
        res = length;
        if (length > 0){
            while (length--) *(pDst++) = *(pSrc++);
        }
    }
    return res;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::clearBlock (quint32 ind, Tbit displ, quint32 length)
{
    Tbit *pDst = store + (Tbit)ind * TnBits + displ;
    quint32 res = 0;

    if (displ < TnBits){
        if (displ+length > TnBits)
            length = TnBits - displ;
        res = length;
        if (length > 0){
            while (length--) *(pDst++) = 0;
        }
    }
    return res;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
quint32 storage_t<TnObj, TnBits, Tbit, Taiid>
::findData (Taiid id, Tbit d, Tbit mask)
{
    quint32 res = 0;
    quint32 ind = (quint32) (id & FSTOR_IND_MASK);

    d &= mask;
    if( isValid( ind)){
        quint32 l = TnBits;
        Tbit *src = store + (Tbit)ind * TnBits;
        while( l-- > 0)
            if( ((*src++) & mask) == d){
                res = TnBits-1; l = 0;
            }
    }
    return res;
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
void storage_t<TnObj, TnBits, Tbit, Taiid>
::rollLeft( quint32 ind, quint32 displ, quint32 len, Tbit last)
{
    if( isValid(ind) && len > 0 && displ + len < TnBits)
    {
        Tbit *pDat = store + (Tbit)ind * TnBits + displ;
        while( len-- > 0){ *pDat = *(pDat+1); pDat++;}
        *pDat = last;
    }
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
bool storage_t<TnObj, TnBits, Tbit, Taiid>
::init (const QString &_name)
{
    bool ok = false;

    pFile = new QFile(_name);
    if( pFile){
        ok = read_all();
    }

    return ok;
}

template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
void storage_t<TnObj, TnBits, Tbit, Taiid>
::finit()
{
    if( pFile){
        save_all();
        delete pFile; pFile = NULL;
    }
}


template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
bool storage_t<TnObj, TnBits, Tbit, Taiid>
::read_all()
{
    bool ok = false;

    if( pFile->open( QIODevice::ReadOnly)){
        quint32      i = 0;
        Tbit           *st_dst;
        quint32     *p_dst;
        Taiid         *p_flags = fSet;
        pAIID       *p_obj = pObj;

        QDataStream in(pFile);
        in.setVersion( QDataStream::Qt_5_3);
        in.setByteOrder( QDataStream::BigEndian);

        in >> nSet >> nDel;
        if( !Block_isInRange( nSet)){
            nSet = N_INITIAL; nDel = N_INITIAL;
        }else if( (!Block_isInRange(nDel)) || nDel > nSet){
            nSet = N_INITIAL; nDel = N_INITIAL;
        }

//!            in >> store ;
        st_dst = store;
        for( i = nSet * TnBits; i != 0; --i) in >> *(st_dst++);
//        i = (TnObj - nSet) * TnBits * sizeof(Tbit); in.skipRawData(i);
//!            in >> flags ;
        for( i = nSet; i != 0; --i) in >> *(p_flags++);
//        i = (TnObj - nSet) * sizeof(Taiid); in.skipRawData(i);
//!            in >> length ;
        p_dst = lSet;
        for( i = nSet; i != 0; --i) in >> *(p_dst++);
//        i = (TnObj - nSet) * sizeof(quint32); in.skipRawData(i);
        p_obj = pObj;
        for( i = nSet; i != 0; --i)  *(p_obj++) = NULL;
//!            in >> qDel ;
        p_dst = qDel;
        if( nDel) for( i = nDel; i != 0; --i) in >> *(p_dst++);
//        i = (TnObj - nDel) * sizeof(quint32); in.skipRawData(i);

        quint32 end;
        in >> end;
        if( end == AI_FILE_END) ok = true; //305419896

        pFile->close();
    }
    else clear();

    return ok;
}

template <quint32 TnObj, quint32 TnBits, typename Tbit, typename Taiid>
bool storage_t<TnObj, TnBits, Tbit, Taiid>
::save_all()
{
    bool ok = false;

//    if( !pFile->exists()){
//        if( pFile->open( QIODevice::WriteOnly)){
//            quint32 sz =
//                    TnObj * TnBits * sizeof(Tbit) +
//                    TnObj * sizeof(quint32) +
//                    TnObj * sizeof(quint32) +
//                    TnObj * sizeof(Taiid) +
//                    3 * sizeof(quint32);
//            pFile->resize(sz);
//            pFile->close();
//        }
//    }
    if( pFile->open( QIODevice::ReadWrite)){
        quint32      i = 0;
        Tbit           *st_src;
        quint32     *p_src;
        Taiid         *p_flags = fSet;

        quint32 sz =
                nSet * TnBits * sizeof(Tbit) +
                nSet * sizeof(quint32) +
                nSet * sizeof(quint32) +
                nDel * sizeof(Taiid) +
                3 * sizeof(quint32);
        pFile->resize(sz);


        QDataStream out( pFile);
        out.setVersion( QDataStream::Qt_5_3);
        out.setByteOrder( QDataStream::BigEndian);

        out << nSet << nDel;

//!            out << store ;
        st_src = store;
        for( i = nSet * TnBits; i != 0; --i) out << *(st_src++);
//        i = (TnObj - nSet) * TnBits * sizeof(Tbit); out.skipRawData(i);
//!            out << flags ;
        for( i = nSet; i != 0; --i) out << *(p_flags++);
//        i = (TnObj - nSet) * sizeof(Taiid); out.skipRawData(i);
//!            out << length ;
        p_src = lSet;
        for( i = nSet; i != 0; --i) out << *(p_src++);
//        i = (TnObj - nSet) * sizeof(quint32); out.skipRawData(i);
//!            out << qDel ;
        p_src = qDel;
        if( nDel) for( i = nDel; i != 0; --i) out << *(p_src++);
//        i = (TnObj - nDel) * sizeof(quint32); out.skipRawData(i);

        quint32 end=AI_FILE_END;
        out << end;
        ok = true;

        pFile->close();
    }

    return ok;
}

#endif // DST_H
