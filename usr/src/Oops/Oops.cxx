#include <iostream>

#include "Lowlevel/MVPrinting.hxx"
#include "Oops.hxx"

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

#pragma mark DictionaryOop
void DictionaryOop::insert (int hash, Oop key, Oop value)
{
    ArrayOop table;
    LinkOop link, nwLink, nextLink;
    Oop tablentry;

    /* first get the hash table */
    table = basicAt (1).asArrayOop ();

    if (table.size () < 3)
    {
        perror ("attempt to insert into too small name table");
    }
    else
    {
        hash = 3 * (hash % (table.size () / 3));
        assert (hash <= table.size () - 3);
        tablentry = table.basicAt (hash + 1);

        /* FIXME: I adapted this from the PDST C sources, and this doesn't
         * appear to handle anything other than symbols (because of
         * tablentry == key). Make it a template in the future or make it accept
         * a comparison callback like findPairByFun() in the future, maybe? */
        if (tablentry.isNil () || (tablentry == key))
        {
            table.basicatPut (hash + 1, key);
            table.basicatPut (hash + 2, value);
        }
        else
        {
            nwLink = LinkOop::newWith (key, value);
            link = table.basicAt (hash + 3).asLinkOop ();
            if (link.isNil ())
            {
                table.basicatPut (hash + 3, nwLink);
            }
            else
                while (1)
                    /* ptrEq (orefOf (link, 1), (objRef)key)) */
                    if (link.one () == key)
                    {
                        /* get rid of unwanted Link */
                        // isVolatilePut (nwLink, false);
                        link.setTwo (value);
                        break;
                    }
                    else if ((nextLink = link.nextLink ()).isNil ())
                    {
                        link.nextLink () = nwLink;
                        break;
                    }
                    else
                        link = nextLink;
        }
    }
}

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
        if (!key.isNil () && (*fun) (key, extraVal))
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
    Case (Smi);
    Case (Symbol);
    else if (anOop.isNil ()) std::cout << blanks (in) + "(nil)\n";
    else std::cout << blanks (in) + "Unknown object.\n";
}

void DictionaryOop::print (int in)
{
    ArrayOop table = basicAt (1).asArrayOop ();
    Oop key, value;
    LinkOop link;
    Oop * hp;
    int tablesize;

    std::cout << blanks (in) + "Dictionary {\n";

    /* now see if table is valid */
    if ((tablesize = table.size ()) < 3)
    {
        printf ("Table Size: %d\n", tablesize);
        perror ("system error lookup on null table");
    }
    for (int i = 1; i <= table.size (); i += 3)
    {
        hp = (Oop *)table.vonNeumannSpace () + (i - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */

        std::cout << blanks (in + 1) + "{ Key:\n";
        dispatchPrint (in + 2, key);
        std::cout << blanks (in + 1) + " Value:\n";
        dispatchPrint (in + 2, value);
        std::cout << blanks (in + 1) + "}\n";

        for (link = *(LinkOop *)hp; !link.isNil (); link = *(LinkOop *)hp)
        {
            hp = (Oop *)link.vonNeumannSpace ();
            key = *hp++;   /* link at: 1 */
            value = *hp++; /* link at: 2 */
            std::cout << blanks (in + 2) + "{ Key:\n";
            dispatchPrint (in + 3, key);
            std::cout << blanks (in + 2) + " Value:\n";
            dispatchPrint (in + 3, value);
            std::cout << blanks (in + 2) + "}\n";
        }
    }

    std::cout << blanks (in) + "}\n";
}

#pragma mark ClassOop
ClassOop ClassOop::allocateRawClass ()
{
    ClassOop cls = memMgr.allocateOopObj (clsInstLength).asClassOop ();
    return cls;
}

ClassPair ClassPair::allocateRaw ()
{
    ClassOop metaCls = ClassOop::allocateRawClass ();
    ClassOop cls = ClassOop::allocateRawClass ();
    return ClassPair (cls, metaCls);
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
    {
        return 1;
    }
    return 0;
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
    std::cout << blanks (in) << "Symbol: " << (const char *)vonNeumannSpace ()
              << "\n";
}