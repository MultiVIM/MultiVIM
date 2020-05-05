#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef L2_SMALLTALK_EMBEDDED
#include <stdint.h>
#endif

#ifdef L2_SMALLTALK_EMBEDDED
#define setjmp(x) 0
typedef struct
{
} jmp_buf;
#else
#include <setjmp.h>
#endif

#ifdef L2_SMALLTALK_EMBEDDED
/* Fix to prevent using GNU macros which rely on runtime info. */
#define isdigit(c) (c >= '0' && c <= '9')
#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isalpha(c) (isupper (c) || islower (c))
#define isalnum(c) (isalpha (c) || isdigit (c))
#define isspace(c)                                                             \
    (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#else
#include <ctype.h>
#endif

/* The Smalltalk compiler (which, somehow, is written in Smalltalk) has
 * been modified to accept underscores as alphanumeric characters (but
 * not strictly as alphabetical characters or digits). The C behaviour
 * will match the higher level behaviour if L2_SMALLTALK_UNDERSCORES is
 * defined.
 */
#define L2_SMALLTALK_UNDERSCORES

#ifdef L2_SMALLTALK_UNDERSCORES
#define isalnum(c) (isalpha (c) || isdigit (c) || c == '_')
#endif

/* Fix from Zak - compilers don't deal with non-static inlines intuitively. */
#define __INLINE__ static

#define streq(a, b) (strcmp (a, b) == 0)

typedef void * addr;

typedef enum
{
    false,
    true
} bool;

typedef unsigned char byte;

typedef unsigned short hwrd;

typedef unsigned int word;

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
    addr vnspc;
    word shift : 3;
    bool orefs : 1;
    bool mrked : 1;
    bool voltl : 1;
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
    word spcct;
    encPtr class;
} ot2Ent;

#define otbLob 0
#define otbHib 65535
#define otbDom ((otbHib + 1) - otbLob)

otbEnt * objTbl = NULL;
/*otbEnt objTbl[otbDom];*/

ot2Ent * ob2Tbl = NULL;
/*ot2Ent ob2Tbl[otbDom];*/

/*
An object reference is either an encoded value or an encoded pointer.
We distinguish one from the other by means of the flag (q.v.) defined in
both.  N.B.:  This kind of overlay definition would be safer and easier
both to specify and to use if compilers would pack a 1-bit flag and a
<wordSize-1>-bit union (resp. field) into a <wordSize>-bit struct.
*/
typedef union {
    encVal val;
    encPtr ptr;
} objRef;

__INLINE__ bool isValue (objRef x)
{
    return (x.val.flg == true);
}

__INLINE__ bool isIndex (objRef x)
{
    return (x.ptr.flg == false);
}

__INLINE__ bool ptrEq (objRef x, objRef y)
{
    return (x.ptr.flg == y.ptr.flg && x.ptr.dat == y.ptr.dat);
}

__INLINE__ bool ptrNe (objRef x, objRef y)
{
    return (x.ptr.flg != y.ptr.flg || x.ptr.dat != y.ptr.dat);
}

__INLINE__ addr addressOf (encPtr x)
{
    return (objTbl[oteIndexOf (x)].vnspc);
}

__INLINE__ void addressOfPut (encPtr x, addr v)
{
    objTbl[oteIndexOf (x)].vnspc = v;
}

__INLINE__ word scaleOf (encPtr x)
{
    return (objTbl[oteIndexOf (x)].shift);
}

__INLINE__ void scaleOfPut (encPtr x, word v)
{
    objTbl[oteIndexOf (x)].shift = v;
}

__INLINE__ bool isObjRefs (encPtr x)
{
    return (objTbl[oteIndexOf (x)].orefs == true);
}

__INLINE__ void isObjRefsPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].orefs = v;
}

__INLINE__ bool isMarked (encPtr x)
{
    return (objTbl[oteIndexOf (x)].mrked == true);
}

__INLINE__ void isMarkedPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].mrked = v;
}

__INLINE__ bool isVolatile (encPtr x)
{
    return (objTbl[oteIndexOf (x)].voltl == true);
}

__INLINE__ void isVolatilePut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].voltl = v;
}

__INLINE__ bool isAvail (encPtr x)
{
    return (objTbl[oteIndexOf (x)].avail == true);
}

__INLINE__ void isAvailPut (encPtr x, bool v)
{
    objTbl[oteIndexOf (x)].avail = v;
}

__INLINE__ word spaceOf (encPtr x)
{
    return (ob2Tbl[oteIndexOf (x)].spcct);
}

__INLINE__ void spaceOfPut (encPtr x, word v)
{
    ob2Tbl[oteIndexOf (x)].spcct = v;
}

__INLINE__ encPtr classOf (encPtr x)
{
    return (ob2Tbl[oteIndexOf (x)].class);
}

__INLINE__ void classOfPut (encPtr x, encPtr v)
{
#if 0
  assert(isIndex(v));
#endif
    isVolatilePut (v, false);
    ob2Tbl[oteIndexOf (x)].class = v;
}

__INLINE__ word countOf (encPtr x)
{
    return (spaceOf (x) >> scaleOf (x));
}

