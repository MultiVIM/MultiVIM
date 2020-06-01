#include <algorithm>
#include <iostream>

#include "Lowlevel/MVPrinting.hxx"
#include "Oops.hxx"

extern "C"
{
    void * GC_find_header (void *);
    void * GC_base (void *);
}

NativeCodeOop NativeCodeOopDesc::newWithSize (size_t codeSize)
{
    NativeCodeOop oop = memMgr.allocateOopObj (1)->asNativeCodeOop ();
    oop.setIsa (memMgr.clsNativeCode);
    oop->setData (ByteArrayOopDesc::newWithSize (sizeof (Fun) + codeSize));
    return oop;
}

void LinkOopDesc::print (int in)
{
    if (!this)
    {
        std::cout << blanks (in) << "<nil>"
                  << "\n";
        return;
    }
    std::cout << blanks (in) << "Link " << this << "{\n";
    std::cout << blanks (in) << "One:\n";
    one ()->print (in + 2);
    std::cout << blanks (in) << "Two:\n";
    two ()->print (in + 2);
    std::cout << blanks (in) << "nextLink:\n";
    nextLink ()->print (in + 2);
    std::cout << blanks (in) << "}\n";
}

template <typename ExtraType>
std::pair<Oop, Oop>
DictionaryOopDesc::findPairByFun (intptr_t hash, ExtraType extraVal,
                                  int (*fun) (Oop, ExtraType))
{
    ArrayOop table = basicAt (1)->asArrayOop ();
    Oop key, value;
    LinkOop link;
    Oop * hp;
    int tablesize;

    /* now see if table is valid */
    if ((tablesize = table->size ()) < 3)
    {
        printf ("Table Size: %d\n", tablesize);
        perror ("system error lookup on null table");
    }
    else
    {
        hash = 1 + (3 * (hash % (tablesize / 3)));
        assert (hash <= tablesize - 2);
        hp = (Oop *)table->vonNeumannSpace () + (hash - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if ((!key.isNil ()) && (*fun) (key, extraVal))
            return {key, value};
        for (link = *(LinkOop *)hp; !link.isNil (); link = *(LinkOop *)hp)
        {
            hp = (Oop *)link->vonNeumannSpace ();
            key = *hp++;   /* link at: 1 */
            value = *hp++; /* link at: 2 */
            if (!key.isNil () && (*fun) (key, extraVal))
                return {key, value};
        }
    }

    return {Oop (), Oop ()};
}

static void dispatchPrint (int in, Oop anOop)
{
    if (anOop.isNil ())
        std::cout << blanks (in) + "<nil>\n";
    else if (anOop == MemoryManager::objFalse)
        std::cout << blanks (in) + "<FALSE>\n";
    else if (anOop == MemoryManager::objTrue)
        std::cout << blanks (in) + "<TRUE>\n";
    else if (anOop.isa () == MemoryManager::clsDictionary ||
             anOop.isa () == MemoryManager::clsSystemDictionary)
        anOop->asDictionaryOop ()->print (in);
#define Case(ClassName)                                                        \
    else if (anOop.isa () == MemoryManager::cls##ClassName)                    \
        anOop->as##ClassName##Oop ()                                           \
            ->print (in)
    else if (anOop.isa () == MemoryManager::clsInteger)
        anOop.asSmiOop ()->print (in);
    else if (anOop.isa () == MemoryManager::clsString)
        anOop->asSymbolOop ()->print (in);
    Case (Symbol);
    Case (Method);
    Case (Block);
    Case (Link);
    /*else if ((anOop->index () > MemoryManager::clsObject ().index ()) &&
             (anOop->index () < MemoryManager::clsSystemDictionary ().index ()))
        anOop->asClassOop ()
            ->print (in);*/
    else std::cout << blanks (in) + "Unknown object: index " << anOop.index ()
                   << " .\n";
}

void OopDesc::print (int in)
{
    dispatchPrint (in, *this);
}

#pragma mark ArrayOop
ArrayOop ArrayOopDesc::newWithSize (size_t size)
{
    ArrayOop newArr = memMgr.allocateOopObj (size)->asArrayOop ();
    newArr.setIsa (MemoryManager::clsArray);
    // printf (
    //    "Set isa of Arr %p to %p\n", newArr.isa ().index (), newArr.index ());
    return newArr;
}

ArrayOop ArrayOopDesc::fromVector (std::vector<Oop> vec)
{
    ArrayOop newArr = newWithSize (vec.size ());
    std::copy (vec.begin (), vec.end (), newArr->vonNeumannSpace ());
    return newArr;
}

ArrayOop
ArrayOopDesc::symbolArrayFromStringVector (std::vector<std::string> vec)
{
    std::vector<Oop> symVec;

    for (auto s : vec)
        symVec.push_back (SymbolOopDesc::fromString (s));

    return fromVector (symVec);
}

#pragma mark ByteArrayOop
ByteArrayOop ByteArrayOopDesc::newWithSize (size_t size)
{
    ByteArrayOop newArr = memMgr.allocateByteObj (size)->asByteArrayOop ();
    newArr.setIsa (MemoryManager::clsByteArray);
    return newArr;
}

ByteArrayOop ByteArrayOopDesc::fromVector (std::vector<uint8_t> vec)
{
    ByteArrayOop newArr = newWithSize (vec.size ());
    std::copy (vec.begin (), vec.end (), newArr->vonNeumannSpace ());
    return newArr;
}

int strHash (std::string str)
{
    int hash;
    const char * p;

    hash = 0;
    for (p = str.c_str (); *p; p++)
        hash += *p;
    if (hash < 0)
        hash = -hash;
    /* make sure it can be a smalltalk integer */
    if (hash > 16384)
        hash >>= 2;
    return hash;
}

int strTest (Oop key, std::string aString)
{
    if (key->asStringOop ()->strEquals (aString))
        return 1;
    return 0;
}

int identityTest (Oop key, Oop match)
{
    /*  DEBUG  printf ("Compare %s to %s\n",
               key->asStringOop ()->asCStr (),
               match->asStringOop ()->asCStr ());*/
    return key == match;
}

void SmiOopDesc::print (int in)
{
    std::cout << blanks (in) << "SmallInt: " << SmiOop (this).intValue ()
              << "\n";
}

CharOop CharOopDesc::newWith (intptr_t value)
{
    CharOop newChar = memMgr.allocateOopObj (1)->asCharOop ();
    newChar.setIsa (MemoryManager::clsChar);
    newChar->setValue (value);
    return newChar;
}

LinkOop LinkOopDesc::newWith (Oop a, Oop b)
{
    LinkOop newLink = memMgr.allocateOopObj (3)->asLinkOop ();
    newLink.setIsa (MemoryManager::clsLink);
    newLink->setOne (a);
    newLink->setTwo (b);
    return newLink;
}

#pragma mark ClassOop

void ClassOopDesc::addMethod (MethodOop method)
{
    if (methods ().isNil ())
        setMethods (DictionaryOopDesc::newWithSize (20));
    methods ()->symbolInsert (method->selector (), method);
    method->setMethodClass (this);
}

void ClassOopDesc::addClassMethod (MethodOop method)
{
    isa ()->addMethod (method);
}

void ClassOopDesc::setupClass (ClassOop superClass, std::string name)
{
    /* allocate metaclass if necessary */
    if (isa ().isNil ())
        setIsa (ClassOopDesc::allocateRawClass ());

    /* set metaclass isa to object metaclass */
    isa ().setIsa (MemoryManager::clsObjectMeta);

    isa ()->setName (SymbolOopDesc::fromString (name + "Meta"));
    setName (SymbolOopDesc::fromString (name));

    setupSuperclass (superClass);
}

void ClassOopDesc::setupSuperclass (ClassOop superClass)
{
    if (!superClass.isNil ())
    {
        isa ()->setSuperClass (superClass.isa ());
        setSuperClass (superClass);
    }
    else /* root object */
    {
        isa ()->setSuperClass (this);
        /* class of Object is the terminal class for both the metaclass and
         * class hierarchy */
    }
}

ClassOop ClassOopDesc::allocateRawClass ()
{
    ClassOop cls = memMgr.allocateOopObj (clsInstLength)->asClassOop ();
    return cls;
}

ClassOop ClassOopDesc::allocate (ClassOop superClass, std::string name)
{
    ClassOop metaCls = memMgr.allocateOopObj (clsInstLength)->asClassOop (),
             cls = memMgr.allocateOopObj (clsInstLength)->asClassOop ();

    cls.setIsa (metaCls);

    cls->setupClass (superClass, name);

    return cls;
}

ClassPair ClassPair::allocateRaw ()
{
    ClassOop metaCls = ClassOopDesc::allocateRawClass ();
    ClassOop cls = ClassOopDesc::allocateRawClass ();
    return ClassPair (cls, metaCls);
}

void DictionaryOopDesc::symbolInsert (SymbolOop key, Oop value)
{
    insert (key.index (), key, value);
}

Oop DictionaryOopDesc::symbolLookup (SymbolOop aSymbol)
{
    return findPairByFun<Oop> (aSymbol.index (), aSymbol, identityTest).second;
}

Oop DictionaryOopDesc::symbolLookup (std::string aString)
{
    SymbolOop sym = SymbolOopDesc::fromString (aString);
    return symbolLookup (sym);
}

StringOop StringOopDesc::fromString (std::string aString)
{
    StringOop newObj =
        memMgr.allocateByteObj (aString.size () + 1)->asStringOop ();

    newObj.setIsa (MemoryManager::clsString);
    strncpy (
        (char *)newObj->vonNeumannSpace (), aString.c_str (), aString.size ());
    return newObj;
}

SymbolOop SymbolOopDesc::fromString (std::string aString)
{
    SymbolOop newObj = MemoryManager::objSymbolTable
                           ->findPairByFun (strHash (aString), aString, strTest)
                           .first->asSymbolOop ();

    if (!newObj.isNil ())
        return newObj;

    /* not found, must make */
    newObj = memMgr.allocateByteObj (aString.size () + 1)->asSymbolOop ();

    newObj.setIsa (MemoryManager::clsSymbol);
    strncpy (
        (char *)newObj->vonNeumannSpace (), aString.c_str (), aString.size ());
    MemoryManager::objSymbolTable->insert (
        strHash (aString.c_str ()), newObj, Oop ());
    return newObj;
}

void SymbolOopDesc::print (int in)
{
    std::cout << blanks (in) << "Symbol " << this << ": "
              << (const char *)vonNeumannSpace () << "\n";
}

#pragma mark MethodOop

MethodOop MethodOopDesc::allocate ()
{
    MethodOop newMeth = memMgr.allocateOopObj (clsNstLength)->asMethodOop ();
    newMeth.setIsa (MemoryManager::clsMethod);
    return newMeth;
}

#pragma mark BlockOop

BlockOop BlockOopDesc::allocate ()
{
    BlockOop newBlock = memMgr.allocateOopObj (clsNstLength)->asBlockOop ();
    newBlock.setIsa (MemoryManager::clsBlock);
    return newBlock;
}