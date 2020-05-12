#ifndef CLASSOOP_HXX_
#define CLASSOOP_HXX_

#include "ObjectMemory/ObjectMemory.hxx"

int strHash (std::string str);
int strTest (Oop key, std::string aString);
int identityTest (Oop key, Oop match);

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
  public:
    static ArrayOop newWithSize (size_t size);
    static ArrayOop fromVector (std::vector<Oop> vec);
    static ArrayOop symbolArrayFromStringVector (std::vector<std::string> vec);
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

#pragma mark symbol table functions
    /*
     * Symbol table functions - to be moved later to a dedicated subclass!!!
     */

    /*
     * Inserts a value to be associated with a symbol key.
     */
    void symbolInsert (SymbolOop key, Oop value);

    /*
     * Looks up the value associated with whichever symbol carries string value
     * aString.
     */
    Oop symbolLookup (std::string aString);

#pragma mark creation

    static DictionaryOop newWithSize (size_t numBuckets);

#pragma mark misc
    void print (int in);
};

/* Only at:ifAbsent: and at:put: need be implemented to do ByteArrays. The other
 * logic can remain identical. */
class ByteArrayOop : public ByteOop
{
  public:
    static ByteArrayOop newWithSize (size_t size);
    static ByteArrayOop fromVector (std::vector<uint8_t> vec);
};

class StringOop : public ByteArrayOop
{
  public:
    static StringOop fromString (std::string aString);

    bool strEquals (std::string aString);
    std::string asString ();
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
    static const int clsNstLength = 8;

  public:
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (ArrayOop, literals, setLiterals);
    DeclareAccessorPair (StringOop, sourceText, setSourceText);
    DeclareAccessorPair (SymbolOop, selector, setSelector);
    DeclareAccessorPair (SmiOop, stackSize, setStackSize);
    DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
    DeclareAccessorPair (ClassOop, methodClass, setMethodClass);
    DeclareAccessorPair (SmiOop, watch, setWatch);

    static MethodOop allocate ();

    void print (int in);
};

class BlockOop : public OopOop
{
    static const int clsNstLength = 8;

  public:
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (ArrayOop, literals, setLiterals);
    DeclareAccessorPair (StringOop, sourceText, setSourceText);
    DeclareAccessorPair (SymbolOop, selector, setSelector);
    DeclareAccessorPair (SmiOop, stackSize, setStackSize);
    DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
    /* FIXME: We will need to start copying blocks before pushing them to the
     * stack, because otherwise how do we reliably get the receiver at the time
     * of closure? */
    DeclareAccessorPair (Oop, receiver, setReceiver);
    DeclareAccessorPair (SmiOop, argumentCount, setArgumentCount);

    /**
     * Allocates a new empty block.
     */
    static BlockOop allocate ();

    void print (int in);
};

class ContextOop : public OopOop
{
    static const int clsNstLength = 7;

  public:
    DeclareAccessorPair (ContextOop, previousContext, setPreviousContext);
    DeclareAccessorPair (SmiOop, programCounter, setProgramCounter);
    DeclareAccessorPair (Oop, receiver, setReceiver);
    DeclareAccessorPair (ArrayOop, arguments, setArguments);
    DeclareAccessorPair (ArrayOop, temporaries, setTemporaries);
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (OopOop, methodOrBlock, setMethodOrBlock);

    /* Fetch the next byte of bytecode, incrementing the program counter. */
    uint8_t fetchByte ();

    void print (int in);
};

class ProcessOop : public OopOop
{
    const int clsNstLength = 4;

  public:
    DeclareAccessorPair (ContextOop, context, setContext);
};

#include "Oops.inl.hxx"

#endif