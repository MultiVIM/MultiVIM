#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "VMMemory.hxx"

#define streq(a, b) (strcmp (a, b) == 0)

otbEnt * objTbl = NULL;
ot2Ent * ob2Tbl = NULL;

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

void freeStorage (HostMemoryAddress x)
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
                objRef * f = (objRef *)vonNeumannSpaceOf (x.ptr);
                objRef * p =
                    (objRef *)((char *)f + vonNeumannSpaceLengthOf (x.ptr));
                while (p != f)
                    visit (*--p);
            }
        }
    }
}

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
        if (vonNeumannSpaceLengthOf (ptr))
        {
            freeStorage (vonNeumannSpaceOf (ptr));
            vonNeumannSpaceOfPut (ptr, 0);
            vonNeumannSpaceLengthOfPut (ptr, 0);
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

HostMemoryAddress newStorage (word bytes)
{
    HostMemoryAddress ans;
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
    objTbl = (otbEnt *)calloc (otbDom, sizeof (otbEnt));
    assert (objTbl != NULL);
    ob2Tbl = (ot2Ent *)calloc (otbDom, sizeof (ot2Ent));
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
    objTbl = (otbEnt *)calloc (otbDom, sizeof (otbEnt));
    assert (objTbl != NULL);
    ob2Tbl = (ot2Ent *)calloc (otbDom, sizeof (ot2Ent));
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

encPtr allocOrefObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 2; /*fix*/
    HostMemoryAddress mem = newStorage (num);
    vonNeumannSpaceOfPut (ptr, mem);
    scaleOfPut (ptr, 2); /*fix*/
    isObjRefsPut (ptr, true);
    vonNeumannSpaceLengthOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    while (n--)
    {
        /* Fix from Zak - modern compilers don't like assign-to-cast. */
        encPtr * tmpmem = (encPtr *)mem;
        *tmpmem++ = nilObj;
        mem = (HostMemoryAddress)tmpmem;
    }
    return (ptr);
}

encPtr allocByteObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 0; /*fix*/
    HostMemoryAddress mem = newStorage (num);
    vonNeumannSpaceOfPut (ptr, mem);
    scaleOfPut (ptr, 0); /*fix*/
    isObjRefsPut (ptr, false);
    vonNeumannSpaceLengthOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocHWrdObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 1; /*fix*/
    HostMemoryAddress mem = newStorage (num);
    vonNeumannSpaceOfPut (ptr, mem);
    scaleOfPut (ptr, 1); /*fix*/
    isObjRefsPut (ptr, false);
    vonNeumannSpaceLengthOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocWordObj (word n)
{
    encPtr ptr = newPointer ();
    word num = n << 2; /*fix*/
    HostMemoryAddress mem = newStorage (num);
    vonNeumannSpaceOfPut (ptr, mem);
    scaleOfPut (ptr, 2); /*fix*/
    isObjRefsPut (ptr, false);
    vonNeumannSpaceLengthOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    return (ptr);
}

encPtr allocZStrObj (const char * zstr)
{
    encPtr ptr = newPointer ();
    word num = strlen (zstr) + 1;
    HostMemoryAddress mem = newStorage (num);
    vonNeumannSpaceOfPut (ptr, mem);
    scaleOfPut (ptr, 0); /*fix*/
    isObjRefsPut (ptr, false);
    vonNeumannSpaceLengthOfPut (ptr, num);
    classOfPut (ptr, nilObj);
    (void)strcpy ((char *)vonNeumannSpaceOf (ptr), zstr);
    return (ptr);
}

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
        hp = (encPtr *)vonNeumannSpaceOf (table) + (hash - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if (ptrNe ((objRef)key, (objRef)nilObj) && (*fun) (key))
            return value;
        for (link = *hp; ptrNe ((objRef)link, (objRef)nilObj); link = *hp)
        {
            hp = (encPtr *)vonNeumannSpaceOf (link);
            key = *hp++;   /* link at: 1 */
            value = *hp++; /* link at: 2 */
            if (ptrNe ((objRef)key, (objRef)nilObj) && (*fun) (key))
                return value;
        }
    }
    return nilObj;
}

int strHash (const char * str)
{
    int hash;
    const char * p;

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

const char * charBuffer = 0;
encPtr objBuffer = {true, 0};

int strTest (encPtr key)
{
    if (vonNeumannSpaceOf (key) &&
        streq ((char *)vonNeumannSpaceOf (key), charBuffer))
    {
        objBuffer = key;
        return 1;
    }
    return 0;
}

encPtr globalKey (const char * str)
{
    charBuffer = str;
    objBuffer = nilObj;
    (void)hashEachElement (symbols, strHash (str), strTest);
    return objBuffer;
}

encPtr nameTableLookup (encPtr dict, const char * str)
{
    charBuffer = str;
    return hashEachElement (dict, strHash (str), strTest);
}

const char * unStrs[] = {"isNil",
                         "notNil",
                         "value",
                         "new",
                         "class",
                         "size",
                         "basicSize",
                         "print",
                         "printString",
                         0};

const char * binStrs[] = {"+",
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

    (void)memcpy (&d, vonNeumannSpaceOf (o), sizeof (double));
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

encPtr newClass (const char * name)
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

    metaName = (char *)newStorage (strlen (name) + 4 + 1);
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
    (void)memcpy (vonNeumannSpaceOf (newObj), &d, sizeof (double));
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

encPtr newSymbol (const char * str)
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