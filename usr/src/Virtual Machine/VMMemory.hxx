#ifndef VMMEMORY_H_
#define VMMEMORY_H_

#include <stddef.h>

#define __INLINE__ static inline

typedef void * HostMemoryAddress;

typedef unsigned char byte;

typedef unsigned short hwrd;

typedef unsigned int word;

typedef union objRef objRef;

void sysError (const char *, const char *);

/*
Some kinds of objects are small enough and used often enough that it can
be worthwhile to tightly encode the entire representation (both a class
reference and a value).  We refer to them using "encoded values" and
treat a subset of the host's signed integer range this way.
*/
typedef struct
{
    bool flg : 1; /* true */
    int dat : 31;

#ifdef __cplusplus
    inline operator objRef () const;
#endif
} encVal;

__INLINE__ encVal encValueOf (int x)
{
    encVal ans = {true, x};
    return (ans);
}

__INLINE__ int intValueOf (encVal x)
{
    return (x.dat);
}

/*
The safest and easiest way to find out if a value can be embedded
(encoded) without losing information is to try it and test whether or
not it works.
*/
__INLINE__ bool canEmbed (int x)
{
    return (intValueOf (encValueOf (x)) == x);
}

/*
Objects which are not referenced using encoded values must be referenced
by other means.  We refer to them using "encoded pointers" and treat the
datum as an index into an "object table".
*/
typedef struct
{
    bool flg : 1; /* false */
    word dat : 31;

#ifdef __cplusplus
    inline operator objRef () const;
#endif
} encPtr;

__INLINE__ encPtr encIndexOf (word x)
{
    encPtr ans = {false, x};
    return (ans);
}

__INLINE__ word oteIndexOf (encPtr x)
{
    return (x.dat);
}

/*
Any part of an object representation that isn't kept in either an
encoded value or an object table entry is kept elsewhere in the host's
main memory.  We call that part of the object "von Neumann space" and
keep a pointer to it.  Mapping between field counts and address units is
done using a scale factor expressed as a shift count.  We distinguish
between objects whose fields do or don't contain object references.  We
distinguish between objects whose fields have or haven't been traced.
We call objects which might not be transitively accessible from any root
object(s) "volatile".  Within the object table, we distinguish between
entries that are or aren't available.
*/
typedef struct
{
    HostMemoryAddress vnspc; /**< where is its von Neumann space? */
    word shift : 3;          /**< how big are its fields? */
    bool orefs : 1;          /**< are its fields object references? */
    bool mrked : 1;          /**< have the fields been traced by the GC? */
    bool voltl : 1;          /** is the object volatile? */
    bool avail : 1;
    word : 25;
} otbEnt;

/*
We keep track of how large a given von Neumann space is in address
units.  This "space count" is used along with the scale factor to derive
a field count, among other things.  We also need to keep track of the
class of which a given object is an instance.
*/
typedef struct
{
    /**
     * Size in bytes of this object's von Neumann space.
     */
    word spcct;
    encPtr klass; /**< Class of this object */
} ot2Ent;

#define otbLob 0
#define otbHib 65535
#define otbDom ((otbHib + 1) - otbLob)

extern otbEnt * objTbl;
extern ot2Ent * ob2Tbl;

/*
An object reference is either an encoded value or an encoded pointer.
We distinguish one from the other by means of the flag (q.v.) defined in
both.  N.B.:  This kind of overlay definition would be safer and easier
both to specify and to use if compilers would pack a 1-bit flag and a
<wordSize-1>-bit union (resp. field) into a <wordSize>-bit struct.
*/
union objRef
{
    encVal val;
    encPtr ptr;
};

#ifdef __cplusplus
inline encVal::operator objRef () const
{
    objRef o;
    o.val = *this;
    return o;
}

inline encPtr::operator objRef () const
{
    objRef o;
    o.ptr = *this;
    return o;
}
#endif

/**
 * Is this object reference a direct value?
 */
__INLINE__ bool isValue (objRef x)
{
    return (x.val.flg == true);
}

/**
 * Is this object reference an encoded pointer?
 * i.e. is it an index into the object table?
 */
__INLINE__ bool isIndex (objRef x)
{
    return (x.ptr.flg == false);
}

/**
 * Are \p x and \p y both pointers, and if so, are they equal?
 */