__INLINE__ objRef orefOf (encPtr x, word i)
{
    return (((objRef *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

__INLINE__ void orefOfPut (encPtr x, word i, objRef v)
{
    if (isIndex (v))
        isVolatilePut (v.ptr, false);
    ((objRef *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

__INLINE__ byte byteOf (encPtr x, word i)
{
    return (((byte *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

__INLINE__ void byteOfPut (encPtr x, word i, byte v)
{
    ((byte *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

__INLINE__ hwrd hwrdOf (encPtr x, word i)
{
    return (((hwrd *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

__INLINE__ void hwrdOfPut (encPtr x, word i, hwrd v)
{
    ((hwrd *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

__INLINE__ word wordOf (encPtr x, word i)
{
    return (((word *)objTbl[oteIndexOf (x)].vnspc)[i - 1]);
}

__INLINE__ void wordOfPut (encPtr x, word i, word v)
{
    ((word *)objTbl[oteIndexOf (x)].vnspc)[i - 1] = v;
}

#define pointerList encIndexOf (0)

int availCount (void)
{
    int ans = 0;
    encPtr tmp = classOf (pointerList);
    while (oteIndexOf (tmp) != 0)
    {
        ans++;
        tmp = classOf (tmp);
    }
    return (ans);
}

void freePointer (encPtr x)
{
#if 0
  assert(false);
#endif
    scaleOfPut (x, 0);
    isObjRefsPut (x, false);
    isMarkedPut (x, false);
    isVolatilePut (x, false);
    isAvailPut (x, true);
    classOfPut (x, classOf (pointerList));
    classOfPut (pointerList, x);
}

void freeStorage (addr x)
{
#if 0
  assert(false);
#endif
    assert (x != NULL);
    free (x);
}

void visit (objRef x)
{
    if (isIndex (x))
    {
        if (isMarked (x.ptr) == false)
        {
            /* then it's the first time we've visited it, so: */
            isMarkedPut (x.ptr, true);
            visit ((objRef)classOf (x.ptr));
            if (isObjRefs (x.ptr))
            {
                objRef * f = addressOf (x.ptr);
                objRef * p = (void *)f + spaceOf (x.ptr);
                while (p != f)
                    visit (*--p);
            }
        }
    }
}

extern encPtr symbols;

/*
It's safe to ignore volatile objects only when all necessary object
references are stored in object memory.  Currently, that's the case
only upon a successful return from the interpreter.  In operation, the
interpreter does many stores directly into host memory (as opposed to
indirectly via the object table).  As a result, volatile objects will
remain flagged as such.  Tracing them ensures that they (and their
referents) get kept.
*/
void reclaim (bool all)
{
    word ord;
    encPtr ptr;
    visit ((objRef)symbols);
    if (all)
        for (ord = otbLob; ord <= otbHib; ord++)
        {
            ptr = encIndexOf (ord);
            if (isVolatile (ptr))
                visit ((objRef)ptr);
        }
    classOfPut (pointerList, encIndexOf (0));
    for (ord = otbHib; ord > otbLob; ord--)
    { /*fix*/
        ptr = encIndexOf (ord);
        if (isAvail (ptr))
        {
            freePointer (ptr);
            continue;
        }
        if (isMarked (ptr))
        {
            if (!all) /*stored but not by orefOfPut...*/
                isVolatilePut (ptr, false);
            isMarkedPut (ptr, false);
            continue;
        }
        if (spaceOf (ptr))
        {
            freeStorage (addressOf (ptr));
            addressOfPut (ptr, 0);
            spaceOfPut (ptr, 0);
        }
        freePointer (ptr);
    }
}

encPtr newPointer (void)
{
    encPtr ans = classOf (pointerList);
    if (oteIndexOf (ans) == 0)
    {
        reclaim (true);
        ans = classOf (pointerList);
    }
    assert (oteIndexOf (ans) != 0);
    classOfPut (pointerList, classOf (ans));
#if 0
  classOfPut(ans,encIndexOf(0));
#endif
    isVolatilePut (ans, true);
    isAvailPut (ans, false);
    return (ans);
}

addr newStorage (word bytes)
{
    addr ans;
    if (bytes)
    {
        ans = calloc (bytes, sizeof (byte));
        assert (ans != NULL);
    }
    else
        ans = NULL;
    return (ans);
}

void coldObjectTable (void)
{
    word i;
    objTbl = calloc (otbDom, sizeof (otbEnt));
    assert (objTbl != NULL);
    ob2Tbl = calloc (otbDom, sizeof (ot2Ent));
    assert (ob2Tbl != NULL);
    for (i = otbLob; i != otbHib; i++)
    {
        classOfPut (encIndexOf (i), encIndexOf (i + 1));
        isAvailPut (encIndexOf (i + 1), true);
    }
}

void warmObjectTableOne (void)
{
    word i;
    objTbl = calloc (otbDom, sizeof (otbEnt));
    assert (objTbl != NULL);
    ob2Tbl = calloc (otbDom, sizeof (ot2Ent));
    assert (ob2Tbl != NULL);
    for (i = otbLob; i != otbHib; i++)
        isAvailPut (encIndexOf (i + 1), true);
}

void warmObjectTableTwo (void)
{
    word i;
    classOfPut (pointerList, encIndexOf (0));
    for (i = otbHib; i > otbLob; i--) /*fix*/
        if (isAvail (encIndexOf (i)))
            freePointer (encIndexOf (i));
}

extern encPtr nilObj;

encPtr allocOrefObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 2; /*fix*/
    addr mem = newStorage (num);
    addressOfPut (ptr, mem);
    scaleOfPut (ptr, 2); /*fix*/
    isObjRefsPut (ptr, true);
    spaceOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    while (n--)
    {
        /* Fix from Zak - modern compilers don't like assign-to-cast. */
        encPtr * tmpmem = (encPtr *)mem;
        *tmpmem++ = nilObj;
        mem = (addr)tmpmem;
    }
    return (ptr);
}

encPtr allocByteObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 0; /*fix*/
    addr mem = newStorage (num);
    addressOfPut (ptr, mem);
    scaleOfPut (ptr, 0); /*fix*/
    isObjRefsPut (ptr, false);
    spaceOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocHWrdObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 1; /*fix*/
    addr mem = newStorage (num);
    addressOfPut (ptr, mem);
    scaleOfPut (ptr, 1); /*fix*/
    isObjRefsPut (ptr, false);
    spaceOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocWordObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 2; /*fix*/
    addr mem = newStorage (num);
    addressOfPut (ptr, mem);
    scaleOfPut (ptr, 2); /*fix*/
    isObjRefsPut (ptr, false);
    spaceOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocZStrObj (char * zstr)
{
    encPtr ptr = newPointer ();
    word num = strlen (zstr) + 1;
    addr mem = newStorage (num);
    addressOfPut (ptr, mem);
    scaleOfPut (ptr, 0); /*fix*/
    isObjRefsPut (ptr, false);
    spaceOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    (void)strcpy (addressOf (ptr), zstr);
    return (ptr);
}

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

encPtr nilObj = {false, 1}; /* pseudo variable nil */

encPtr trueObj = {false, 2};  /* pseudo variable true */
encPtr falseObj = {false, 3}; /* pseudo variable false */

#if 0
encPtr hashTable = {false,4};
#endif
encPtr symbols = {false, 5};
encPtr classes = {false, 1};

encPtr unSyms[16] = {};
encPtr binSyms[32] = {};

#define globalValue(s) nameTableLookup (symbols, s)

void sysError (char *, char *);

encPtr newLink (encPtr key, encPtr value);

void nameTableInsert (encPtr dict, word hash, encPtr key, encPtr value)
{
    encPtr table, link, nwLink, nextLink, tablentry;

    /* first get the hash table */
    table = orefOf (dict, 1).ptr;

    if (countOf (table) < 3)
        sysError ("attempt to insert into", "too small name table");
    else
    {
        hash = 3 * (hash % (countOf (table) / 3));
        assert (hash <= countOf (table) - 3);
        tablentry = orefOf (table, hash + 1).ptr;
        if (ptrEq ((objRef)tablentry, (objRef)nilObj) ||
            ptrEq ((objRef)tablentry, (objRef)key))
        {
            orefOfPut (table, hash + 1, (objRef)key);
            orefOfPut (table, hash + 2, (objRef)value);
        }
        else
        {
            nwLink = newLink (key, value);
            link = orefOf (table, hash + 3).ptr;
            if (ptrEq ((objRef)link, (objRef)nilObj))
            {
                orefOfPut (table, hash + 3, (objRef)nwLink);
            }
            else
                while (1)
                    if (ptrEq (orefOf (link, 1), (objRef)key))
                    {
                        /* get rid of unwanted Link */
                        isVolatilePut (nwLink, false);
                        orefOfPut (link, 2, (objRef)value);
                        break;
                    }
                    else if (ptrEq ((objRef) (nextLink = orefOf (link, 3).ptr),
                                    (objRef)nilObj))
                    {
                        orefOfPut (link, 3, (objRef)nwLink);
                        break;
                    }
                    else
                        link = nextLink;
        }
    }
}

encPtr hashEachElement (encPtr dict, word hash, int (*fun) (encPtr))
{
    encPtr table, key, value, link;
    encPtr * hp;
    word tablesize;

    table = orefOf (dict, 1).ptr;

    /* now see if table is valid */
    if ((tablesize = countOf (table)) < 3)
        sysError ("system error", "lookup on null table");
    else
    {
        hash = 1 + (3 * (hash % (tablesize / 3)));
        assert (hash <= tablesize - 2);
        hp = (encPtr *)addressOf (table) + (hash - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if (ptrNe ((objRef)key, (objRef)nilObj) && (*fun) (key))
            return value;
        for (link = *hp; ptrNe ((objRef)link, (objRef)nilObj); link = *hp)
        {
            hp = addressOf (link);
            key = *hp++;   /* link at: 1 */
            value = *hp++; /* link at: 2 */
            if (ptrNe ((objRef)key, (objRef)nilObj) && (*fun) (key))
                return value;
        }
    }
    return nilObj;
}

int strHash (char * str)
{
    int hash;
    char * p;

    hash = 0;
    for (p = str; *p; p++)
        hash += *p;
    if (hash < 0)
        hash = -hash;
    /* make sure it can be a smalltalk integer */
    if (hash > 16384)
        hash >>= 2;
    return hash;
}

word symHash (encPtr sym)
{
    return (oteIndexOf (sym));
}

char * charBuffer = 0;
encPtr objBuffer = {true, 0};

int strTest (encPtr key)
{
    if (addressOf (key) && streq (addressOf (key), charBuffer))
    {
        objBuffer = key;
        return 1;
    }
    return 0;
}

encPtr globalKey (char * str)
{
    charBuffer = str;
    objBuffer = nilObj;
    (void)hashEachElement (symbols, strHash (str), strTest);
    return objBuffer;
}

encPtr nameTableLookup (encPtr dict, char * str)
{
    charBuffer = str;
    return hashEachElement (dict, strHash (str), strTest);
}

char * unStrs[] = {"isNil",
                   "notNil",
                   "value",
                   "new",
                   "class",
                   "size",
                   "basicSize",
                   "print",
                   "printString",
                   0};

char * binStrs[] = {"+",
                    "-",
                    "<",
                    ">",
                    "<=",
                    ">=",
                    "=",
                    "~=",
                    "*",
                    "quo:",
                    "rem:",
                    "bitAnd:",
                    "bitXor:",
                    "==",
                    ",",
                    "at:",
                    "basicAt:",
                    "do:",
                    "coerce:",
                    "error:",
                    "includesKey:",
                    "isMemberOf:",
                    "new:",
                    "to:",
                    "value:",
                    "whileTrue:",
                    "addFirst:",
                    "addLast:",
                    0};

encPtr newSymbol (char * str);

void initCommonSymbols (void)
{
    int i;

    assert (ptrEq ((objRef)nilObj, (objRef)globalValue ("nil")));

    assert (ptrEq ((objRef)trueObj, (objRef)globalValue ("true")));
    assert (ptrEq ((objRef)falseObj, (objRef)globalValue ("false")));

#if 0
  assert(ptrEq(hashTable,globalValue("hashTable")));
#endif
    assert (ptrEq ((objRef)symbols, (objRef)globalValue ("symbols")));
    classes = globalValue ("classes");

    for (i = 0; i != 16; i++)
        unSyms[i] = nilObj;
    for (i = 0; unStrs[i]; i++)
        unSyms[i] = newSymbol (unStrs[i]);
    for (i = 0; i != 32; i++)
        binSyms[i] = nilObj;
    for (i = 0; binStrs[i]; i++)
        binSyms[i] = newSymbol (binStrs[i]);
}

encPtr arrayClass = {false, 1};  /* the class Array */
encPtr intClass = {false, 1};    /* the class Integer */
encPtr stringClass = {false, 1}; /* the class String */
encPtr symbolClass = {false, 1}; /* the class Symbol */

double floatValue (encPtr o)
{
    double d;

    (void)memcpy (&d, addressOf (o), sizeof (double));
    return d;
}

encPtr newArray (int size)
{
    encPtr newObj;

    newObj = allocOrefObj (size);
    if (ptrEq ((objRef)arrayClass, (objRef)nilObj))
        arrayClass = globalValue ("Array");
    classOfPut (newObj, arrayClass);
    return newObj;
}

encPtr newBlock (void)
{
    encPtr newObj;

    newObj = allocOrefObj (blockSize);
    classOfPut (newObj, globalValue ("Block"));
    return newObj;
}

encPtr newByteArray (int size)
{
    encPtr newobj;

    newobj = allocByteObj (size);
    classOfPut (newobj, globalValue ("ByteArray"));
    return newobj;
}

encPtr newChar (int value)
{
    encPtr newobj;

    newobj = allocOrefObj (1);
    orefOfPut (newobj, 1, (objRef)encValueOf (value));
    classOfPut (newobj, globalValue ("Char"));
    return (newobj);
}

encPtr newClass (char * name)
{
    encPtr newMeta;
    encPtr newInst;
    char * metaName;
    encPtr nameMeta;
    encPtr nameInst;

    newMeta = allocOrefObj (classSize);
    classOfPut (newMeta, globalValue ("Metaclass"));
    orefOfPut (newMeta, sizeInClass, (objRef)encValueOf (classSize));
    newInst = allocOrefObj (classSize);
    classOfPut (newInst, newMeta);

    metaName = newStorage (strlen (name) + 4 + 1);
    (void)strcpy (metaName, name);
    (void)strcat (metaName, "Meta");

    /* now make names */
    nameMeta = newSymbol (metaName);
    orefOfPut (newMeta, nameInClass, (objRef)nameMeta);
    nameInst = newSymbol (name);
    orefOfPut (newInst, nameInClass, (objRef)nameInst);

    /* now put in global symbols and classes tables */
    nameTableInsert (symbols, strHash (metaName), nameMeta, newMeta);
    nameTableInsert (symbols, strHash (name), nameInst, newInst);
    if (ptrNe ((objRef)classes, (objRef)nilObj))
    {
        nameTableInsert (classes, symHash (nameMeta), nameMeta, newMeta);
        nameTableInsert (classes, symHash (nameInst), nameInst, newInst);
    }

    freeStorage (metaName);

    return (newInst);
}

encPtr newContext (int link, encPtr method, encPtr args, encPtr temp)
{
    encPtr newObj;

    newObj = allocOrefObj (contextSize);
    classOfPut (newObj, globalValue ("Context"));
    orefOfPut (newObj, linkPtrInContext, (objRef)encValueOf (link));
    orefOfPut (newObj, methodInContext, (objRef)method);
    orefOfPut (newObj, argumentsInContext, (objRef)args);
    orefOfPut (newObj, temporariesInContext, (objRef)temp);
    return newObj;
}

encPtr newDictionary (int size)
{
    encPtr newObj;

    newObj = allocOrefObj (1);
    classOfPut (newObj, globalValue ("Dictionary"));
    orefOfPut (newObj, 1, (objRef)newArray (size));
    return newObj;
}

encPtr newFloat (double d)
{
    encPtr newObj;

    newObj = allocByteObj (sizeof (double));
    (void)memcpy (addressOf (newObj), &d, sizeof (double));
    classOfPut (newObj, globalValue ("Float"));
    return newObj;
}

encPtr newLink (encPtr key, encPtr value)
{
    encPtr newObj;

    newObj = allocOrefObj (3);
    classOfPut (newObj, globalValue ("Link"));
    orefOfPut (newObj, 1, (objRef)key);
    orefOfPut (newObj, 2, (objRef)value);
    return newObj;
}

encPtr newMethod (void)
{
    encPtr newObj;

    newObj = allocOrefObj (methodSize);
    classOfPut (newObj, globalValue ("Method"));
    return newObj;
}

encPtr newString (char * value)
{
    encPtr newObj;

    newObj = allocZStrObj (value);
    if (ptrEq ((objRef)stringClass, (objRef)nilObj))
        stringClass = globalValue ("String");
    classOfPut (newObj, stringClass);
    return (newObj);
}

encPtr newSymbol (char * str)
{
    encPtr newObj;

    /* first see if it is already there */
    newObj = globalKey (str);
    if (ptrNe ((objRef)newObj, (objRef)nilObj))
        return newObj;

    /* not found, must make */
    newObj = allocZStrObj (str);
    if (ptrEq ((objRef)symbolClass, (objRef)nilObj))
        symbolClass = globalValue ("Symbol");
    classOfPut (newObj, symbolClass);
    nameTableInsert (symbols, strHash (str), newObj, nilObj);
    return newObj;
}

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

typedef enum tokensyms
{
    nothing,
    nameconst,
    namecolon,
    intconst,
    floatconst,
    charconst,
    symconst,
    arraybegin,
    strconst,
    binary,
    closing,
    inputend
} tokentype;

tokentype token = nothing;
char tokenString[4096] = {}; /* text of current token */
int tokenInteger = 0;        /* or character */
double tokenFloat = 0.0;

char * cp = 0;
char cc = 0;

int pushindex = 0;
char pushBuffer[16] = {};

long longresult = 0; /*fix*/

void pushBack (char c)
{
    pushBuffer[pushindex++] = c;
}

char nextChar (void)
{
    if (pushindex > 0)
        cc = pushBuffer[--pushindex];
    else if (*cp)
        cc = *cp++;
    else
        cc = '\0';
    return (cc);
}

char peek (void)
{
    pushBack (nextChar ());
    return (cc);
}

bool isClosing (char c)
{
    switch (c)
    {
    case '.':
    case ']':
    case ')':
    case ';':
    case '\"':
    case '\'':
        return (true);
    }
    return (false);
}

bool isSymbolChar (char c)
{
    if (isdigit (c) || isalpha (c))
        return (true);
    if (isspace (c) || isClosing (c))
        return (false);
    return (true);
}

bool singleBinary (char c)
{
    switch (c)
    {
    case '[':
    case '(':
    case ')':
    case ']':
        return (true);
    }
    return (false);
}

bool binarySecond (char c)
{
    if (isalpha (c) || isdigit (c) || isspace (c) || isClosing (c) ||
        singleBinary (c))
        return (false);
    return (true);
}

tokentype nextToken (void)
{
    char * tp;
    bool sign;

    /* skip over blanks and comments */
    while (nextChar () && (isspace (cc) || (cc == '"')))
        if (cc == '"')
        {
            /* read comment */
            while (nextChar () && (cc != '"'))
                ;
            if (!cc)
                break; /* break if we run into eof */
        }
    tp = tokenString;
    *tp++ = cc;

    if (!cc) /* end of input */
        token = inputend;

    else if (isalpha (cc))
    { /* identifier */
        while (nextChar () && isalnum (cc))
            *tp++ = cc;
        if (cc == ':')
        {
            *tp++ = cc;
            token = namecolon;
        }
        else
        {
            pushBack (cc);
            token = nameconst;
        }
    }
    else if (isdigit (cc))
    { /* number */
        longresult = cc - '0';
        while (nextChar () && isdigit (cc))
        {
            *tp++ = cc;
            longresult = (longresult * 10) + (cc - '0');
        }
        if (canEmbed (longresult))
        {
            tokenInteger = longresult;
            token = intconst;
        }
        else
        {
            token = floatconst;
            tokenFloat = (double)longresult;
        }
        if (cc == '.')
        { /* possible float */
            if (nextChar () && isdigit (cc))
            {
                *tp++ = '.';
                do
                    *tp++ = cc;
                while (nextChar () && isdigit (cc));
                if (cc)
                    pushBack (cc);
                token = floatconst;
                *tp = '\0';
                tokenFloat = atof (tokenString);
            }
            else
            {
                /* nope, just an ordinary period */
                if (cc)
                    pushBack (cc);
                pushBack ('.');
            }
        }
        else
            pushBack (cc);

        if (nextChar () && cc == 'e')
        { /* possible float */
            if (nextChar () && cc == '-')
            {
                sign = true;
                (void)nextChar ();
            }
            else
                sign = false;
            if (cc && isdigit (cc))
            { /* yep, its a float */
                *tp++ = 'e';
                if (sign)
                    *tp++ = '-';
                while (cc && isdigit (cc))
                {
                    *tp++ = cc;
                    (void)nextChar ();
                }
                if (cc)
                    pushBack (cc);
                *tp = '\0';
                token = floatconst;
                tokenFloat = atof (tokenString);
            }
            else
            { /* nope, wrong again */
                if (cc)
                    pushBack (cc);
                if (sign)
                    pushBack ('-');
                pushBack ('e');
            }
        }
        else if (cc)
            pushBack (cc);
    }
    else if (cc == '$')
    { /* character constant */
        tokenInteger = (int)nextChar ();
        token = charconst;
    }
    else if (cc == '#')
    {         /* symbol */
        tp--; /* erase pound sign */
        if (nextChar () == '(')
            token = arraybegin;
        else
        {
            pushBack (cc);
            while (nextChar () && isSymbolChar (cc))
                *tp++ = cc;
            pushBack (cc);
            token = symconst;
        }
    }
    else if (cc == '\'')
    {         /* string constant */
        tp--; /* erase pound sign */
    strloop:
        while (nextChar () && (cc != '\''))
            *tp++ = cc;
        /* check for nested quote marks */
        if (cc && nextChar () && (cc == '\''))
        {
            *tp++ = cc;
            goto strloop;
        }
        pushBack (cc);
        token = strconst;
    }
    else if (isClosing (cc)) /* closing expressions */
        token = closing;

    else if (singleBinary (cc))
    { /* single binary expressions */
        token = binary;
    }
    else
    { /* anything else is binary */
        if (nextChar () && binarySecond (cc))
            *tp++ = cc;
        else
            pushBack (cc);
        token = binary;
    }

    *tp = '\0';
    return (token);
}

void lexinit (char * str)
{
    pushindex = 0;
    cp = str;
    /* get first token */
    (void)nextToken ();
}

#define Extended 0
#define PushInstance 1
#define PushArgument 2
#define PushTemporary 3
#define PushLiteral 4
#define PushConstant 5
#define AssignInstance 6
#define AssignTemporary 7
#define MarkArguments 8
#define SendMessage 9
#define SendUnary 10
#define SendBinary 11
#define DoPrimitive 13
#define DoSpecial 15

#define minusOne 3     /* the value -1 */
#define contextConst 4 /* the current context */
#define nilConst 5     /* the constant nil */
#define trueConst 6    /* the constant true */
#define falseConst 7   /* the constant false */

#define SelfReturn 1
#define StackReturn 2
#define Duplicate 4
#define PopTop 5
#define Branch 6
#define BranchIfTrue 7
#define BranchIfFalse 8
#define AndBranch 9
#define OrBranch 10
#define SendToSuper 11

void sysWarn (char * s1, char * s2);
void compilWarn (char * selector, char * str1, char * str2);
void compilError (char * selector, char * str1, char * str2);

#define codeLimit 256
#define literalLimit 256
#define temporaryLimit 256
#define argumentLimit 256
#define instanceLimit 256

bool parseOk = false;

int codeTop = 0;
byte codeArray[codeLimit] = {};

int literalTop = 0;
objRef literalArray[literalLimit] = {};

int temporaryTop = 0;
char * temporaryName[temporaryLimit] = {};

int argumentTop = 0;
char * argumentName[argumentLimit] = {};

int instanceTop = 0;
char * instanceName[instanceLimit] = {};

int maxTemporary = 0;
char selector[4096] = {};

enum blockstatus
{
    NotInBlock,
    InBlock,
    OptimizedBlock
} blockstat = NotInBlock;

void setInstanceVariables (encPtr aClass)
{
    int i, limit;
    encPtr vars;

    if (ptrEq ((objRef)aClass, (objRef)nilObj))
        instanceTop = 0;
    else
    {
        setInstanceVariables (orefOf (aClass, superClassInClass).ptr);
        vars = orefOf (aClass, variablesInClass).ptr;
        if (ptrNe ((objRef)vars, (objRef)nilObj))
        {
            limit = countOf (vars);
            for (i = 1; i <= limit; i++)
                instanceName[++instanceTop] = addressOf (orefOf (vars, i).ptr);
        }
    }
}

void genMessage (bool toSuper, int argumentCount, encPtr messagesym);
void expression (void);
void parsePrimitive (void);
void block (void);
void body (void);
void assignment (char * name);

void genCode (int value)
{
    if (codeTop >= codeLimit)
        compilError (selector, "too many bytecode instructions in method", "");
    else
        codeArray[codeTop++] = value;
}

void genInstruction (int high, int low)
{
    if (low >= 16)
    {
        genInstruction (Extended, high);
        genCode (low);
    }
    else
        genCode (high * 16 + low);
}

int genLiteral (objRef aLiteral)
{
    if (literalTop >= literalLimit)
        compilError (selector, "too many literals in method", "");
    else
    {
        literalArray[++literalTop] = aLiteral;
    }
    return (literalTop - 1);
}

void genInteger (int val)
{
    if (val == -1)
        genInstruction (PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction (PushConstant, val);
    else
        genInstruction (PushLiteral, genLiteral ((objRef)encValueOf (val)));
}

char * glbsyms[] = {"currentInterpreter", "nil", "true", "false", 0};

bool nameTerm (char * name)
{
    int i;
    bool done = false;
    bool isSuper = false;

    /* it might be self or super */
    if (streq (name, "self") || streq (name, "super"))
    {
        genInstruction (PushArgument, 0);
        done = true;
        if (streq (name, "super"))
            isSuper = true;
    }
    /* or it might be a temporary (reverse this to get most recent first) */
    if (!done)
        for (i = temporaryTop; (!done) && (i >= 1); i--)
            if (streq (name, temporaryName[i]))
            {
                genInstruction (PushTemporary, i - 1);
                done = true;
            }
    /* or it might be an argument */
    if (!done)
        for (i = 1; (!done) && (i <= argumentTop); i++)
            if (streq (name, argumentName[i]))
            {
                genInstruction (PushArgument, i);
                done = true;
            }
    /* or it might be an instance variable */
    if (!done)
        for (i = 1; (!done) && (i <= instanceTop); i++)
        {
            if (streq (name, instanceName[i]))
            {
                genInstruction (PushInstance, i - 1);
                done = true;
            }
        }
    /* or it might be a global constant */
    if (!done)
        for (i = 0; (!done) && glbsyms[i]; i++)
            if (streq (name, glbsyms[i]))
            {
                genInstruction (PushConstant, i + 4);
                done = true;
            }
    /* not anything else, it must be a global */
    /* must look it up at run time */
    if (!done)
    {
        genInstruction (PushLiteral, genLiteral ((objRef)newSymbol (name)));
        genMessage (false, 0, newSymbol ("value"));
    }
    return (isSuper);
}

int parseArray (void)
{
    int i, size, base;
    encPtr newLit;
    objRef obj;

    base = literalTop;
    (void)nextToken ();
    while (parseOk && (token != closing))
    {
        switch (token)
        {
        case arraybegin:
            (void)parseArray ();
            break;

        case intconst:
            (void)genLiteral ((objRef)encValueOf (tokenInteger));
            (void)nextToken ();
            break;

        case floatconst:
            (void)genLiteral ((objRef)newFloat (tokenFloat));
            (void)nextToken ();
            break;

        case nameconst:
        case namecolon:
        case symconst:
            (void)genLiteral ((objRef)newSymbol (tokenString));
            (void)nextToken ();
            break;

        case binary:
            if (streq (tokenString, "("))
            {
                (void)parseArray ();
                break;
            }
            if (streq (tokenString, "-") && isdigit (peek ()))
            {
                (void)nextToken ();
                if (token == intconst)
                    (void)genLiteral ((objRef)encValueOf (-tokenInteger));
                else if (token == floatconst)
                {
                    (void)genLiteral ((objRef)newFloat (-tokenFloat));
                }
                else
                    compilError (
                        selector, "negation not followed", "by number");
                (void)nextToken ();
                break;
            }
            (void)genLiteral ((objRef)newSymbol (tokenString));
            (void)nextToken ();
            break;

        case charconst:
            (void)genLiteral ((objRef)newChar (tokenInteger));
            (void)nextToken ();
            break;

        case strconst:
            (void)genLiteral ((objRef)newString (tokenString));
            (void)nextToken ();
            break;

        default:
            compilError (
                selector, "illegal text in literal array", tokenString);
            (void)nextToken ();
            break;
        }
    }

    if (parseOk)
        if (!streq (tokenString, ")"))
            compilError (selector,
                         "array not terminated by right parenthesis",
                         tokenString);
        else
            (void)nextToken ();
    size = literalTop - base;
    newLit = newArray (size);
    for (i = size; i >= 1; i--)
    {
        obj = literalArray[literalTop];
        orefOfPut (newLit, i, obj);
        literalArray[literalTop] = (objRef)nilObj;
        literalTop = literalTop - 1;
    }
    return (genLiteral ((objRef)newLit));
}

bool term (void)
{
    bool superTerm = false; /* true if term is pseudo var super */

    if (token == nameconst)
    {
        superTerm = nameTerm (tokenString);
        (void)nextToken ();
    }
    else if (token == intconst)
    {
        genInteger (tokenInteger);
        (void)nextToken ();
    }
    else if (token == floatconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newFloat (tokenFloat)));
        (void)nextToken ();
    }
    else if ((token == binary) && streq (tokenString, "-"))
    {
        (void)nextToken ();
        if (token == intconst)
            genInteger (-tokenInteger);
        else if (token == floatconst)
        {
            genInstruction (PushLiteral,
                            genLiteral ((objRef)newFloat (-tokenFloat)));
        }
        else
            compilError (selector, "negation not followed", "by number");
        (void)nextToken ();
    }
    else if (token == charconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newChar (tokenInteger)));
        (void)nextToken ();
    }
    else if (token == symconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newSymbol (tokenString)));
        (void)nextToken ();
    }
    else if (token == strconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newString (tokenString)));
        (void)nextToken ();
    }
    else if (token == arraybegin)
    {
        genInstruction (PushLiteral, parseArray ());
    }
    else if ((token == binary) && streq (tokenString, "("))
    {
        (void)nextToken ();
        expression ();
        if (parseOk)
            if ((token != closing) || !streq (tokenString, ")"))
                compilError (selector, "Missing Right Parenthesis", "");
            else
                (void)nextToken ();
    }
    else if ((token == binary) && streq (tokenString, "<"))
        parsePrimitive ();
    else if ((token == binary) && streq (tokenString, "["))
        block ();
    else
        compilError (selector, "invalid expression start", tokenString);

    return (superTerm);
}

void parsePrimitive (void)
{
    int primitiveNumber, argumentCount;

    if (nextToken () != intconst)
        compilError (selector, "primitive number missing", "");
    primitiveNumber = tokenInteger;
    (void)nextToken ();
    argumentCount = 0;
    while (parseOk && !((token == binary) && streq (tokenString, ">")))
    {
        (void)term ();
        argumentCount++;
    }
    genInstruction (DoPrimitive, argumentCount);
    genCode (primitiveNumber);
    (void)nextToken ();
}

void genMessage (bool toSuper, int argumentCount, encPtr messagesym)
{
    bool sent = false;
    int i;

    if ((!toSuper) && (argumentCount == 0))
        for (i = 0; (!sent) && ptrNe ((objRef)unSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)unSyms[i]))
            {
                genInstruction (SendUnary, i);
                sent = true;
            }
    if ((!toSuper) && (argumentCount == 1))
        for (i = 0; (!sent) && ptrNe ((objRef)binSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)binSyms[i]))
            {
                genInstruction (SendBinary, i);
                sent = true;
            }
    if (!sent)
    {
        genInstruction (MarkArguments, 1 + argumentCount);
        if (toSuper)
        {
            genInstruction (DoSpecial, SendToSuper);
            genCode (genLiteral ((objRef)messagesym));
        }
        else
            genInstruction (SendMessage, genLiteral ((objRef)messagesym));
    }
}

bool unaryContinuation (bool superReceiver)
{
    int i;
    bool sent;

    while (parseOk && (token == nameconst))
    {
        /* first check to see if it could be a temp by mistake */
        for (i = 1; i < temporaryTop; i++)
            if (streq (tokenString, temporaryName[i]))
                compilWarn (
                    selector, "message same as temporary:", tokenString);
        for (i = 1; i < argumentTop; i++)
            if (streq (tokenString, argumentName[i]))
                compilWarn (selector, "message same as argument:", tokenString);
        /* the next generates too many spurious messages */
        /* for (i=1; i < instanceTop; i++)
           if (streq(tokenString, instanceName[i]))
           compilWarn(selector,"message same as instance",
           tokenString); */

        sent = false;

        if (!sent)
        {
            genMessage (superReceiver, 0, newSymbol (tokenString));
        }
        /* once a message is sent to super, reciever is not super */
        superReceiver = false;
        (void)nextToken ();
    }
    return (superReceiver);
}

bool binaryContinuation (bool superReceiver)
{
    bool superTerm;
    encPtr messagesym;

    superReceiver = unaryContinuation (superReceiver);
    while (parseOk && (token == binary))
    {
        messagesym = newSymbol (tokenString);
        (void)nextToken ();
        superTerm = term ();
        (void)unaryContinuation (superTerm);
        genMessage (superReceiver, 1, messagesym);
        superReceiver = false;
    }
    return (superReceiver);
}

int optimizeBlock (int instruction, bool dopop)
{
    int location;
    enum blockstatus savebstat;

    savebstat = blockstat;
    genInstruction (DoSpecial, instruction);
    location = codeTop;
    genCode (0);
    if (dopop)
        genInstruction (DoSpecial, PopTop);
    (void)nextToken ();
    if (streq (tokenString, "["))
    {
        (void)nextToken ();
        if (blockstat == NotInBlock)
            blockstat = OptimizedBlock;
        body ();
        if (!streq (tokenString, "]"))
            compilError (selector, "missing close", "after block");
        (void)nextToken ();
    }
    else
    {
        (void)binaryContinuation (term ());
        genMessage (false, 0, newSymbol ("value"));
    }
    codeArray[location] = codeTop + 1;
    blockstat = savebstat;
    return (location);
}

bool keyContinuation (bool superReceiver)
{
    int i, j, argumentCount;
    bool sent, superTerm;
    encPtr messagesym;
    char pattern[4096];

    superReceiver = binaryContinuation (superReceiver);
    if (token == namecolon)
    {
        if (streq (tokenString, "ifTrue:"))
        {
            i = optimizeBlock (BranchIfFalse, false);
            if (streq (tokenString, "ifFalse:"))
            {
                codeArray[i] = codeTop + 3;
                (void)optimizeBlock (Branch, true);
            }
        }
        else if (streq (tokenString, "ifFalse:"))
        {
            i = optimizeBlock (BranchIfTrue, false);
            if (streq (tokenString, "ifTrue:"))
            {
                codeArray[i] = codeTop + 3;
                (void)optimizeBlock (Branch, true);
            }
        }
        else if (streq (tokenString, "whileTrue:"))
        {
            j = codeTop;
            genInstruction (DoSpecial, Duplicate);
            genMessage (false, 0, newSymbol ("value"));
            i = optimizeBlock (BranchIfFalse, false);
            genInstruction (DoSpecial, PopTop);
            genInstruction (DoSpecial, Branch);
            genCode (j + 1);
            codeArray[i] = codeTop + 1;
            genInstruction (DoSpecial, PopTop);
        }
        else if (streq (tokenString, "and:"))
            (void)optimizeBlock (AndBranch, false);
        else if (streq (tokenString, "or:"))
            (void)optimizeBlock (OrBranch, false);
        else
        {
            pattern[0] = '\0';
            argumentCount = 0;
            while (parseOk && (token == namecolon))
            {
                (void)strcat (pattern, tokenString);
                argumentCount++;
                (void)nextToken ();
                superTerm = term ();
                (void)binaryContinuation (superTerm);
            }
            sent = false;

            /* check for predefined messages */
            messagesym = newSymbol (pattern);

            if (!sent)
            {
                genMessage (superReceiver, argumentCount, messagesym);
            }
        }
        superReceiver = false;
    }
    return (superReceiver);
}

void continuation (bool superReceiver)
{
    superReceiver = keyContinuation (superReceiver);

    while (parseOk && (token == closing) && streq (tokenString, ";"))
    {
        genInstruction (DoSpecial, Duplicate);
        (void)nextToken ();
        (void)keyContinuation (superReceiver);
        genInstruction (DoSpecial, PopTop);
    }
}

void expression (void)
{
    bool superTerm;
    char assignname[4096];

    if (token == nameconst)
    { /* possible assignment */
        (void)strcpy (assignname, tokenString);
        (void)nextToken ();
        if ((token == binary) && streq (tokenString, "<-"))
        {
            (void)nextToken ();
            assignment (assignname);
        }
        else
        { /* not an assignment after all */
            superTerm = nameTerm (assignname);
            continuation (superTerm);
        }
    }
    else
    {
        superTerm = term ();
        if (parseOk)
            continuation (superTerm);
    }
}

void assignment (char * name)
{
    int i;
    bool done;

    done = false;

    /* it might be a temporary */
    for (i = temporaryTop; (!done) && (i > 0); i--)
        if (streq (name, temporaryName[i]))
        {
            expression ();
            genInstruction (AssignTemporary, i - 1);
            done = true;
        }
    /* or it might be an instance variable */
    for (i = 1; (!done) && (i <= instanceTop); i++)
        if (streq (name, instanceName[i]))
        {
            expression ();
            genInstruction (AssignInstance, i - 1);
            done = true;
        }
    if (!done)
    { /* not known, handle at run time */
        genInstruction (PushArgument, 0);
        genInstruction (PushLiteral, genLiteral ((objRef)newSymbol (name)));
        expression ();
        genMessage (false, 2, newSymbol ("assign:value:"));
    }
}

void statement (void)
{

    if ((token == binary) && streq (tokenString, "^"))
    {
        (void)nextToken ();
        expression ();
        if (blockstat == InBlock)
        {
            /* change return point before returning */
            genInstruction (PushConstant, contextConst);
            genMessage (false, 0, newSymbol ("blockReturn"));
            genInstruction (DoSpecial, PopTop);
        }
        genInstruction (DoSpecial, StackReturn);
    }
    else
    {
        expression ();
    }
}

void body (void)
{
    /* empty blocks are same as nil */
    if ((blockstat == InBlock) || (blockstat == OptimizedBlock))
        if ((token == closing) && streq (tokenString, "]"))
        {
            genInstruction (PushConstant, nilConst);
            return;
        }
    while (parseOk)
    {
        statement ();
        if (token == closing)
            if (streq (tokenString, "."))
            {
                (void)nextToken ();
                if (token == inputend)
                    break;
                else /* pop result, go to next statement */
                    genInstruction (DoSpecial, PopTop);
            }
            else
                break; /* leaving result on stack */
        else if (token == inputend)
            break; /* leaving result on stack */
        else
        {
            compilError (
                selector, "invalid statement ending; token is ", tokenString);
        }
    }
}

void block (void)
{
    int saveTemporary, argumentCount, fixLocation;
    encPtr tempsym, newBlk;
    enum blockstatus savebstat;

    saveTemporary = temporaryTop;
    savebstat = blockstat;
    argumentCount = 0;
    (void)nextToken ();
    if ((token == binary) && streq (tokenString, ":"))
    {
        while (parseOk && (token == binary) && streq (tokenString, ":"))
        {
            if (nextToken () != nameconst)
                compilError (selector,
                             "name must follow colon",
                             "in block argument list");
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            argumentCount++;
            if (temporaryTop > temporaryLimit)
                compilError (selector, "too many temporaries in method", "");
            else
            {
                tempsym = newSymbol (tokenString);
                temporaryName[temporaryTop] = addressOf (tempsym);
            }
            (void)nextToken ();
        }
        if ((token != binary) || !streq (tokenString, "|"))
            compilError (
                selector, "block argument list must be terminated", "by |");
        (void)nextToken ();
    }
    newBlk = newBlock ();
    orefOfPut (
        newBlk, argumentCountInBlock, (objRef)encValueOf (argumentCount));
    orefOfPut (newBlk,
               argumentLocationInBlock,
               (objRef)encValueOf (saveTemporary + 1));
    genInstruction (PushLiteral, genLiteral ((objRef)newBlk));
    genInstruction (PushConstant, contextConst);
    genInstruction (DoPrimitive, 2);
    genCode (29);
    genInstruction (DoSpecial, Branch);
    fixLocation = codeTop;
    genCode (0);
    /*genInstruction(DoSpecial, PopTop); */
    orefOfPut (
        newBlk, bytecountPositionInBlock, (objRef)encValueOf (codeTop + 1));
    blockstat = InBlock;
    body ();
    if ((token == closing) && streq (tokenString, "]"))
        (void)nextToken ();
    else
        compilError (selector, "block not terminated by ]", "");
    genInstruction (DoSpecial, StackReturn);
    codeArray[fixLocation] = codeTop + 1;
    temporaryTop = saveTemporary;
    blockstat = savebstat;
}

void temporaries (void)
{
    encPtr tempsym;

    temporaryTop = 0;
    if ((token == binary) && streq (tokenString, "|"))
    {
        (void)nextToken ();
        while (token == nameconst)
        {
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            if (temporaryTop > temporaryLimit)
                compilError (selector, "too many temporaries in method", "");
            else
            {
                tempsym = newSymbol (tokenString);
                temporaryName[temporaryTop] = addressOf (tempsym);
            }
            (void)nextToken ();
        }
        if ((token != binary) || !streq (tokenString, "|"))
            compilError (selector, "temporary list not terminated by bar", "");
        else
            (void)nextToken ();
    }
}

void messagePattern (void)
{
    encPtr argsym;

    argumentTop = 0;
    (void)strcpy (selector, tokenString);
    if (token == nameconst) /* unary message pattern */
        (void)nextToken ();
    else if (token == binary)
    { /* binary message pattern */
        (void)nextToken ();
        if (token != nameconst)
            compilError (selector,
                         "binary message pattern not followed by name",
                         selector);
        argsym = newSymbol (tokenString);
        argumentName[++argumentTop] = addressOf (argsym);
        (void)nextToken ();
    }
    else if (token == namecolon)
    { /* keyword message pattern */
        selector[0] = '\0';
        while (parseOk && (token == namecolon))
        {
            (void)strcat (selector, tokenString);
            (void)nextToken ();
            if (token != nameconst)
                compilError (selector,
                             "keyword message pattern",
                             "not followed by a name");
            if (++argumentTop > argumentLimit)
                compilError (selector, "too many arguments in method", "");
            argsym = newSymbol (tokenString);
            argumentName[argumentTop] = addressOf (argsym);
            (void)nextToken ();
        }
    }
    else
        compilError (selector, "illegal message selector", tokenString);
}

bool parse (encPtr method, char * text, bool saveText)
{
    int i;
    encPtr bytecodes, theLiterals;
    byte * bp;

    lexinit (text);
    parseOk = true;
    blockstat = NotInBlock;
    codeTop = 0;
    literalTop = temporaryTop = argumentTop = 0;
    maxTemporary = 0;

    messagePattern ();
    if (parseOk)
        temporaries ();
    if (parseOk)
        body ();
    if (parseOk)
    {
        genInstruction (DoSpecial, PopTop);
        genInstruction (DoSpecial, SelfReturn);
    }
    if (!parseOk)
    {
        orefOfPut (method, bytecodesInMethod, (objRef)nilObj);
    }
    else
    {
        bytecodes = newByteArray (codeTop);
        bp = addressOf (bytecodes);
        for (i = 0; i < codeTop; i++)
        {
            bp[i] = codeArray[i];
        }
        orefOfPut (method, messageInMethod, (objRef)newSymbol (selector));
        orefOfPut (method, bytecodesInMethod, (objRef)bytecodes);
        if (literalTop > 0)
        {
            theLiterals = newArray (literalTop);
            for (i = 1; i <= literalTop; i++)
            {
                orefOfPut (theLiterals, i, literalArray[i]);
            }
            orefOfPut (method, literalsInMethod, (objRef)theLiterals);
        }
        else
        {
            orefOfPut (method, literalsInMethod, (objRef)nilObj);
        }
        orefOfPut (method, stackSizeInMethod, (objRef)encValueOf (6));
        orefOfPut (method,
                   temporarySizeInMethod,
                   (objRef)encValueOf (1 + maxTemporary));
        if (saveText)
        {
            orefOfPut (method, textInMethod, (objRef)newString (text));
        }
        return (true);
    }
    return (false);
}

extern word traceVect[];

#define traceSize 3
#define execTrace traceVect[0]
#define primTrace traceVect[1]
#define mselTrace traceVect[2]

__INLINE__ objRef unsupportedPrim (objRef arg[])
{
    return ((objRef)nilObj);
}

/*
Prints the number of available object table entries.
Always fails.
Called from Scheduler>>initialize
*/
objRef primAvailCount (objRef arg[])
{
    fprintf (stderr, "free: %d\n", availCount ());
    return ((objRef)nilObj);
}

/*
Returns a pseudo-random integer.
Called from
  Random>>next
  Random>>randInteger:
*/
objRef primRandom (objRef arg[])
{
    short i;
    /* this is hacked because of the representation */
    /* of integers as shorts */
    i = rand () >> 8; /* strip off lower bits */
    if (i < 0)
        i = -i;
    return ((objRef)encValueOf (i >> 1));
}

extern bool watching;

/*
Inverts the state of a switch.  The switch controls, in part, whether or
not "watchWith:" messages are sent to Methods during execution.
Returns the Boolean representation of the switch value after the invert.
Called from Smalltalk>>watch
*/
objRef primFlipWatching (objRef arg[])
{
    watching = !watching;
    return ((objRef) (watching ? trueObj : falseObj));
}

/*
Terminates the interpreter.
Never returns.
Not called from the image.
*/
objRef primExit (objRef arg[])
{
    exit (0);
}

/*
Returns the class of which the receiver is an instance.
Called from Object>>class
*/
objRef primClass (objRef arg[])
{
    return ((objRef)getClass (arg[0]));
}

/*
Returns the field count of the von Neumann space of the receiver.
Called from Object>>basicSize
*/
objRef primSize (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        i = 0;
    else
        i = countOf (arg[0].ptr);
    return ((objRef)encValueOf (i));
}

/*
Returns a hashed representation of the receiver.
Called from Object>>hash
*/
objRef primHash (objRef arg[])
{
    if (isValue (arg[0]))
        return (arg[0]);
    else
        return ((objRef)encValueOf (oteIndexOf (arg[0].ptr)));
}

extern encPtr processStack;
extern int linkPointer;
int * counterAddress = NULL;

/*
Changes the active process stack if appropriate.  The change causes
control to be returned (eventually) to the context which sent the
message which created the context which invoked this primitive.
Returns true if the change was made; false if not.
Called from Context>>blockReturn
N.B.:  This involves some tricky code.  The compiler generates the
message which invokes Context>>blockReturn.  Context>>blockReturn is a
normal method.  It processes the true/false indicator.  Its result is
discarded when it returns, exposing the value to be returned from the
context which invokes this primitive.  Only then is the process stack
change effective.
*/
objRef primBlockReturn (objRef arg[])
{
    int i;
    int j;
    /* first get previous link pointer */
    i = intValueOf (orefOf (processStack, linkPointer).val);
    /* then creating context pointer */
    j = intValueOf (orefOf (arg[0].ptr, 1).val);
    if (ptrNe (orefOf (processStack, j + 1), arg[0]))
        return ((objRef)falseObj);
    /* first change link pointer to that of creator */
    orefOfPut (processStack, i, orefOf (processStack, j));
    /* then change return point to that of creator */
    orefOfPut (processStack, i + 2, orefOf (processStack, j + 2));
    return ((objRef)trueObj);
}

jmp_buf jb = {};

void brkfun (int sig)
{
    longjmp (jb, 1);
}

void brkignore (int sig)
{
}

bool execute (encPtr aProcess, int maxsteps);

/*
Executes the receiver until its time slice is ended or terminated.
Returns true in the former case; false in the latter.
Called from Process>>execute
*/
objRef primExecute (objRef arg[])
{
    encPtr saveProcessStack;
    int saveLinkPointer;
    int * saveCounterAddress;
    objRef returnedObject;
    /* first save the values we are about to clobber */
    saveProcessStack = processStack;
    saveLinkPointer = linkPointer;
    saveCounterAddress = counterAddress;
    /* trap control-C */
    signal (SIGINT, brkfun);
    if (setjmp (jb))
        returnedObject = (objRef)falseObj;
    else if (execute (arg[0].ptr, 1 << 12))
        returnedObject = (objRef)trueObj;
    else
        returnedObject = (objRef)falseObj;
    signal (SIGINT, brkignore);
    /* then restore previous environment */
    processStack = saveProcessStack;
    linkPointer = saveLinkPointer;
    counterAddress = saveCounterAddress;
    return (returnedObject);
}

/*
Returns true if the content of the receiver's objRef is equal to that
of the first argument's; false otherwise.
Called from Object>>==
*/
objRef primIdent (objRef arg[])
{
    if (ptrEq (arg[0], arg[1]))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Defines the receiver to be an instance of the first argument.
Returns the receiver.
Called from
  BlockNode>>newBlock
  ByteArray>>asString
  ByteArray>>size:
  Class>>new:
*/
objRef primClassOfPut (objRef arg[])
{
    classOfPut (arg[0].ptr, arg[1].ptr);
    return (arg[0]);
}

/*
Creates a new String.  The von Neumann space of the new String is that
of the receiver, up to the left-most null, followed by that of the first
argument, up to the left-most null, followed by a null.
Returns the new String.
Called from
  String>>,
  Symbol>>asString
*/
objRef primStringCat (objRef arg[])
{
    addr src1 = addressOf (arg[0].ptr);
    word len1 = strlen (src1);
    addr src2 = addressOf (arg[1].ptr);
    word len2 = strlen (src2);
    encPtr ans = allocByteObj (len1 + len2 + 1);
    addr tgt = addressOf (ans);
    (void)memcpy (tgt, src1, len1);
    (void)memcpy (((byte *)tgt) + len1, src2, len2);
    if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
        stringClass = globalValue ("String");
    classOfPut (ans, stringClass);
    return ((objRef)ans);
}

/*
Returns the objRef of the receiver denoted by the argument.
Called from Object>>basicAt:
*/
objRef primBasicAt (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        return ((objRef)nilObj);
    if (!isObjRefs (arg[0].ptr))
        return ((objRef)nilObj);
    if (isIndex (arg[1]))
        return ((objRef)nilObj);
    i = intValueOf (arg[1].val);
    if (i < 1 || i > countOf (arg[0].ptr))
        return ((objRef)nilObj);
    return (orefOf (arg[0].ptr, i));
}

/*
Returns an encoded representation of the byte of the receiver denoted by
the argument.
Called from ByteArray>>basicAt:
*/
objRef primByteAt (objRef arg[]) /*fix*/
{
    int i;
    if (isIndex (arg[1]))
        sysError ("non integer index", "byteAt:");
    i = byteOf (arg[0].ptr, intValueOf (arg[1].val));
    if (i < 0)
        i += 256;
    return ((objRef)encValueOf (i));
}

/*
Defines the global value of the receiver to be the first argument.
Returns the receiver.
Called from Symbol>>assign:
*/
objRef primSymbolAssign (objRef arg[]) /*fix*/
{
    nameTableInsert (
        symbols, strHash (addressOf (arg[0].ptr)), arg[0].ptr, arg[1].ptr);
    return (arg[0]);
}

/*
Changes the active process stack.  The change causes control to be
returned in the method containing the block controlled by the receiver
rather than the method which sent the message (e.g. Block>>value) which
created the context which invoked this primitive.  Execution will resume
at the location denoted by the first argument.
Called from Context>>returnToBlock:
N.B.:  The code involved here isn't quite as tricky as that involved
in primBlockReturn (q.v.).
*/
objRef primBlockCall (objRef arg[]) /*fix*/
{
    int i;
    /* first get previous link */
    i = intValueOf (orefOf (processStack, linkPointer).val);
    /* change context and byte pointer */
    orefOfPut (processStack, i + 1, arg[0]);
    orefOfPut (processStack, i + 4, arg[1]);
    return (arg[0]);
}

/*
Returns a modified copy of the receiver.  The receiver is a block.  The
modification defines the controlling context of the clone to be the
argument.  The argument is the current context and is the target of any
"^" return eventually invoked by the receiver.
This primitive is called by compiler-generated code.
N.B.:  The code involved here isn't quite as tricky as that involved
in primBlockReturn (q.v.).
*/
objRef primBlockClone (objRef arg[]) /*fix*/
{
    objRef returnedObject;
    returnedObject = (objRef)newBlock ();
    orefOfPut (returnedObject.ptr, 1, arg[1]);
    orefOfPut (returnedObject.ptr, 2, orefOf (arg[0].ptr, 2));
    orefOfPut (returnedObject.ptr, 3, orefOf (arg[0].ptr, 3));
    orefOfPut (returnedObject.ptr, 4, orefOf (arg[0].ptr, 4));
    return (returnedObject);
}

/*
Defines the objRef of the receiver denoted by the first argument to be
the second argument.
Returns the receiver.
Called from Object>>basicAt:put:
*/
objRef primBasicAtPut (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        return ((objRef)nilObj);
    if (!isObjRefs (arg[0].ptr))
        return ((objRef)nilObj);
    if (isIndex (arg[1]))
        return ((objRef)nilObj);
    i = intValueOf (arg[1].val);
    if (i < 1 || i > countOf (arg[0].ptr))
        return ((objRef)nilObj);
    orefOfPut (arg[0].ptr, i, arg[2]);
    return (arg[0]);
}

/*
Defines the byte of the receiver denoted by the first argument to be a
decoded representation of the second argument.
Returns the receiver.
Called from ByteArray>>basicAt:put:
*/
objRef primByteAtPut (objRef arg[]) /*fix*/
{
    if (isIndex (arg[1]))
        sysError ("non integer index", "byteAtPut");
    if (isIndex (arg[2]))
        sysError ("assigning non int", "to byte");
    byteOfPut (arg[0].ptr, intValueOf (arg[1].val), intValueOf (arg[2].val));
    return (arg[0]);
}

__INLINE__ word min (word one, word two)
{
    return (one <= two ? one : two);
}

/*
Creates a new String.  The von Neumann space of the new String is
usually that of a substring of the receiver, from the byte denoted by
the first argument through the byte denoted by the second argument,
followed by a null.  However, if the denoted substring is partially
outside the space of the receiver, only that portion within the space of
the receiver is used.  Also, if the denoted substring includes a null,
only that portion up to the left-most null is used.  Further, if the
denoted substring is entirely outside the space of the receiver or its
length is less than one, none of it is used.
Returns the new String.
Called from String>>copyFrom:to:
*/
objRef primCopyFromTo (objRef arg[]) /*fix*/
{
    if ((isIndex (arg[1])) || (isIndex (arg[2])))
        sysError ("non integer index", "copyFromTo");
    {
        addr src = addressOf (arg[0].ptr);
        word len = strlen (src);
        int pos1 = intValueOf (arg[1].val);
        int pos2 = intValueOf (arg[2].val);
        int req = pos2 + 1 - pos1;
        word act;
        encPtr ans;
        addr tgt;
        if (pos1 >= 1 && pos1 <= len && req >= 1)
            act = min (req, strlen (((byte *)src) + (pos1 - 1)));
        else
            act = 0;
        ans = allocByteObj (act + 1);
        tgt = addressOf (ans);
        (void)memcpy (tgt, ((byte *)src) + (pos1 - 1), act);
        if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
            stringClass = globalValue ("String");
        classOfPut (ans, stringClass);
        return ((objRef)ans);
    }
}

void flushCache (encPtr messageToSend, encPtr class);

/*
Kills the cache slot denoted by the receiver and argument.  The receiver
should be a message selector symbol.  The argument should be a class.
Returns the receiver.
Called from Class>>install:
*/
objRef primFlushCache (objRef arg[])
{
    if (isValue (arg[0]) || isValue (arg[1]))
        return ((objRef)nilObj);
    flushCache (arg[0].ptr, arg[1].ptr);
    return (arg[0]);
}

objRef primParse (objRef arg[]) /*del*/
{
    setInstanceVariables (arg[0].ptr);
    if (parse (arg[2].ptr, addressOf (arg[1].ptr), false))
    {
        flushCache (orefOf (arg[2].ptr, messageInMethod).ptr, arg[0].ptr);
        return ((objRef)trueObj);
    }
    else
        return ((objRef)falseObj);
}

/*
Returns the equivalent of the receiver's value in a floating-point
representation.
Called from Integer>>asFloat
*/
objRef primAsFloat (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)newFloat ((double)intValueOf (arg[0].val)));
}

/*
Defines a counter to be the argument's value.  When this counter is
less than 1, a Process time slice is finished.
Always fails.
Called from
  Scheduler>>critical:
  Scheduler>>yield
*/
objRef primSetTimeSlice (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    *counterAddress = intValueOf (arg[0].val);
    return ((objRef)nilObj);
}

/*
Sets the seed for a pseudo-random number generator.
Always fails.
Called from Random>>set:
*/
objRef primSetSeed (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    (void)srand ((unsigned)intValueOf (arg[0].val));
    return ((objRef)nilObj);
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of objRefs.  The number is denoted by the
receiver.
Called from
  BlockNode>>newBlock
  Class>>new:
*/
objRef primAllocOrefObj (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)allocOrefObj (intValueOf (arg[0].val)));
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of bytes.  The number is denoted by the
receiver.
Called from
  ByteArray>>size:
*/
objRef primAllocByteObj (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)allocByteObj (intValueOf (arg[0].val)));
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Integer>>+
Also called for SendBinary bytecodes.
*/
objRef primAdd (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult += intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Integer>>-
Also called for SendBinary bytecodes.
*/
objRef primSubtract (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult -= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Integer>><
Also called for SendBinary bytecodes.
*/
objRef primLessThan (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) < intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Called from Integer>>>
Also called for SendBinary bytecodes.
*/
objRef primGreaterThan (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) > intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primLessOrEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) <= intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primGreaterOrEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) >= intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called for SendBinary bytecodes.
*/
objRef primEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) == intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primNotEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) != intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Integer>>*
Also called for SendBinary bytecodes.
*/
objRef primMultiply (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult *= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the quotient of the result of dividing the receiver's value by
the argument's value.
Called from Integer>>quo:
Also called for SendBinary bytecodes.
*/
objRef primQuotient (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[1].val) == 0)
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult /= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the remainder of the result of dividing the receiver's value by
the argument's value.
Called for SendBinary bytecodes.
*/
objRef primRemainder (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[1].val) == 0)
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult %= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the bit-wise "and" of the receiver's value and the argument's
value.
Called from Integer>>bitAnd:
Also called for SendBinary bytecodes.
*/
objRef primBitAnd (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult &= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the bit-wise "exclusive or" of the receiver's value and the
argument's value.
Called from Integer>>bitXor:
Also called for SendBinary bytecodes.
*/
objRef primBitXor (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult ^= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the result of shifting the receiver's value a number of bit
positions denoted by the argument's value.  Positive arguments cause
left shifts.  Negative arguments cause right shifts.  Note that the
result is truncated to the range of embeddable values.
Called from Integer>>bitXor:
*/
objRef primBitShift (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    if (intValueOf (arg[1].val) < 0)
        longresult >>= -intValueOf (arg[1].val);
    else
        longresult <<= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the field count of the von Neumann space of the receiver up to
the left-most null.
Called from String>>size
*/
objRef primStringSize (objRef arg[])
{
    return ((objRef)encValueOf (strlen (addressOf (arg[0].ptr))));
}

/*
Returns a hashed representation of the von Neumann space of the receiver
up to the left-most null.
Called from
  String>>hash
  Symbol>>stringHash
*/
objRef primStringHash (objRef arg[])
{
    return ((objRef)encValueOf (strHash (addressOf (arg[0].ptr))));
}

/*
Returns a unique object.  Here, "unique" is determined by the
von Neumann space of the receiver up to the left-most null.  A copy will
either be found in or added to the global symbol table.  The returned
object will refer to the copy.
Called from String>>asSymbol
*/
objRef primAsSymbol (objRef arg[])
{
    return ((objRef)newSymbol (addressOf (arg[0].ptr)));
}

/*
Returns the object associated with the receiver in the global symbol
table.
Called from Symbol>>value
*/
objRef primGlobalValue (objRef arg[])
{
    return ((objRef)globalValue (addressOf (arg[0].ptr)));
}

/*
Passes the von Neumann space of the receiver to the host's "system"
function.  Returns what that function returns.
Called from String>>unixCommand
*/
objRef primHostCommand (objRef arg[])
{
    return ((objRef)encValueOf (system (addressOf (arg[0].ptr))));
}

/*
Returns the equivalent of the receiver's value in a printable character
representation.
Called from Float>>printString
*/
objRef primAsString (objRef arg[])
{
    char buffer[32];
    (void)sprintf (buffer, "%g", floatValue (arg[0].ptr));
    return ((objRef)newString (buffer));
}

/*
Returns the natural logarithm of the receiver's value.
Called from Float>>ln
*/
objRef primNaturalLog (objRef arg[])
{
    return ((objRef)newFloat (log (floatValue (arg[0].ptr))));
}

/*
Returns "e" raised to a power denoted by the receiver's value.
Called from Float>>exp
*/
objRef primERaisedTo (objRef arg[])
{
    return ((objRef)newFloat (exp (floatValue (arg[0].ptr))));
}

/*
Returns a new Array containing two integers n and m such that the
receiver's value can be expressed as n * 2**m.
Called from Float>>integerPart
*/
objRef primIntegerPart (objRef arg[])
{
    double temp;
    int i;
    int j;
    encPtr returnedObject = nilObj;
#define ndif 12
    temp = frexp (floatValue (arg[0].ptr), &i);
    if ((i >= 0) && (i <= ndif))
    {
        temp = ldexp (temp, i);
        i = 0;
    }
    else
    {
        i -= ndif;
        temp = ldexp (temp, ndif);
    }
    j = (int)temp;
    returnedObject = newArray (2);
    orefOfPut (returnedObject, 1, (objRef)encValueOf (j));
    orefOfPut (returnedObject, 2, (objRef)encValueOf (i));
#ifdef trynew
    /* if number is too big it can't be integer anyway */
    if (floatValue (arg[0].ptr) > 2e9)
        returnedObject = nilObj;
    else
    {
        (void)modf (floatValue (arg[0].ptr), &temp);
        ltemp = (long)temp;
        if (canEmbed (ltemp))
            returnedObject = encValueOf ((int)temp);
        else
            returnedObject = newFloat (temp);
    }
#endif
    return ((objRef)returnedObject);
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Float>>+
*/
objRef primFloatAdd (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result += floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Float>>-
*/
objRef primFloatSubtract (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result -= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Float>><
*/
objRef primFloatLessThan (objRef arg[])
{
    if (floatValue (arg[0].ptr) < floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Not called from the image.
*/
objRef primFloatGreaterThan (objRef arg[])
{
    if (floatValue (arg[0].ptr) > floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
objRef primFloatLessOrEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) <= floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
objRef primFloatGreaterOrEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) >= floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called from Float>>=
*/
objRef primFloatEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) == floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Not called from the image.
*/
objRef primFloatNotEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) != floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Float>>*
*/
objRef primFloatMultiply (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result *= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns the result of dividing the receiver's value by the argument's
value.
Called from Float>>/
*/
objRef primFloatDivide (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result /= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

#define MAXFILES 32

FILE * fp[MAXFILES] = {};

/*
Opens the file denoted by the first argument, if necessary.  Some of the
characteristics of the file and/or the operations permitted on it may be
denoted by the second argument.
Returns non-nil if successful; nil otherwise.
Called from File>>open
*/
objRef primFileOpen (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    char * p = addressOf (arg[1].ptr);
    if (streq (p, "stdin"))
        fp[i] = stdin;
    else if (streq (p, "stdout"))
        fp[i] = stdout;
    else if (streq (p, "stderr"))
        fp[i] = stderr;
    else
    {
        char * q = addressOf (arg[2].ptr);
        char * r = strchr (q, 'b');
        encPtr s = {false, 1};
        if (r == NULL)
        {
            int t = strlen (q);
            s = allocByteObj (t + 2);
            r = addressOf (s);
            memcpy (r, q, t);
            *(r + t) = 'b';
            q = r;
        }
        fp[i] = fopen (p, q);
        if (r == NULL)
            isVolatilePut (s, false);
    }
    if (fp[i] == NULL)
        return ((objRef)nilObj);
    else
        return ((objRef)encValueOf (i));
}

/*
Closes the file denoted by the receiver.
Always fails.
Called from File>>close
*/
objRef primFileClose (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        (void)fclose (fp[i]);
    fp[i] = NULL;
    return ((objRef)nilObj);
}

void coldFileIn (encVal tagRef);

/*
Applies the built-in "fileIn" function to the file denoted by the
receiver.
Always fails.
Not called from the image.
N.B.:  The built-in function uses the built-in compiler.  Both should be
used only in connection with building an initial image.
*/
objRef primFileIn (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        coldFileIn (arg[0].val);
    return ((objRef)nilObj);
}

/*
Reads the next line of characters from the file denoted by the receiver.
This line usually starts with the character at the current file position
and ends with the left-most newline.  However, if reading from standard
input, the line may be continued by immediately preceding the newline
with a backslash, both of which are deleted.  Creates a new String.  The
von Neumann space of the new String is usually the characters of the
complete line followed by a null.  However, if reading from standard
input, the trailing newline is deleted.  Also, if the line includes a
null, only that portion up to the left-most null is used.
Returns the new String if successful, nil otherwise.
Called from File>>getString
*/
objRef primGetString (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    int j;
    char buffer[4096];
    if (!fp[i])
        return ((objRef)nilObj);
    j = 0;
    buffer[j] = '\0';
    while (1)
    {
        if (fgets (&buffer[j], 512, fp[i]) == NULL)
        {
            if (fp[i] == stdin)
                (void)fputc ('\n', stdout);
            return ((objRef)nilObj); /* end of file */
        }
        if (fp[i] == stdin)
        {
            /* delete the newline */
            j = strlen (buffer);
            if (buffer[j - 1] == '\n')
                buffer[j - 1] = '\0';
        }
        j = strlen (buffer) - 1;
        if (buffer[j] != '\\')
            break;
        /* else we loop again */
    }
    return ((objRef)newString (buffer));
}

__INLINE__ bool irf (FILE * tag, addr dat, word len)
{
    return ((fread (dat, len, 1, tag) == 1) ? true : false);
}

encPtr imageRead (FILE * tag)
{
    encVal ver = encValueOf (3);
    encVal val;
    word ord;
    otbEnt * otp;
    ot2Ent * o2p;
    encPtr ptr;
    word len;
    if (irf (tag, &val, sizeof val) != true)
        goto fail;
    if (ptrNe ((objRef)val, (objRef)ver))
        goto fail;
    while (irf (tag, &val, sizeof val) == true)
    {
        ord = intValueOf (val);
        otp = &objTbl[ord];
#if 0
    if(irf(tag, (void*)otp, sizeof(addr)) != true)
      goto fail;
#endif
        if (irf (tag,
                 ((void *)otp) + sizeof (addr),
                 sizeof (otbEnt) - sizeof (addr)) != true)
            goto fail;
        o2p = &ob2Tbl[ord];
        if (irf (tag, o2p, sizeof (ot2Ent)) != true)
            goto fail;
        ptr = encIndexOf (ord);
        if ((len = spaceOf (ptr)))
        {
            addressOfPut (ptr, newStorage (len));
            if (irf (tag, addressOf (ptr), len) != true)
                goto fail;
        }
    }
    return (trueObj);
fail:
    return (falseObj);
}

__INLINE__ bool iwf (FILE * tag, addr dat, word len)
{
    return ((fwrite (dat, len, 1, tag) == 1) ? true : false);
}

encPtr imageWrite (FILE * tag)
{
    encVal val = encValueOf (3);
    word ord;
    encPtr ptr;
    otbEnt * otp;
    ot2Ent * o2p;
    word len;
    if (iwf (tag, &val, sizeof val) != true)
        goto fail;
    for (ord = otbLob; ord <= otbHib; ord++)
    {
        ptr = encIndexOf (ord);
        if (isAvail (ptr))
            continue;
        val = encValueOf (ord);
        if (iwf (tag, &val, sizeof val) != true)
            goto fail;
        otp = &objTbl[ord];
#if 0
    if(iwf(tag, (void*)otp, sizeof(addr)) != true)
      goto fail;
#endif
        if (iwf (tag,
                 ((void *)otp) + sizeof (addr),
                 sizeof (otbEnt) - sizeof (addr)) != true)
            goto fail;
        o2p = &ob2Tbl[ord];
        if (iwf (tag, o2p, sizeof (ot2Ent)) != true)
            goto fail;
        if ((len = spaceOf (ptr)))
            if (iwf (tag, addressOf (ptr), len) != true)
                goto fail;
    }
    return (trueObj);
fail:
    return (falseObj);
}

/*
Writes the currently running set of objects in binary form to the file
denoted by the receiver.
Returns true if successful; false or nil otherwise.
Called from File>>saveImage
*/
objRef primImageWrite (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        return ((objRef)imageWrite (fp[i]));
    else
        return ((objRef)nilObj);
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver.
Always fails.
Called from File>>printNoReturn:
*/
objRef primPrintWithoutNL (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (!fp[i])
        return ((objRef)nilObj);
    (void)fputs (addressOf (arg[1].ptr), fp[i]);
    (void)fflush (fp[i]);
    return ((objRef)nilObj);
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver and appends a newline.
Always fails.
Called from File>>print:
*/
objRef primPrintWithNL (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (!fp[i])
        return ((objRef)nilObj);
    (void)fputs (addressOf (arg[1].ptr), fp[i]);
    (void)fputc ('\n', fp[i]);
    return ((objRef)nilObj);
}

/*
Defines the trace vector slot denoted by the receiver to be the value
denoted by the argument.
Returns the receiver.
Not usually called from the image.
*/
objRef primSetTrace (objRef arg[])
{
    traceVect[intValueOf (arg[0].val)] = intValueOf (arg[1].val);
    return (arg[0]);
}

/*
Prints the von Neumann space of the receiver, followed by a newline, and
causes an abort.
Not usually called from the image.
*/
objRef primError (objRef arg[])
{
    (void)fprintf (stderr, "error: '%s'\n", (char *)addressOf (arg[0].ptr));
    assert (false);
    return (arg[0]);
}

/*
Causes memory reclamation.
Returns the receiver.
Not usually called from the image.
N.B.:  Do not call this primitive from the image with a receiver of
false.
*/
objRef primReclaim (objRef arg[])
{
    if (ptrEq (arg[0], (objRef)trueObj) || ptrEq (arg[0], (objRef)falseObj))
    {
        reclaim (ptrEq (arg[0], (objRef)trueObj));
        return (arg[0]);
    }
    else
        return ((objRef)nilObj);
}

FILE * logTag = NULL;
encPtr logBuf = {false, 1};
addr logPtr = 0;
word logSiz = 0;
word logPos = 0;

void logInit ()
{
    logPos = 0;
}

void logByte (byte val)
{
    if (logPos == logSiz)
    {
        encPtr newBuf = allocByteObj (logSiz + 128);
        addr newPtr = addressOf (newBuf);
        (void)memcpy (newPtr, logPtr, logSiz);
        isVolatilePut (logBuf, false);
        logBuf = newBuf;
        logPtr = newPtr;
        logSiz = countOf (logBuf);
    }
    *(((byte *)logPtr) + logPos++) = val;
}

bool logFini ()
{
    if (logTag == NULL)
        return (false);
    if (fwrite (logPtr, logPos, 1, logTag) != 1)
        return (false);
    if (fflush (logTag) == EOF)
        return (false);
    return (true);
}

/*
Writes the von Neumann space of the receiver, except for trailing nulls,
to the transcript in "chunk" form.  A chunk is usually a sequence of
non-'!' bytes followed by a '!' byte followed by a newline.  To
support '!' bytes within a chunk, such bytes are written as pairs of
'!' bytes.
Returns the receiver if successful; nil otherwise.
Called from ByteArray>>logChunk
*/
objRef primLogChunk (objRef arg[])
{
    logInit ();
    {
        encPtr txtBuf = arg[0].ptr;
        addr txtPtr = addressOf (txtBuf);
        word txtSiz = countOf (txtBuf);
        word txtPos = 0;
        while (txtSiz && *(((byte *)txtPtr) + (txtSiz - 1)) == '\0')
            txtSiz--;
        while (txtPos != txtSiz)
        {
            byte val = *(((byte *)txtPtr) + txtPos++);
            logByte (val);
            if (val == '!')
                logByte (val);
        }
    }
    logByte ('!');
    logByte ('\n');
    if (logFini () != true)
        return ((objRef)nilObj);
    return (arg[0]);
}

encPtr bwsBuf = {false, 1};
addr bwsPtr = 0;
word bwsSiz = 0;
word bwsPos = 0;

void bwsInit (void)
{
    bwsPos = 0;
}

void bwsNextPut (byte val)
{
    if (bwsPos == bwsSiz)
    {
        encPtr newBuf = allocByteObj (bwsSiz + 128);
        addr newPtr = addressOf (newBuf);
        (void)memcpy (newPtr, bwsPtr, bwsSiz);
        isVolatilePut (bwsBuf, false);
        bwsBuf = newBuf;
        bwsPtr = newPtr;
        bwsSiz = countOf (bwsBuf);
    }
    *(((byte *)bwsPtr) + bwsPos++) = val;
}

encPtr bwsFiniGet (void)
{
    encPtr ans = allocByteObj (bwsPos + 1);
    addr tgt = addressOf (ans);
    (void)memcpy (tgt, bwsPtr, bwsPos);
    if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
        stringClass = globalValue ("String");
    classOfPut (ans, stringClass);
    return (ans);
}

bool bwsFiniPut (FILE * tag)
{
    if (fwrite (bwsPtr, bwsPos, 1, tag) != 1)
        return (false);
    if (fflush (tag) == EOF)
        return (false);
    return (true);
}

/*
Reads the next chunk of characters from the file denoted by the
receiver.  A chunk is usually a sequence of non-'!' bytes followed by
a '!' byte followed by a newline.  To support '!' bytes within a
chunk, such bytes are read as pairs of '!' bytes.  Creates a new
String.  The von Neumann space of the new String is the bytes of the
chunk, not including the trailing '!' byte or newline, followed by a
null.
Returns the new String if successful, nil otherwise.
Called from File>>getChunk
*/
objRef primGetChunk (objRef arg[])
{
    int i;
    FILE * tag;
    int val;
    i = intValueOf (arg[0].val);
    if ((tag = fp[i]) == NULL)
        goto fail;
    bwsInit ();
    while ((val = fgetc (tag)) != EOF)
    {
        if (val == '!')
            switch ((val = fgetc (tag)))
            {
            case '\n':
                goto done;
            case '!':
                break;
            default:
                goto fail;
            }
        bwsNextPut (val);
    }
fail:
    return ((objRef)nilObj);
done:
    return ((objRef)bwsFiniGet ());
}

/*
Writes the von Neumann space of the argument, except for trailing nulls,
to the file denoted by the receiver in "chunk" form.  A chunk is usually
a sequence of non-'!' bytes followed by a '!' byte followed by a
newline.  To support '!' bytes within a chunk, such bytes are written
as pairs of '!' bytes.
Returns the receiver if successful; nil otherwise.
Called from File>>putChunk
*/
objRef primPutChunk (objRef arg[])
{
    int i;
    FILE * tag;
    i = intValueOf (arg[0].val);
    if ((tag = fp[i]) == NULL)
        goto fail;
    bwsInit ();
    {
        encPtr txtBuf = arg[1].ptr;
        addr txtPtr = addressOf (txtBuf);
        word txtSiz = countOf (txtBuf);
        word txtPos = 0;
        while (txtSiz && *(((byte *)txtPtr) + (txtSiz - 1)) == '\0')
            txtSiz--;
        while (txtPos != txtSiz)
        {
            byte val = *(((byte *)txtPtr) + txtPos++);
            bwsNextPut (val);
            if (val == '!')
                bwsNextPut (val);
        }
    }
    bwsNextPut ('!');
    bwsNextPut ('\n');
    if (bwsFiniPut (tag) == true)
        goto done;
fail:
    return ((objRef)nilObj);
done:
    return (arg[0]);
}

/* If built as part of L2, then the "special" primitive (#44) maps
 * straight back to the Pascal code. Otherwise, it returns nil to the
 * Smalltalk application.
 */
#ifdef L2_SMALLTALK_EMBEDDED
objRef L2_SMALLTALK_SPECIAL (objRef arguments[]);
#define primSpecial L2_SMALLTALK_SPECIAL
#else
objRef primSpecial (objRef arguments)
{
    return (objRef)nilObj;
}
#endif

typedef objRef primitiveMethod (objRef arg[]);

#define primVectLob 0
#define primVectHib 255
#define primVectDom ((primVectHib + 1) - primVectLob)

primitiveMethod * primitiveVector[primVectDom] = {
    /*000*/ &unsupportedPrim,
    /*001*/ &unsupportedPrim,
    /*002*/ &primAvailCount,
    /*003*/ &primRandom,
    /*004*/ &unsupportedPrim,
    /*005*/ &primFlipWatching,
    /*006*/ &unsupportedPrim,
    /*007*/ &unsupportedPrim,
    /*008*/ &unsupportedPrim,
    /*009*/ &primExit,
    /*010*/ &unsupportedPrim,
    /*011*/ &primClass,
    /*012*/ &primSize,
    /*013*/ &primHash,
    /*014*/ &unsupportedPrim,
    /*015*/ &unsupportedPrim,
    /*016*/ &unsupportedPrim,
    /*017*/ &unsupportedPrim,
    /*018*/ &primBlockReturn,
    /*019*/ &primExecute,
    /*020*/ &unsupportedPrim,
    /*021*/ &primIdent,
    /*022*/ &primClassOfPut,
    /*023*/ &unsupportedPrim,
    /*024*/ &primStringCat,
    /*025*/ &primBasicAt,
    /*026*/ &primByteAt,
    /*027*/ &primSymbolAssign,
    /*028*/ &primBlockCall,
    /*029*/ &primBlockClone,
    /*030*/ &unsupportedPrim,
    /*031*/ &primBasicAtPut,
    /*032*/ &primByteAtPut,
    /*033*/ &primCopyFromTo,
    /*034*/ &unsupportedPrim,
    /*035*/ &unsupportedPrim,
    /*036*/ &unsupportedPrim,
    /*037*/ &unsupportedPrim,
    /*038*/ &primFlushCache,
    /*039*/ &primParse,
    /*040*/ &unsupportedPrim,
    /*041*/ &unsupportedPrim,
    /*042*/ &unsupportedPrim,
    /*043*/ &unsupportedPrim,
    /*044*/ &primSpecial,
    /*045*/ &unsupportedPrim,
    /*046*/ &unsupportedPrim,
    /*047*/ &unsupportedPrim,
    /*048*/ &unsupportedPrim,
    /*049*/ &unsupportedPrim,
    /*050*/ &unsupportedPrim,
    /*051*/ &primAsFloat,
    /*052*/ &unsupportedPrim,
    /*053*/ &primSetTimeSlice,
    /*054*/ &unsupportedPrim,
    /*055*/ &primSetSeed,
    /*056*/ &unsupportedPrim,
    /*057*/ &unsupportedPrim,
    /*058*/ &primAllocOrefObj,
    /*059*/ &primAllocByteObj,
    /*060*/ &primAdd,
    /*061*/ &primSubtract,
    /*062*/ &primLessThan,
    /*063*/ &primGreaterThan,
    /*064*/ &primLessOrEqual,
    /*065*/ &primGreaterOrEqual,
    /*066*/ &primEqual,
    /*067*/ &primNotEqual,
    /*068*/ &primMultiply,
    /*069*/ &primQuotient,
    /*070*/ &primRemainder,
    /*071*/ &primBitAnd,
    /*072*/ &primBitXor,
    /*073*/ &unsupportedPrim,
    /*074*/ &unsupportedPrim,
    /*075*/ &unsupportedPrim,
    /*076*/ &unsupportedPrim,
    /*077*/ &unsupportedPrim,
    /*078*/ &unsupportedPrim,
    /*079*/ &primBitShift,
    /*080*/ &unsupportedPrim,
    /*081*/ &primStringSize,
    /*082*/ &primStringHash,
    /*083*/ &primAsSymbol,
    /*084*/ &unsupportedPrim,
    /*085*/ &unsupportedPrim,
    /*086*/ &unsupportedPrim,
    /*087*/ &primGlobalValue,
    /*088*/ &primHostCommand,
    /*089*/ &unsupportedPrim,
    /*090*/ &unsupportedPrim,
    /*091*/ &unsupportedPrim,
    /*092*/ &unsupportedPrim,
    /*093*/ &unsupportedPrim,
    /*094*/ &unsupportedPrim,
    /*095*/ &unsupportedPrim,
    /*096*/ &unsupportedPrim,
    /*097*/ &unsupportedPrim,
    /*098*/ &unsupportedPrim,
    /*099*/ &unsupportedPrim,
    /*100*/ &unsupportedPrim,
    /*101*/ &primAsString,
    /*102*/ &primNaturalLog,
    /*103*/ &primERaisedTo,
    /*104*/ &unsupportedPrim,
    /*105*/ &unsupportedPrim,
    /*106*/ &primIntegerPart,
    /*107*/ &unsupportedPrim,
    /*108*/ &unsupportedPrim,
    /*109*/ &unsupportedPrim,
    /*110*/ &primFloatAdd,
    /*111*/ &primFloatSubtract,
    /*112*/ &primFloatLessThan,
    /*113*/ &primFloatGreaterThan,
    /*114*/ &primFloatLessOrEqual,
    /*115*/ &primFloatGreaterOrEqual,
    /*116*/ &primFloatEqual,
    /*117*/ &primFloatNotEqual,
    /*118*/ &primFloatMultiply,
    /*119*/ &primFloatDivide,
    /*120*/ &primFileOpen,
    /*121*/ &primFileClose,
    /*122*/ &unsupportedPrim,
    /*123*/ &primFileIn,
    /*124*/ &unsupportedPrim,
    /*125*/ &primGetString,
    /*126*/ &unsupportedPrim,
    /*127*/ &primImageWrite,
    /*128*/ &primPrintWithoutNL,
    /*129*/ &primPrintWithNL,
    /*130*/ &unsupportedPrim,
    /*131*/ &unsupportedPrim,
    /*132*/ &unsupportedPrim,
    /*133*/ &unsupportedPrim,
    /*134*/ &unsupportedPrim,
    /*135*/ &unsupportedPrim,
    /*136*/ &unsupportedPrim,
    /*137*/ &unsupportedPrim,
    /*138*/ &unsupportedPrim,
    /*139*/ &unsupportedPrim,
    /*140*/ &unsupportedPrim,
    /*141*/ &unsupportedPrim,
    /*142*/ &unsupportedPrim,
    /*143*/ &unsupportedPrim,
    /*144*/ &unsupportedPrim,
    /*145*/ &unsupportedPrim,
    /*146*/ &unsupportedPrim,
    /*147*/ &unsupportedPrim,
    /*148*/ &unsupportedPrim,
    /*149*/ &unsupportedPrim,
    /*150*/ &unsupportedPrim,
    /*151*/ &primSetTrace,
    /*152*/ &primError,
    /*153*/ &primReclaim,
    /*154*/ &primLogChunk,
    /*155*/ &unsupportedPrim,
    /*156*/ &unsupportedPrim,
    /*157*/ &primGetChunk,
    /*158*/ &primPutChunk,
    /*159*/ &unsupportedPrim};

__INLINE__ objRef primitive (int primitiveNumber, objRef * arguments)
{
    if (primitiveNumber >= primVectLob && primitiveNumber <= primVectHib)
    {
        primitiveMethod * primMethPtr = primitiveVector[primitiveNumber];
        if (primMethPtr)
            return ((*primMethPtr) (arguments));
    }
    return (unsupportedPrim (arguments));
}

encPtr findClass (char * name)
{
    encPtr newobj;

    newobj = globalValue (name);
    if (ptrEq ((objRef)newobj, (objRef)nilObj))
        newobj = newClass (name);
    if (ptrEq (orefOf (newobj, sizeInClass), (objRef)nilObj))
    {
        orefOfPut (newobj, sizeInClass, (objRef)encValueOf (0));
    }
    return newobj;
}

void coldClassDef (encPtr strRef)
{
    encPtr superStr;
    encPtr classObj;
    int size;
    lexinit (addressOf (strRef));
    superStr = newString (tokenString);
    (void)nextToken ();
    (void)nextToken ();
    classObj = findClass (tokenString);
    if (streq (addressOf (superStr), "nil"))
        size = 0;
    else
    {
        encPtr superObj;
        superObj = findClass (addressOf (superStr));
        size = intValueOf (orefOf (superObj, sizeInClass).val);
        orefOfPut (classObj, superClassInClass, (objRef)superObj);
        {
            encPtr classMeta = classOf (classObj);
            encPtr superMeta = classOf (superObj);
            orefOfPut (classMeta, superClassInClass, (objRef)superMeta);
        }
    }
    (void)nextToken ();
    (void)nextToken ();
    if (*tokenString)
    {
        encPtr instStr;
        int instTop;
        encPtr instVars[256];
        encPtr varVec;
        int i;
        instStr = newString (tokenString);
        lexinit (addressOf (instStr));
        instTop = 0;
        while (*tokenString)
        {
            instVars[instTop++] = newSymbol (tokenString);
            size++;
            (void)nextToken ();
        }
        varVec = newArray (instTop);
        for (i = 0; i < instTop; i++)
            orefOfPut (varVec, i + 1, (objRef)instVars[i]);
        orefOfPut (classObj, variablesInClass, (objRef)varVec);
        isVolatilePut (instStr, false);
    }
    orefOfPut (classObj, sizeInClass, (objRef)encValueOf (size));
    isVolatilePut (superStr, false);
}

#define MethodTableSize 39

void coldMethods (encVal tagRef)
{
    encPtr strRef;
    encPtr classObj;
    encPtr methTable;
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    if (ptrEq ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
               (objRef)nilObj))
        return;
    if (streq (addressOf (strRef), "}"))
        return;
    lexinit (addressOf (strRef));
    classObj = findClass (tokenString);
    setInstanceVariables (classObj);
    /* now find or create a method table */
    methTable = orefOf (classObj, methodsInClass).ptr;
    if (ptrEq ((objRef)methTable, (objRef)nilObj))
    { /* must make */
        methTable = newDictionary (MethodTableSize);
        orefOfPut (classObj, methodsInClass, (objRef)methTable);
    }
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    while (ptrNe ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
                  (objRef)nilObj))
    {
        encPtr theMethod;
        encPtr selector;
        if (streq (addressOf (strRef), "}"))
            return;
        /* now we have a method */
        theMethod = newMethod ();
        if (parse (theMethod, addressOf (strRef), true))
        {
            orefOfPut (theMethod, methodClassInMethod, (objRef)classObj);
            selector = orefOf (theMethod, messageInMethod).ptr;
            nameTableInsert (
                methTable, oteIndexOf (selector), selector, theMethod);
        }
        else
        {
            /* get rid of unwanted method */
            isVolatilePut (theMethod, false);
        }
    }
}

void coldFileIn (encVal tagRef)
{
    encPtr strRef;
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    while (ptrNe ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
                  (objRef)nilObj))
    {
        if (streq (addressOf (strRef), "{"))
            coldMethods (tagRef);
        else
            coldClassDef (strRef);
    }
}

/*
The basic scheduling unit is a Process.  We keep a separate copy of its
reference.  This interpreter is explicitly stack-based.  We use a
separate stack for each Process and keep pointers to both its bottom and
top.  Information about particular activations of a Method can be
maintained in separate Context objects.  However, if a separate object
isn't needed, this information is kept within the process stack.  A
returned object replaces the receiver and arguments of the message which
produced it.  This occurs within the process stack at an offset called
the "return point".  We treat arguments and temporaries as if they were
stored in separate spaces.  However, they may actually be kept within
the process stack.  Though the receiver can be thought of as the
"zeroth" argument and accessed from the argument space, we keep separate
copies of its reference and a pointer to its instance variable space.
We also keep separate pointers to the literal and bytecode spaces of a
Method.  The "instruction pointer" is kept as an offset into the
bytecode space.  An explicit counter supports a rudimentary multi-
programming scheme.
*/
typedef struct
{
    encPtr pcso;   /* process object */
                   /*encPtr  pso;      process stack object */
    objRef * psb;  /* process stack base address */
    objRef * pst;  /* process stack top address */
    encPtr cxto;   /* context or process stack object */
    objRef * cxtb; /* context or process stack base address */
    int rtnp;      /* offset at which to store a returned object */
    objRef * argb; /* argument base address */
    objRef * tmpb; /* temporary base address */
    objRef rcvo;   /* receiver object */
    objRef * rcvb; /* receiver base address */
                   /*encPtr  lito;     literal object */
    objRef * litb; /* literal base address */
                   /*encPtr  byto;     bytecode object */
    byte * bytb;   /* bytecode base address - 1 */
    word byteOffset;
    int timeSliceCounter;
} execState;
#define processObject pcso
#define contextObject cxto
#define returnPoint rtnp
#define receiverObject rcvo

__INLINE__ objRef processStackAt (execState * es, int n)
{
    return (*(es->psb + (n - 1)));
}

__INLINE__ objRef stackTop (execState * es)
{
    return (*es->pst);
}

__INLINE__ void stackTopPut (execState * es, objRef x)
{
    *es->pst = x;
}

__INLINE__ void stackTopFree (execState * es)
{
    *es->pst-- = (objRef)nilObj;
}

__INLINE__ int stackInUse (execState * es)
{
    return ((es->pst + 1) - es->psb);
}

__INLINE__ void ipush (execState * es, objRef x)
{
    *++es->pst = x;
}

__INLINE__ objRef ipop (execState * es)
{
    objRef x = *es->pst;
    *es->pst-- = (objRef)nilObj;
    return (x);
}

__INLINE__ objRef argumentAt (execState * es, int n)
{
    return (*(es->argb + n));
}

__INLINE__ objRef temporaryAt (execState * es, int n)
{
    return (*(es->tmpb + n));
}

__INLINE__ void temporaryAtPut (execState * es, int n, objRef x)
{
    *(es->tmpb + n) = x;
}

__INLINE__ objRef receiverAt (execState * es, int n)
{
    return (*(es->rcvb + n));
}

__INLINE__ void receiverAtPut (execState * es, int n, objRef x)
{
    *(es->rcvb + n) = x;
}

__INLINE__ objRef literalAt (execState * es, int n)
{
    return (*(es->litb + n));
}

__INLINE__ byte nextByte (execState * es)
{
    return (*(es->bytb + es->byteOffset++));
}

bool unsupportedByte (execState * es, int low)
{
    sysError ("invalid bytecode", "");
    return (false);
}

/*
Pushes the value of one of the receiver's instance variables onto the
process stack.  The instruction operand denotes which one.
*/
bool bytePushInstance (execState * es, int low)
{
    ipush (es, receiverAt (es, low));
    return (true);
}

/*
Pushes the value of one of the message's argument variables onto the
process stack.  The instruction operand denotes which one.  Note that
the receiver is accessed as the "zeroth" argument.
*/
bool bytePushArgument (execState * es, int low)
{
    ipush (es, argumentAt (es, low));
    return (true);
}

/*
Pushes the value of one of the method's temporary variables onto the
process stack.  The instruction operand denotes which one.
*/
bool bytePushTemporary (execState * es, int low)
{
    ipush (es, temporaryAt (es, low));
    return (true);
}

/*
Pushes one of the method's literal values onto the process stack.  The
instruction operand denotes which one.  See also "bytePushConstant".
*/
bool bytePushLiteral (execState * es, int low)
{
    ipush (es, literalAt (es, low));
    return (true);
}

encPtr method = {true, 0};

encPtr copyFrom (encPtr obj, int start, int size)
{
    encPtr newObj;
    int i;

    newObj = newArray (size);
    for (i = 1; i <= size; i++)
    {
        orefOfPut (newObj, i, orefOf (obj, start));
        start++;
    }
    return newObj;
}

void fetchLinkageState (execState * es)
{
    es->contextObject = processStackAt (es, linkPointer + 1).ptr;
    es->returnPoint = intValueOf (processStackAt (es, linkPointer + 2).val);
    es->byteOffset = intValueOf (processStackAt (es, linkPointer + 4).val);
    if (ptrEq ((objRef)es->contextObject, (objRef)nilObj))
    {
        es->contextObject = processStack;
        es->cxtb = es->psb;
        es->argb = es->cxtb + (es->returnPoint - 1);
        method = processStackAt (es, linkPointer + 3).ptr;
        es->tmpb = es->cxtb + linkPointer + 4;
    }
    else
    { /* read from context object */
        es->cxtb = addressOf (es->contextObject);
        method = orefOf (es->contextObject, methodInContext).ptr;
        es->argb =
            addressOf (orefOf (es->contextObject, argumentsInContext).ptr);
        es->tmpb =
            addressOf (orefOf (es->contextObject, temporariesInContext).ptr);
    }
}

__INLINE__ void fetchReceiverState (execState * es)
{
    es->receiverObject = argumentAt (es, 0);
    if (isIndex (es->receiverObject))
    {
        assert (ptrNe (es->receiverObject, (objRef)pointerList));
        es->rcvb = addressOf (es->receiverObject.ptr);
    }
    else
        es->rcvb = (objRef *)0;
}

__INLINE__ void fetchMethodState (execState * es)
{
    es->litb = addressOf (orefOf (method, literalsInMethod).ptr);
    es->bytb = addressOf (orefOf (method, bytecodesInMethod).ptr) - 1;
}

/*
Pushes one of several "constant" value onto the process stack.  The
instruction operand denotes which one.  Note that a given context object
is not "constant" in that the values of its instance variables may
change.  However, the identity of a given context object is "constant"
in that it will not change.  See also "bytePushLiteral".
*/
bool bytePushConstant (execState * es, int low)
{
    switch (low)
    {
    case 0:
    case 1:
    case 2:
        ipush (es, (objRef)encValueOf (low));
        break;
    case minusOne:
        ipush (es, (objRef)encValueOf (-1));
        break;
    case contextConst:
        /* check to see if we have made a block context yet */
        if (ptrEq ((objRef)es->contextObject, (objRef)processStack))
        {
            /* not yet, do it now - first get real return point */
            es->returnPoint =
                intValueOf (processStackAt (es, linkPointer + 2).val);
            es->contextObject = newContext (
                linkPointer,
                method,
                copyFrom (processStack,
                          es->returnPoint,
                          linkPointer - es->returnPoint),
                copyFrom (
                    processStack, linkPointer + 5, methodTempSize (method)));
            orefOfPut (
                processStack, linkPointer + 1, (objRef)es->contextObject);
            ipush (es, (objRef)es->contextObject);
            /* save byte pointer then restore things properly */
            orefOfPut (processStack,
                       linkPointer + 4,
                       (objRef)encValueOf (es->byteOffset));
            fetchLinkageState (es);
            fetchReceiverState (es);
            fetchMethodState (es);
            break;
        }
        ipush (es, (objRef)es->contextObject);
        break;
    case nilConst:
        ipush (es, (objRef)nilObj);
        break;
    case trueConst:
        ipush (es, (objRef)trueObj);
        break;
    case falseConst:
        ipush (es, (objRef)falseObj);
        break;
    default:
        sysError ("unimplemented constant", "pushConstant");
        return (false);
    }
    return (true);
}

/*
Stores the value on the top of the process stack into of one of the
receiver's instance variables.  The instruction operand denotes which
one.  Note that this doesn't pop the value from the stack.
*/
bool byteAssignInstance (execState * es, int low)
{
    receiverAtPut (es, low, stackTop (es));
    return (true);
}

/*
Stores the value on the top of the process stack into of one of the
method's temporary variables.  The instruction operand denotes which
one.  Note that this doesn't pop the value from the stack.
*/
bool byteAssignTemporary (execState * es, int low)
{
    temporaryAtPut (es, low, stackTop (es));
    return (true);
}

/*
Computes the offset within the process stack at which a returned object
will replace the receiver and arguments of a message.
*/
bool byteMarkArguments (execState * es, int low)
{
    es->returnPoint = (stackInUse (es) - low) + 1;
    es->timeSliceCounter++; /* make sure we do send */
    return (true);
}

__INLINE__ encPtr firstLookupClass (execState * es)
{
    es->argb = es->psb + (es->returnPoint - 1);
    fetchReceiverState (es);
    return (getClass (es->receiverObject));
}

encPtr messageToSend = {true, 0};

int messTest (encPtr obj)
{
    return (ptrEq ((objRef)obj, (objRef)messageToSend));
}

bool findMethod (encPtr * methodClassLocation)
{
    encPtr methodTable, methodClass;

    method = nilObj;
    methodClass = *methodClassLocation;

    for (; ptrNe ((objRef)methodClass, (objRef)nilObj);
         methodClass = orefOf (methodClass, superClassInClass).ptr)
    {
        methodTable = orefOf (methodClass, methodsInClass).ptr;
        if (ptrEq ((objRef)methodTable, (objRef)nilObj))
        { /*fix*/
            methodTable = newDictionary (MethodTableSize);
            orefOfPut (methodClass, methodsInClass, (objRef)methodTable);
        }
        method =
            hashEachElement (methodTable, oteIndexOf (messageToSend), messTest);
        if (ptrNe ((objRef)method, (objRef)nilObj))
            break;
    }

    if (ptrEq ((objRef)method, (objRef)nilObj))
    { /* it wasn't found */
        methodClass = *methodClassLocation;
        return false;
    }
    *methodClassLocation = methodClass;
    return true;
}

#define cacheSize 211

struct
{
    encPtr cacheMessage; /* the message being requested */
    encPtr lookupClass;  /* the class of the receiver */
    encPtr cacheClass;   /* the class of the method */
    encPtr cacheMethod;  /* the method itself */
} methodCache[cacheSize] = {};

void flushCache (encPtr messageToSend, encPtr class)
{
    int i;
    for (i = 0; i != cacheSize; i++)
        if (ptrEq ((objRef)methodCache[i].cacheMessage, (objRef)messageToSend))
            methodCache[i].cacheMessage = nilObj;
}

bool lookupGivenSelector (execState * es, encPtr methodClass)
{
    int hash;
    int j;
    encPtr argarray;
    objRef returnedObject;
    if (mselTrace)
        fprintf (
            stderr, "%d: %s\n", mselTrace--, (char *)addressOf (messageToSend));
    /* look up method in cache */
    hash = (oteIndexOf (messageToSend) + oteIndexOf (methodClass)) % cacheSize;
    assert (hash >= 0 && hash < cacheSize);
    if (ptrEq ((objRef)methodCache[hash].cacheMessage, (objRef)messageToSend) &&
        ptrEq ((objRef)methodCache[hash].lookupClass, (objRef)methodClass))
    {
        method = methodCache[hash].cacheMethod;
        methodClass = methodCache[hash].cacheClass;
        assert (isAvail (method) == false);
    }
    else
    {
        methodCache[hash].lookupClass = methodClass;
        if (!findMethod (&methodClass))
        {
            /* not found, we invoke a smalltalk method */
            /* to recover */
            j = stackInUse (es) - es->returnPoint;
            argarray = newArray (j + 1);
            for (; j >= 0; j--)
            {
                returnedObject = ipop (es);
                orefOfPut (argarray, j + 1, returnedObject);
            }
            ipush (es, orefOf (argarray, 1)); /* push receiver back */
            ipush (es, (objRef)messageToSend);
            messageToSend = newSymbol ("message:notRecognizedWithArguments:");
            ipush (es, (objRef)argarray);
            /* try again - if fail really give up */
            if (!findMethod (&methodClass))
            {
                sysWarn ("can't find", "error recovery method");
                /* just quit */
                return false;
            }
        }
        methodCache[hash].cacheMessage = messageToSend;
        methodCache[hash].cacheMethod = method;
        methodCache[hash].cacheClass = methodClass;
    }
    return (true);
}

bool watching = 0;

bool lookupWatchSelector (execState * es)
{
    int j;
    encPtr argarray;
    objRef returnedObject;
    encPtr methodClass;
    if (watching && ptrNe (orefOf (method, watchInMethod), (objRef)nilObj))
    {
        /* being watched, we send to method itself */
        j = stackInUse (es) - es->returnPoint;
        argarray = newArray (j + 1);
        for (; j >= 0; j--)
        {
            returnedObject = ipop (es);
            orefOfPut (argarray, j + 1, returnedObject);
        }
        ipush (es, (objRef)method); /* push method */
        ipush (es, (objRef)argarray);
        messageToSend = newSymbol ("watchWith:");
        /* try again - if fail really give up */
        methodClass = classOf (method);
        if (!findMethod (&methodClass))
        {
            sysWarn ("can't find", "watch method");
            /* just quit */
            return false;
        }
    }
    return (true);
}

encPtr growProcessStack (int top, int toadd)
{
    int size, i;
    encPtr newStack;

    if (toadd < 128)
        toadd = 128;
    size = countOf (processStack) + toadd;
    newStack = newArray (size);
    for (i = 1; i <= top; i++)
    {
        orefOfPut (newStack, i, orefOf (processStack, i));
    }
    return newStack;
}

void pushStateAndEnter (execState * es)
{
    int i;
    int j;
    /* save the current byte pointer */
    orefOfPut (
        processStack, linkPointer + 4, (objRef)encValueOf (es->byteOffset));
    /* make sure we have enough room in current process */
    /* stack, if not make stack larger */
    i = 6 + methodTempSize (method) + methodStackSize (method);
    j = stackInUse (es);
    if ((j + i) > countOf (processStack))
    {
        processStack = growProcessStack (j, i);
        es->psb = addressOf (processStack);
        es->pst = (es->psb + j);
        orefOfPut (es->processObject, stackInProcess, (objRef)processStack);
    }
    es->byteOffset = 1;
    /* now make linkage area */
    /* position 0 : old linkage pointer */
    ipush (es, (objRef)encValueOf (linkPointer));
    linkPointer = stackInUse (es);
    /* position 1 : context obj (nil means stack) */
    ipush (es, (objRef)nilObj);
    es->contextObject = processStack;
    es->cxtb = es->psb;
    /* position 2 : return point */
    ipush (es, (objRef)encValueOf (es->returnPoint));
    es->argb = es->cxtb + (es->returnPoint - 1);
    /* position 3 : method */
    ipush (es, (objRef)method);
    /* position 4 : bytecode counter */
    ipush (es, (objRef)encValueOf (es->byteOffset));
    /* then make space for temporaries */
    es->tmpb = es->pst + 1;
    es->pst += methodTempSize (method);
    fetchMethodState (es);
#if 0
  /* break if we are too big and probably looping */
  if (countOf(processStack) > 4096)
    es->timeSliceCounter = 0;
#endif
}

__INLINE__ bool lookupAndEnter (execState * es, encPtr methodClass)
{
    if (!lookupGivenSelector (es, methodClass))
        return (false);
    if (!lookupWatchSelector (es))
        return (false);
    pushStateAndEnter (es);
    return (true);
}

/*
Looks for a Method corresponding to the combination of a prospective
receiver's class and a symbol denoting some desired behavior.  The
instruction operand denotes which symbol.  Changes the execution state
of the interpreter such that the next bytecode executed will be that of
the Method located, if possible, in an appropriate context.  See also
"byteSendUnary", "byteSendBinary" and "byteDoSpecial".
*/
bool byteSendMessage (execState * es, int low)
{
    encPtr methodClass;
    messageToSend = literalAt (es, low).ptr;
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Handles certain special cases of messages involving one object.  See
also "byteSendMessage", "byteSendBinary" and "byteDoSpecial".
*/
bool byteSendUnary (execState * es, int low)
{
    encPtr methodClass;
    /* do isNil and notNil as special cases, since */
    /* they are so common */
    if ((!watching) && (low >= 0 && low <= 1))
    {
        switch (low)
        {
        case 0: /* isNil */
            stackTopPut (es,
                         (objRef) (ptrEq (stackTop (es), (objRef)nilObj)
                                       ? trueObj
                                       : falseObj));
            return (true);
        case 1: /* notNil */
            stackTopPut (es,
                         (objRef) (ptrEq (stackTop (es), (objRef)nilObj)
                                       ? falseObj
                                       : trueObj));
            return (true);
        }
    }
    es->returnPoint = stackInUse (es);
    messageToSend = unSyms[low];
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Handles certain special cases of messages involving two objects.  See
also "byteSendMessage", "byteSendUnary" and "byteDoSpecial".
*/
bool byteSendBinary (execState * es, int low)
{
    objRef * primargs;
    objRef returnedObject;
    encPtr methodClass;
    /* optimized as long as arguments are int */
    /* and conversions are not necessary */
    /* and overflow does not occur */
    if ((!watching) && (low >= 0 && low <= 12))
    {
        if (primTrace)
            fprintf (stderr, "%d: <%d>\n", primTrace--, low + 60);
        primargs = es->pst - 1;
        returnedObject = primitive (low + 60, primargs);
        if (ptrNe (returnedObject, (objRef)nilObj))
        {
            /* pop arguments off stack , push on result */
            stackTopFree (es);
            stackTopPut (es, returnedObject);
            return (true);
        }
    }
    es->returnPoint = stackInUse (es) - 1;
    messageToSend = binSyms[low];
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Calls a routine to evoke some desired behavior which is not implemented
in the form of a Method.
*/
bool byteDoPrimitive (execState * es, int low)
{
    objRef * primargs;
    int i;
    objRef returnedObject;
    /* low gives number of arguments */
    /* next byte is primitive number */
    primargs = (es->pst - low) + 1;
    /* next byte gives primitive number */
    i = nextByte (es);
    if (primTrace)
        fprintf (stderr, "%d: <%d>\n", primTrace--, i);
    returnedObject = primitive (i, primargs);
    /* pop off arguments */
    while (low-- > 0)
    {
        if (isIndex (stackTop (es)))
            isVolatilePut (stackTop (es).ptr, false);
        stackTopFree (es);
    }
    ipush (es, returnedObject);
    return (true);
}

bool leaveAndAnswer (execState * es, objRef returnedObject)
{
    es->returnPoint = intValueOf (orefOf (processStack, linkPointer + 2).val);
    linkPointer = intValueOf (orefOf (processStack, linkPointer).val);
    while (stackInUse (es) >= es->returnPoint)
    {
        if (isIndex (stackTop (es)))
            isVolatilePut (stackTop (es).ptr, false);
        stackTopFree (es);
    }
    ipush (es, returnedObject);
    /* now go restart old routine */
    if (linkPointer)
    {
        fetchLinkageState (es);
        fetchReceiverState (es);
        fetchMethodState (es);
        return (true);
    }
    else
        return (false); /* all done */
}

/*
Handles operations which aren't handled in other ways.  The instruction
operand denotes which operation.  Returning objects changes the
execution state of the interpreter such that the next bytecode executed
will be that of the Method which is to process the returned object, if
possible, in an appropriate context.  See also "byteSendMessage"
"byteSendUnary" and "byteSendBinary".  Various facilities such as
cascaded messages and optimized control structures involve tinkering
with the top of the process stack and the "instruction counter".
Sending messages to "super" changes the first class to be searched for a
Method from that of the prospective receiver to the superclass of that
in which the executing Method is located, if possible.
*/
bool byteDoSpecial (execState * es, int low)
{
    objRef returnedObject;
    int i;
    encPtr methodClass;
    switch (low)
    {
    case SelfReturn:
        returnedObject = argumentAt (es, 0);
        return (leaveAndAnswer (es, returnedObject));
    case StackReturn:
        returnedObject = ipop (es);
        return (leaveAndAnswer (es, returnedObject));
    case Duplicate:
        /* avoid possible subtle bug */
        returnedObject = stackTop (es);
        ipush (es, returnedObject);
        return (true);
    case PopTop:
        returnedObject = ipop (es);
        if (isIndex (returnedObject))
            isVolatilePut (returnedObject.ptr, false);
        return (true);
    case Branch:
        /* avoid a subtle bug here */
        i = nextByte (es);
        es->byteOffset = i;
        return (true);
    case BranchIfTrue:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)trueObj))
        {
            /* leave nil on stack */
            es->pst++;
            es->byteOffset = i;
        }
        return (true);
    case BranchIfFalse:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)falseObj))
        {
            /* leave nil on stack */
            es->pst++;
            es->byteOffset = i;
        }
        return (true);
    case AndBranch:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)falseObj))
        {
            ipush (es, returnedObject);
            es->byteOffset = i;
        }
        return (true);
    case OrBranch:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)trueObj))
        {
            ipush (es, returnedObject);
            es->byteOffset = i;
        }
        return (true);
    case SendToSuper:
        i = nextByte (es);
        messageToSend = literalAt (es, i).ptr;
        (void)firstLookupClass (es); /* fix? */
        methodClass = orefOf (method, methodClassInMethod).ptr;
        /* if there is a superclass, use it
           otherwise for class Object (the only
           class that doesn't have a superclass) use
           the class again */
        returnedObject = orefOf (methodClass, superClassInClass);
        if (ptrNe (returnedObject, (objRef)nilObj))
            methodClass = returnedObject.ptr;
        return (lookupAndEnter (es, methodClass));
    default:
        sysError ("invalid doSpecial", "");
        return (false);
    }
}

typedef bool bytecodeMethod (execState * es, int low);

#define byteVectLob 0
#define byteVectHib 15
#define byteVectDom ((byteVectHib + 1) - byteVectLob)

bytecodeMethod * bytecodeVector[byteVectDom] = {
    /*00*/ &unsupportedByte,
    /*01*/ &bytePushInstance,
    /*02*/ &bytePushArgument,
    /*03*/ &bytePushTemporary,
    /*04*/ &bytePushLiteral,
    /*05*/ &bytePushConstant,
    /*06*/ &byteAssignInstance,
    /*07*/ &byteAssignTemporary,
    /*08*/ &byteMarkArguments,
    /*09*/ &byteSendMessage,
    /*10*/ &byteSendUnary,
    /*11*/ &byteSendBinary,
    /*12*/ &unsupportedByte,
    /*13*/ &byteDoPrimitive,
    /*14*/ &unsupportedByte,
    /*15*/ &byteDoSpecial};

encPtr processStack = {true, 0};

int linkPointer = 0;

void fetchProcessState (execState * es)
{
    int j;
    processStack = orefOf (es->processObject, stackInProcess).ptr;
    es->psb = addressOf (processStack);
    j = intValueOf (orefOf (es->processObject, stackTopInProcess).val);
    es->pst = es->psb + (j - 1);
    linkPointer = intValueOf (orefOf (es->processObject, linkPtrInProcess).val);
}

void storeProcessState (execState * es)
{
    orefOfPut (es->processObject, stackInProcess, (objRef)processStack);
    orefOfPut (es->processObject,
               stackTopInProcess,
               (objRef)encValueOf (stackInUse (es)));
    orefOfPut (
        es->processObject, linkPtrInProcess, (objRef)encValueOf (linkPointer));
}

word traceVect[traceSize] = {};

bool execute (encPtr aProcess, int maxsteps)
{
    execState es = {};

    es.processObject = aProcess;
    es.timeSliceCounter = maxsteps;
    counterAddress = &es.timeSliceCounter;

    fetchProcessState (&es);
    fetchLinkageState (&es);
    fetchReceiverState (&es);
    fetchMethodState (&es);

    while (--es.timeSliceCounter > 0)
    {
        int low;
        int high;
        low = (high = nextByte (&es)) & 0x0F;
        high >>= 4;
        if (high == 0)
        {
            high = low;
            low = nextByte (&es);
        }
        if (execTrace)
            fprintf (stderr, "%d: %d %d\n", execTrace--, high, low);
        if (high >= byteVectLob && high <= byteVectHib)
        {
            bytecodeMethod * byteMethPtr = bytecodeVector[high];
            if (byteMethPtr)
            {
                if (!(*byteMethPtr) (&es, low))
                    return (false);
                continue;
            }
        }
        if (!unsupportedByte (&es, low))
            return (false);
    }

    orefOfPut (
        processStack, linkPointer + 4, (objRef)encValueOf (es.byteOffset));
    storeProcessState (&es);

    return (true);
}

void makeInitialImage (void)
{
    encPtr hashTable;
    encPtr symbolObj;
    encPtr symbolClass; /*shadows global for a reason*/
    encPtr metaclassClass;
    encPtr linkClass;

    nilObj = allocOrefObj (0);
    assert (oteIndexOf (nilObj) == 1);

    trueObj = allocOrefObj (0);
    assert (oteIndexOf (trueObj) == 2);
    falseObj = allocOrefObj (0);
    assert (oteIndexOf (falseObj) == 3);

    /* create the symbol table */

    hashTable = allocOrefObj (3 * 53);
    assert (oteIndexOf (hashTable) == 4);
    symbols = allocOrefObj (1);
    assert (oteIndexOf (symbols) == 5);
    orefOfPut (symbols, 1, (objRef)hashTable);

    /* create #Symbol, Symbol[Meta] and Metaclass[Meta] */

    symbolObj = newSymbol ("Symbol");
#if 0
  assert(ptrEq(classOf(symbolObj),nilObj));
  assert(ptrEq(globalValue("Symbol"),nilObj));
#endif
    symbolClass = newClass ("Symbol");
#if 0
  assert(ptrNe(classOf(symbolClass),nilObj));
  assert(ptrEq(classOf(classOf(symbolClass)),nilObj));
  assert(ptrEq(globalValue("Symbol"),symbolClass));
#endif
    classOfPut (symbolObj, symbolClass);
    classOfPut (newSymbol ("SymbolMeta"), symbolClass);
    metaclassClass = newClass ("Metaclass");
#if 0
  assert(ptrNe(classOf(metaclassClass),nilObj));
  assert(ptrEq(classOf(classOf(metaclassClass)),nilObj));
  assert(ptrEq(globalValue("Metaclass"),metaclassClass));
#endif
    classOfPut (classOf (symbolClass), metaclassClass);
    classOfPut (classOf (metaclassClass), metaclassClass);

    /* patch the class fields of nil, true and false */
    /* set their global values */

    classOfPut (nilObj, newClass ("UndefinedObject"));
    nameTableInsert (symbols, strHash ("nil"), newSymbol ("nil"), nilObj);
    classOfPut (trueObj, newClass ("True"));
    nameTableInsert (symbols, strHash ("true"), newSymbol ("true"), trueObj);
    classOfPut (falseObj, newClass ("False"));
    nameTableInsert (symbols, strHash ("false"), newSymbol ("false"), falseObj);

    /* patch the class fields of the symbol table links */
    /* make the symbol table refer to itself */ /*fix?*/

    linkClass = newClass ("Link");
    {
        word ord = 0;
        word hib = countOf (hashTable);
        for (; ord != hib; ord += 3)
        {
            encPtr link = orefOf (hashTable, ord + 3).ptr;
            while (ptrNe ((objRef)link, (objRef)nilObj))
            {
                if (ptrEq ((objRef)classOf (link), (objRef)nilObj))
                    classOfPut (link, linkClass);
                else
                    assert (ptrEq ((objRef)classOf (link), (objRef)linkClass));
                link = orefOf (link, 3).ptr;
            }
        }
    }
    classOfPut (hashTable, newClass ("Array"));
    classOfPut (symbols, newClass ("SymbolTable"));
    nameTableInsert (
        symbols, strHash ("symbols"), newSymbol ("symbols"), symbols);

    /* graft a skeleton metaclass tree to a skeleton class tree */
    {
        encPtr objectInst = newClass ("Object");
        encPtr classInst = newClass ("Class");
        orefOfPut (classOf (objectInst), superClassInClass, (objRef)classInst);
    }

    /* create other skeleton classes */

    /*(void) newClass("Array");*/
    (void)newClass ("Block");
    (void)newClass ("ByteArray");
    (void)newClass ("Char");
    (void)newClass ("Context");
    (void)newClass ("Dictionary");
    (void)newClass ("Float");
    /*(void) newClass("Link");*/
    /*(void) newClass("Metaclass");*/
    (void)newClass ("Method");
    (void)newClass ("String");
    /*(void) newClass("Symbol");*/
}

void goDoIt (char * text)
{
    encPtr method;
    encPtr process;
    encPtr stack;

    method = newMethod ();
    setInstanceVariables (nilObj);
    (void)parse (method, text, false);

    process = allocOrefObj (processSize);
    stack = allocOrefObj (50);

    /* make a process */
    orefOfPut (process, stackInProcess, (objRef)stack);
    orefOfPut (process, stackTopInProcess, (objRef)encValueOf (10));
    orefOfPut (process, linkPtrInProcess, (objRef)encValueOf (2));

    /* put argument on stack */
    orefOfPut (stack, 1, (objRef)nilObj); /* argument */
    /* now make a linkage area in stack */
    orefOfPut (stack, 2, (objRef)encValueOf (0)); /* previous link */
    orefOfPut (stack, 3, (objRef)nilObj); /* context object (nil => stack) */
    orefOfPut (stack, 4, (objRef)encValueOf (1)); /* return point */
    orefOfPut (stack, 5, (objRef)method);         /* method */
    orefOfPut (stack, 6, (objRef)encValueOf (1)); /* byte offset */

    /* now go execute it */
    while (execute (process, 1 << 14))
        fprintf (stderr, ".");

    /* get rid of unwanted process */
    isVolatilePut (process, false);
}

int main_1 (int argc, char * argv[])
{
    char methbuf[4096];
    int i;

    sysWarn ("\nPublic Domain Smalltalk", "");

    coldObjectTable ();

    makeInitialImage ();

    initCommonSymbols ();

    for (i = 1; i < argc; i++)
    {
        fputs ("Processing ", stderr);
        fputs (argv[i], stderr);
        fputs ("...\n", stderr);
        // fprintf(stderr, "%s:\n", argv[i]);
        //(void) sprintf(methbuf,
        //	   "goDoIt <120 1 '%s' 'r'>. <123 1>. <121 1>",
        //	   argv[i]);
        methbuf[0] = 0;
        strcat (methbuf, "goDoIt <120 1 '");
        strcat (methbuf, argv[i]);
        strcat (methbuf, "' 'r'>. <123 1>. <121 1>");
        goDoIt (methbuf);
    }

    /* when we are all done looking at the arguments, do initialization */
    fprintf (stderr, "initialization\n");
#if 0
  execTrace = 16;
  primTrace = 16;
  mselTrace = 16;
#endif
    goDoIt ("goDoIt nil initialize\n");
    fprintf (stderr, "finished\n");

    return 0;
}

int main_2 (int argc, char * argv[])
{
    FILE * fp;
    encPtr firstProcess;
    char *p, buffer[4096];

    sysWarn ("\nPublic Domain Smalltalk", "");

    warmObjectTableOne ();

    strcpy (buffer, "systemImage");
    p = buffer;
    if (argc != 1)
        p = argv[1];

    fp = fopen (p, "rb");
    if (fp == NULL)
    {
        sysError ("cannot open image", p);
        return (1);
    }

    if (ptrNe ((objRef)imageRead (fp), (objRef)trueObj))
    {
        sysError ("cannot read image", p);
        return (1);
    }

    (void)fclose (fp);

    warmObjectTableTwo ();

    initCommonSymbols ();

    firstProcess = globalValue ("systemProcess");
    if (ptrEq ((objRef)firstProcess, (objRef)nilObj))
    {
        sysError ("no initial process", "in image");
        return (1);
    }

    /* execute the main system process loop repeatedly */

    while (execute (firstProcess, 1 << 14))
        ;

    return 0;
}

void compilError (char * selector, char * str1, char * str2)
{
#ifdef L2_SMALLTALK_EMBEDDED
    fputs ("Compiler error:\n\tMethod:\t", stderr);
    fputs (selector, stderr);
    fputs ("\n\tStr1:\t", stderr);
    fputs (str1, stderr);
    fputs ("\n\tStr2:\t", stderr);
    fputs (str2, stderr);
    fputs ("\n", stderr);
#else
    (void)fprintf (
        stderr, "compiler error: Method %s : %s %s\n", selector, str1, str2);
#endif
    parseOk = false;
}

void compilWarn (char * selector, char * str1, char * str2)
{
    (void)fprintf (
        stderr, "compiler warning: Method %s : %s %s\n", selector, str1, str2);
}

void sysError (char * s1, char * s2)
{
#ifdef L2_SMALLTALK_EMBEDDED
    fputs (s1, stderr);
    fputs ("\n", stderr);
    fputs (s2, stderr);
    fputs ("\n", stderr);
#else
    (void)fprintf (stderr, "%s\n%s\n", s1, s2);
#endif
    (void)abort ();
}

void sysWarn (char * s1, char * s2)
{
#ifdef L2_SMALLTALK_EMBEDDED
    fputs (s1, stderr);
    fputs ("\n", stderr);
    fputs (s2, stderr);
    fputs ("\n", stderr);
#else
    (void)fprintf (stderr, "%s\n%s\n", s1, s2);
#endif
}

#ifdef L2_SMALLTALK_EMBEDDED
int L2_SMALLTALK_MAIN (int argc, char * argv[])
#else
int main (int argc, char * argv[])
#endif
{
    // printf("sizeof(otbEnt) = %d, sizeof(ot2Ent) = %d\n", sizeof(otbEnt),
    // sizeof(ot2Ent));
    int ans = 1;
    logTag = fopen ("transcript", "ab");
    if (argc > 1 && streq (argv[1], "-c"))
    {
        argv[1] = argv[0];
        argc--;
        argv++;
        ans = main_1 (argc, argv);
    }
    if (argc > 1 && streq (argv[1], "-w"))
    {
        argv[1] = argv[0];
        argc--;
        argv++;
        ans = main_2 (argc, argv);
    }
#if 0
  fprintf(stderr,"%s?\n",
    (char*)addressOf(orefOf(encIndexOf(100),nameInClass).ptr));
#endif
    if (ans == 0)
    {
        FILE * tag;
        tag = fopen ("snapshot", "wb");
        if (tag != NULL)
        {
            reclaim (false);
            if (ptrNe ((objRef)imageWrite (tag), (objRef)trueObj))
                ans = 2;
            (void)fclose (tag);
        }
        else
            ans = 2;
    }
    if (logTag != NULL)
        (void)fclose (logTag);
    return (ans);
}
