#ifndef CLASSOOP_HXX_
#define CLASSOOP_HXX_

#include "ObjectMemory/ObjectMemory.hxx"
#include "Oop.hxx"

int strHash (std::string str);
int strTest (Oop key, std::string aString);
int identityTest (Oop key, Oop match);

class NativeCodeOopDesc : public OopOopDesc
{
  public:
    typedef void (*Fun) (void *);

    DeclareAccessorPair (ByteArrayOop, data, setData);
    /* Pointer to function code area */
    uint8_t * funCode ();
    /* Pointer to function pointer */
    DeclareAccessorPair (Fun, fun, setFun);

    static NativeCodeOop newWithSize (size_t codeSize);
};

class CharOopDesc : public OopOopDesc
{
  public:
    DeclareAccessorPair (SmiOop, value, setValue);

    static CharOop newWith (intptr_t value);
};

class LinkOopDesc : public OopOopDesc
{
  public:
    DeclareAccessorPair (Oop, one, setOne);
    DeclareAccessorPair (Oop, two, setTwo);
    DeclareAccessorPair (LinkOop, nextLink, setNextLink);

    void print (int in);

    static LinkOop newWith (Oop a, Oop b);
};

class ArrayOopDesc : public OopOopDesc
{
  public:
    static ArrayOop newWithSize (size_t size);
    static ArrayOop fromVector (std::vector<Oop> vec);
    static ArrayOop symbolArrayFromStringVector (std::vector<std::string> vec);
};

class DictionaryOopDesc : public OopOopDesc
{
  public:
    /**
     * Inserts /a value under /a key under the hash /a hash.
     */
    void insert (intptr_t hash, Oop key, Oop value);

    /**
     * Looks for a key-value pair by searching for the given hash, then running
     * /a fun on each element that matches, with the value attached to the key
     * as the first parameter and /a extraVal as the second parameter. When
     * /a fun returns true, that key-value pair is returned.
     */
    template <typename ExtraType>
    std::pair<Oop, Oop> findPairByFun (intptr_t hash, ExtraType extraVal,
                                       int (*fun) (Oop, ExtraType));

#pragma mark symbol table functions
    /*
     * Symbol table functions - to be moved later to a dedicated subclass!!!
     */

    /*
     * Inserts a value to be associated with a symbol key.
     */
    void symbolInsert (SymbolOop key, Oop value);

    Oop symbolLookup (SymbolOop aSymbol);

    /*
     * Looks up the value associated with whichever symbol carries string value
     * aString.
     */
    Oop symbolLookup (std::string aString);

    /*
     * Namespace functions.
     */
    ClassOop findOrCreateClass (ClassOop superClass, std::string name);
    /* Create or find a subnamespace with the given name. Name may be colonised.
     */
    DictionaryOop subNamespaceNamed (std::string name);
    /* Look up a symbol, searching superspaces if necessary. */
    Oop symbolLookupNamespaced (std::string aString);

#pragma mark creation

    static DictionaryOop newWithSize (size_t numBuckets);

#pragma mark misc
    void print (int in);
};

/* Only at:ifAbsent: and at:put: need be implemented to do ByteArrays. The other
 * logic can remain identical. */
class ByteArrayOopDesc : public ByteOopDesc
{
  public:
    static ByteArrayOop newWithSize (size_t size);
    static ByteArrayOop fromVector (std::vector<uint8_t> vec);
};

class StringOopDesc : public ByteArrayOopDesc
{
  public:
    static StringOop fromString (std::string aString);

    bool strEquals (std::string aString);
    const char * asCStr ();
    std::string asString ();
};

class SymbolOopDesc : public StringOopDesc
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

class MethodOopDesc : public OopOopDesc
{
    static const int clsNstLength = 10;

  public:
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (ArrayOop, literals, setLiterals);
    DeclareAccessorPair (SmiOop, argumentCount, setArgumentCount);
    DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
    DeclareAccessorPair (SmiOop, heapVarsSize, setHeapVarsSize);
    DeclareAccessorPair (SmiOop, stackSize, setStackSize);
    DeclareAccessorPair (StringOop, sourceText, setSourceText);
    DeclareAccessorPair (SymbolOop, selector, setSelector);
    DeclareAccessorPair (ClassOop, methodClass, setMethodClass);
    DeclareAccessorPair (SmiOop, watch, setWatch);

    static MethodOop allocate ();

    void print (int in);
};

class BlockOopDesc : public OopOopDesc
{
    static const int clsNstLength = 12;

  public:
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (ArrayOop, literals, setLiterals);
    DeclareAccessorPair (SmiOop, argumentCount, setArgumentCount);
    DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
    DeclareAccessorPair (SmiOop, heapVarsSize, setHeapVarsSize);
    DeclareAccessorPair (SmiOop, stackSize, setStackSize);
    DeclareAccessorPair (StringOop, sourceText, setSourceText);
    DeclareAccessorPair (Oop, receiver, setReceiver);
    DeclareAccessorPair (ArrayOop, parentHeapVars, setParentHeapVars);
    DeclareAccessorPair (ContextOop, homeMethodContext, setHomeMethodContext);
    /* This is either:
     * A) an Association<Class, Block>;
     * B) a HandlerCollection;
     * C) an ensure: block handler; or
     * D) an ifCurtailed: block handler.
     * We differentiate between A/B v.s. C or D by the selector; we
     * differentiate A and B by their isKindOf: response.
     */
    DeclareAccessorPair (OopOop, handler, setHandler);

    /**
     * Allocates a new empty block.
     */
    static BlockOop allocate ();

    void print (int in);
};

class ContextOopDesc : public OopOopDesc
{
    static const int clsNstLength = 12;

  public:
    DeclareAccessorPair (ContextOop, previousContext, setPreviousContext);
    DeclareAccessorPair (SmiOop, programCounter, setProgramCounter);
    DeclareAccessorPair (SmiOop, stackPointer, setStackPointer);
    DeclareAccessorPair (Oop, receiver, setReceiver);
    DeclareAccessorPair (ArrayOop, arguments, setArguments);
    DeclareAccessorPair (ArrayOop, temporaries, setTemporaries);
    DeclareAccessorPair (ArrayOop, heapVars, setHeapVars);
    DeclareAccessorPair (ArrayOop, parentHeapVars, setParentHeapVars);
    DeclareAccessorPair (ArrayOop, stack, setStack);
    DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
    DeclareAccessorPair (OopOop, methodOrBlock, setMethodOrBlock);
    DeclareAccessorPair (ContextOop, homeMethodContext, setHomeMethodContext);

    /* Initialise all fields that need to be (i.e. the SmiOops) */
    void init ();
    /* Fetch the next byte of bytecode, incrementing the program counter. */
    uint8_t fetchByte ();

    /**
     * Duplicate the top of the stack
     */
    void dup ();
    /**
     * Push a value to the stack
     */
    void push (Oop obj);
    /**
     * Pop a value from the stack
     */
    Oop pop ();
    /**
     * Fetch the value at the top of the stack
     */
    Oop top ();

    bool isBlockContext ();

    void print (int in);

    static ContextOop newWithBlock (BlockOop aMethod);
    static ContextOop newWithMethod (Oop receiver, MethodOop aMethod);
};

class ProcessOopDesc : public OopOopDesc
{
    static const int clsNstLength = 4;

  public:
    DeclareAccessorPair (ContextOop, context, setContext);

    static ProcessOop allocate ();
};

#include "Oops.inl.hxx"

#endif