__INLINE__ bool ptrEq (objRef x, objRef y)
{
    return (x.ptr.flg == y.ptr.flg && x.ptr.dat == y.ptr.dat);
}

/**
 * Are \p x and \p y either not both pointers, and if they are, do they not both
 * point to the same address?
 */
__INLINE__ bool ptrNe (objRef x, objRef y)
{
    return (x.ptr.flg != y.ptr.flg || x.ptr.dat != y.ptr.dat);
}

/**
 * The location of this object's von Neumann space.
 */
__INLINE__ HostMemoryAddress vonNeumannSpaceOf (encPtr x)
{
    return (objTbl[oteIndexOf (x)].vnspc);
}

/**
 * Set the location of this object's von Neumann space.
 */
__INLINE__ void vonNeumannSpaceOfPut (encPtr x, HostMemoryAddress v)
{
    objTbl[oteIndexOf (x)].vnspc = v;
}

/**
 * How many bits to right-shift this object's vonNeumannSpaceOf() in order to
 * get the count of items within.
 */
__INLINE__ word scaleOf (encPtr x)
{
    return (objTbl[oteIndexOf (x)].shift);
}

__INLINE__ void scaleOfPut (encPtr x, word v)
{
    objTbl[oteIndexOf (x)].shift = v;
}

/**
 * Is this object's von Neumann space composed of object references?
 */
__INLINE__ bool isObjRefs (encPtr x)
{
    return (objTbl[oteIndexOf (x)].orefs == true);
}

__INLINE__ void isObjRefsPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].orefs = v;
}

/**
 * Has this object been marked by the garbage collector?
 */
__INLINE__ bool isMarked (encPtr x)
{
    return (objTbl[oteIndexOf (x)].mrked == true);
}

__INLINE__ void isMarkedPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].mrked = v;
}

/**
 * Is this object volatile?
 * i.e. is it possible for it to become transitively inaccessible from a
 * root object object, yet still not be eligible for garbage collection?
 */
__INLINE__ bool isVolatile (encPtr x)
{
    return (objTbl[oteIndexOf (x)].voltl == true);
}

__INLINE__ void isVolatilePut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].voltl = v;
}

/**
 * Is this entry in the object table available for use? If not, it is already in
 * use by another object.
 */
__INLINE__ bool isAvail (encPtr x)
{
    return (objTbl[oteIndexOf (x)].avail == true);
}

__INLINE__ void isAvailPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].avail = v;
}

/**
 * Length in bytes of this object's von Neumann space
 */
__INLINE__ size_t vonNeumannSpaceLengthOf (encPtr x)
{
    return (ob2Tbl[oteIndexOf (x)].spcct);
}

/**
 * Sets the length in bytes of this object's von Neumann space.
 */
__INLINE__ void vonNeumannSpaceLengthOfPut (encPtr x, size_t v)
{
    ob2Tbl[oteIndexOf (x)].spcct = v;
}

__INLINE__ encPtr classOf (encPtr x)
{
    return (ob2Tbl[oteIndexOf (x)].klass);
}

__INLINE__ void classOfPut (encPtr x, encPtr v)
{
#if 0
  assert(isIndex(v));
#endif
    isVolatilePut (v, false);
    ob2Tbl[oteIndexOf (x)].klass = v;
}

/**
 * The count of fields in this object's von Neumann space.
 */
__INLINE__ word countOf (encPtr x)
{
    return (vonNeumannSpaceLengthOf (x) >> scaleOf (x));
}

/**
 * Value of this object's nth object reference slot. 1-based indexing.
 */
__INLINE__ objRef orefOf (encPtr x, word n)
{
    return (((objRef *)objTbl[oteIndexOf (x)].vnspc)[n - 1]);
}

/**
 * Set the value of this object's nth object reference slot. 1-based indexing.
 */
