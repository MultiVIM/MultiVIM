#ifndef CLASSOOP_HXX_
#define CLASSOOP_HXX_

#include "ObjectMemory/ObjectMemory.hxx"

class LinkOop : public OopOop
{
  public:
    DeclareAccessorPair (Oop, one, setOne);
    DeclareAccessorPair (Oop, two, setTwo);
    DeclareAccessorPair (LinkOop, nextLink, setNextLink);

    static LinkOop newWith (Oop a, Oop b);
};

class ArrayOop : public OopOop
{
};

class DictionaryOop : public OopOop
{
  public:
    /**
     * Inserts /a value under /a key under the hash /a hash.
     */
    void insert (int hash, Oop key, Oop value);

    /**
     * Looks for a key-value pair by searching for the given hash, then running
     * /a fun on each element that matches, with the value attached to the key
     * as the first parameter and /a extraVal as the second parameter. When
     * /a fun returns true, that key-value pair is returned.
     */
    template <typename ExtraType>
    std::pair<Oop, Oop> findPairByFun (int hash, ExtraType extraVal,
                                       int (*fun) (Oop, ExtraType));

    void print (int in);
};

/* Only at:ifAbsent: and at:put: need be implemented to do ByteArrays. The other
 * logic can remain identical. */
class ByteArrayOop : public ByteOop
{
  public:
    static ByteArrayOop newWithSize (size_t size);
};

class StringOop : public ByteArrayOop
{
  public:
    static StringOop fromString (std::string aString);

    bool strEquals (std::string aString);
};

class SymbolOop : public StringOop
{
  public:
    static SymbolOop fromString (std::string aString);

    void print (int in);
};

class ClassPair
{
  public:
    ClassOop cls;
    ClassOop metaCls;

    ClassPair (ClassOop cls, ClassOop metaCls) : cls (cls), metaCls (metaCls)
    {
    }

    /**
     * Allocates a raw classpair: no name, no iVar vector, no method dictionary.
     */
    static ClassPair allocateRaw ();
};

class MethodOop : public OopOop
{
  public:
    DeclareAccessorPair (StringOop, sourceText, setSourceText);
    DeclareAccessorPair (SymbolOop, selector, setSelector);
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (ArrayOop, literals, setLiterals);
    DeclareAccessorPair (SmiOop, stackSize, setStackSize);
    DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
    DeclareAccessorPair (ClassOop, methodClass, setMethodClass);
    DeclareAccessorPair (SmiOop, watch, setWatch);
};

#define textInMethod 1
#define messageInMethod 2
#define bytecodesInMethod 3
#define literalsInMethod 4
#define stackSizeInMethod 5
#define temporarySizeInMethod 6
#define methodClassInMethod 7
#define watchInMethod 8

#include "Oops.inl.hxx"

#endif