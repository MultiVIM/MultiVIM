#include <algorithm>
#include <iostream>

#include "Lowlevel/MVPrinting.hxx"
#include "Oops.hxx"

template <typename ExtraType>
std::pair<Oop, Oop> DictionaryOop::findPairByFun (int hash, ExtraType extraVal,
                                                  int (*fun) (Oop, ExtraType))
{
    ArrayOop table = basicAt (1).asArrayOop ();
    Oop key, value;
    LinkOop link;
    Oop * hp;
    int tablesize;

    /* now see if table is valid */
    if ((tablesize = table.size ()) < 3)
    {
        printf ("Table Size: %d\n", tablesize);
        perror ("system error lookup on null table");
    }
    else
    {
        hash = 1 + (3 * (hash % (tablesize / 3)));
        assert (hash <= tablesize - 2);
        hp = (Oop *)table.vonNeumannSpace () + (hash - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if ((!key.isNil ()) && (*fun) (key, extraVal))
            return {key, value};
        for (link = *(LinkOop *)hp; !link.isNil (); link = *(LinkOop *)hp)
        {
            hp = (Oop *)link.vonNeumannSpace ();
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
    if (anOop.isa () == memMgr.clsDictionary ())
        anOop.asDictionaryOop ().print (in);
#define Case(ClassName)                                                        \
    else if (anOop.isa () == memMgr.cls##ClassName ())                         \
        anOop.as##ClassName##Oop ()                                            \
            .print (in)
    else if (anOop.isa () == memMgr.clsInteger ())
        anOop.asSmiOop ().print (in);
    Case (Symbol);
    Case (Method);
    Case (Block);
    else if ((anOop.index () > memMgr.clsObject ().index ()) &&
             (anOop.index () < memMgr.clsSystemDictionary ().index ()))
        anOop.asClassOop ()
            .print (in);
    else if (anOop.isNil ()) std::cout << blanks (in) + "(nil)\n";
    else std::cout << blanks (in) + "Unknown object: index " << anOop.index ()
                   << " .\n";
}

void Oop::print (int in)
{
    dispatchPrint (in, *this);
}

#pragma mark ArrayOop
ArrayOop ArrayOop ::newWithSize (size_t size)
{
    ArrayOop newArr = memMgr.allocateOopObj (size).asArrayOop ();
    newArr.setIsa (memMgr.clsArray ());
    return newArr;
}

ArrayOop ArrayOop ::fromVector (std::vector<Oop> vec)
{
    ArrayOop newArr = newWithSize (vec.size ());
    std::copy (vec.begin (), vec.end (), newArr.vonNeumannSpace ());
    return newArr;
}

ArrayOop ArrayOop::symbolArrayFromStringVector (std::vector<std::string> vec)
{
    std::vector<Oop> symVec;

    for (auto s : vec)
        symVec.push_back (SymbolOop::fromString (s));

    return fromVector (symVec);
}

#pragma mark ByteArrayOop
ByteArrayOop ByteArrayOop ::newWithSize (size_t size)
{
    ByteArrayOop newArr = memMgr.allocateByteObj (size).asByteArrayOop ();
    newArr.setIsa (memMgr.clsByteArray ());
    return newArr;
}

ByteArrayOop ByteArrayOop ::fromVector (std::vector<uint8_t> vec)
{
    ByteArrayOop newArr = newWithSize (vec.size ());
    std::copy (vec.begin (), vec.end (), newArr.vonNeumannSpace ());
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
    if (key.asStringOop ().strEquals (aString))
        return 1;
    return 0;
}

int identityTest (Oop key, Oop match)
{
    return key == match;
}

void SmiOop::print (int in)
{
    std::cout << blanks (in) << "SmallInt: " << value << "\n";
}

LinkOop LinkOop::newWith (Oop a, Oop b)
{
    LinkOop newLink = memMgr.allocateOopObj (3).asLinkOop ();
    newLink.setOne (a);
    newLink.setTwo (b);
    return newLink;
}

#pragma mark ClassOop

void ClassOop::addMethod (MethodOop method)
{
    if (methods ().isNil ())
        setMethods (DictionaryOop::newWithSize (20));
    methods ().symbolInsert (method.selector (), method);
    method.setMethodClass (*this);
}

void ClassOop::addClassMethod (MethodOop method)
{
    isa ().addMethod (method);
}

void ClassOop::setupClass (ClassOop superClass, std::string name)
{
    isa ().setIsa (memMgr.clsObjectMeta ());

    isa ().setName (SymbolOop::fromString (name + "Meta"));
    setName (SymbolOop::fromString (name));

    if (!superClass.isNil ())
    {
        isa ().setSuperClass (superClass.isa ());
        setSuperClass (superClass);
    }
    else /* root object */
    {
        isa ().setSuperClass (*this);
        /* class of Object is the terminal class for both the metaclass and
         * class hierarchy */
    }
}

ClassOop ClassOop::allocateRawClass ()
{
    ClassOop cls = memMgr.allocateOopObj (clsInstLength).asClassOop ();
    return cls;
}

ClassOop ClassOop::allocate (ClassOop superClass, std::string name)
{
    ClassOop metaCls = memMgr.allocateOopObj (clsInstLength).asClassOop (),
             cls = memMgr.allocateOopObj (clsInstLength).asClassOop ();

    cls.setIsa (metaCls);

    cls.setupClass (superClass, name);

    return cls;
}

ClassPair ClassPair::allocateRaw ()
{
    ClassOop metaCls = ClassOop::allocateRawClass ();
    ClassOop cls = ClassOop::allocateRawClass ();
    return ClassPair (cls, metaCls);
}

void DictionaryOop::symbolInsert (SymbolOop key, Oop value)
{
    insert (key.index (), key, value);
}

Oop DictionaryOop::symbolLookup (SymbolOop aSymbol)
{
    return findPairByFun<Oop> (aSymbol.index (), aSymbol, identityTest).second;
}

Oop DictionaryOop::symbolLookup (std::string aString)
{
    SymbolOop sym = SymbolOop::fromString (aString);
    return symbolLookup (sym);
}

StringOop StringOop::fromString (std::string aString)
{
    StringOop newObj =
        memMgr.allocateByteObj (aString.size () + 1).asStringOop ();

    newObj.setIsa (memMgr.clsString ());
    strncpy (
        (char *)newObj.vonNeumannSpace (), aString.c_str (), aString.size ());
    return newObj;
}

SymbolOop SymbolOop::fromString (std::string aString)
{
    SymbolOop newObj = memMgr.objSymbolTable ()
                           .findPairByFun (strHash (aString), aString, strTest)
                           .first.asSymbolOop ();

    if (!newObj.isNil ())
        return newObj;

    /* not found, must make */
    newObj = memMgr.allocateByteObj (aString.size () + 1).asSymbolOop ();

    newObj.setIsa (memMgr.clsSymbol ());
    strncpy (
        (char *)newObj.vonNeumannSpace (), aString.c_str (), aString.size ());
    memMgr.objSymbolTable ().insert (
        strHash (aString.c_str ()), newObj, Oop ());
    return newObj;
}

void SymbolOop::print (int in)
{
    std::cout << blanks (in) << "Symbol " << index () << ": "
              << (const char *)vonNeumannSpace () << "\n";
}

#pragma mark MethodOop

MethodOop MethodOop::allocate ()
{
    MethodOop newMeth = memMgr.allocateOopObj (clsNstLength).asMethodOop ();
    newMeth.setIsa (memMgr.clsMethod ());
    return newMeth;
}

#pragma mark BlockOop

BlockOop BlockOop::allocate ()
{
    BlockOop newBlock = memMgr.allocateOopObj (clsNstLength).asBlockOop ();
    newBlock.setIsa (memMgr.clsBlock ());
    return newBlock;
}