__INLINE__ void orefOfPut (encPtr x, word i, objRef v)
{
    if (isIndex (v))
        isVolatilePut (v.ptr, false);
    ((objRef *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

/**
 * Value of this object's nth byte slot. 1-based indexing.
 */
__INLINE__ byte byteOf (encPtr x, word i)
{
    return (((byte *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

/**
 * Set the value of this object's nth wor slot to \p v. 1-based indexing.
 */
__INLINE__ void byteOfPut (encPtr x, word i, byte v)
{
    ((byte *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

/**
 * Value of this object's nth half-word slot. 1-based indexing.
 */
__INLINE__ hwrd hwrdOf (encPtr x, word i)
{
    return (((hwrd *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

/**
 * Set the value of this object's nth half-word slot to \p v. 1-based indexing.
 */
__INLINE__ void hwrdOfPut (encPtr x, word n, hwrd v)
{
    ((hwrd *)objTbl[oteIndexOf (x)].vnspc)[n - 1] = v;
}

/**
 * Value of this object's nth word slot. 1-based indexing.
 */
__INLINE__ word wordOf (encPtr x, word i)
{
    return (((word *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

/**
 * Set the value of this object's nth word slot to \p v. 1-based indexing.
 */
__INLINE__ void wordOfPut (encPtr x, word n, word v)
{
    ((word *)objTbl[oteIndexOf (x)].vnspc)[n - 1] = v;
}

/**
 * Initialise object table
 */
void coldObjectTable (void);

/**
 *
 */
void warmObjectTableOne (void);

/**
 *
 */
void warmObjectTableTwo (void);

void initCommonSymbols (void);

/**
 * Garbage collect.
 */
void reclaim (bool all);

/**
 * Allocate host memory.
 */
HostMemoryAddress newStorage (word bytes);

/* Number of available slots in object memory */
int availCount (void);

#define pointerList encIndexOf (0)
#define globalValue(s) nameTableLookup (symbols, s)

extern encPtr symbols, classes;

#define classSize 5
#define nameInClass 1
#define sizeInClass 2
#define methodsInClass 3
#define superClassInClass 4
#define variablesInClass 5

#define methodSize 8
#define textInMethod 1
#define messageInMethod 2
#define bytecodesInMethod 3
#define literalsInMethod 4
#define stackSizeInMethod 5
#define temporarySizeInMethod 6
#define methodClassInMethod 7
#define watchInMethod 8

#define methodStackSize(x) intValueOf (orefOf (x, stackSizeInMethod).val)
#define methodTempSize(x) intValueOf (orefOf (x, temporarySizeInMethod).val)

#define contextSize 6
#define linkPtrInContext 1
#define methodInContext 2
#define argumentsInContext 3
#define temporariesInContext 4

#define blockSize 6
#define contextInBlock 1
#define argumentCountInBlock 2
#define argumentLocationInBlock 3
#define bytecountPositionInBlock 4

#define processSize 3
#define stackInProcess 1
#define stackTopInProcess 2
#define linkPtrInProcess 3

extern encPtr nilObj, trueObj, falseObj;
extern encPtr arrayClass, intClass, stringClass, symbolClass;

/* Compiler related */
extern encPtr unSyms[16], binSyms[32];

encPtr allocByteObj (word n);
encPtr allocHWrdObj (word n);
encPtr allocWordObj (word n);
encPtr allocOrefObj (word n);
encPtr allocZStrObj (word n);

encPtr newSymbol (const char * str);
double floatValue (encPtr o);
encPtr newArray (int size);
encPtr newBlock (void);
encPtr newByteArray (int size);
encPtr newChar (int value);
encPtr newClass (const char * name);
encPtr newContext (int link, encPtr method, encPtr args, encPtr temp);
encPtr newDictionary (int size);
encPtr newFloat (double d);
encPtr newLink (encPtr key, encPtr value);
encPtr newMethod (void);
encPtr newString (char * value);
encPtr newSymbol (const char * str);

encPtr nameTableLookup (encPtr dict, const char * str);
void nameTableInsert (encPtr dict, word hash, encPtr key, encPtr value);

/**
 * For each element of dictionary /p dict matching hashcode /p hash, run
 * /p fun() with the key associated with that hash. If /p fun returns non-zero
 * for that key, returns the object associated with that key.
 */
encPtr hashEachElement (encPtr dict, word hash, int (*fun) (encPtr));

int strHash (const char * str);

__INLINE__ encPtr getClass (objRef obj)
{
    if (isValue (obj))
    {
        if (ptrEq ((objRef)intClass, (objRef)nilObj))
            intClass = globalValue ("Integer");
        return (intClass);
    }
    return (classOf (obj.ptr));
}

#endif