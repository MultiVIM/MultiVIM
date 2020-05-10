#ifndef OBJECTMEMORY_HXX_
#define OBJECTMEMORY_HXX_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class ActualObject;
class StringOop;
class ClassOop;
class ClassPair;
class ArrayOop;
class DictionaryOop;
class LinkOop;
class SymbolOop;
class SmiOop;

#include "Lowlevel/MVBeginPackStruct.h"
class Oop
{
  protected:
    friend class MemoryManager;
    friend class ActualObject;

    PACKSTRUCT bool isIntegerFlag : 1;
    PACKSTRUCT intptr_t value : 31;

    ActualObject * actualObject ();
    /**
     * Gets the index into the object table of this Oop.
     */
    size_t index ();

  public:
    Oop () : value (0), isIntegerFlag (0)
    {
    }
    Oop (bool isIntegerFlag, intptr_t value)
        : value (value), isIntegerFlag (isIntegerFlag)
    {
    }

    inline bool operator== (const Oop & anOop);

    /**
     * Nil is defined a pointer to object table entry 0.
     */
    inline bool isNil ();
    inline bool isInteger ();

    /**
     * The size of the object's von Neumann space, i.e. the number of Oops or
     * Bytes.
     */
    inline size_t size ();
    inline ClassOop & isa ();

    inline ClassOop & setIsa (ClassOop val);

    ArrayOop & asArrayOop ();
    ClassOop & asClassOop ();
    DictionaryOop & asDictionaryOop ();
    LinkOop & asLinkOop ();
    StringOop & asStringOop ();
    SymbolOop & asSymbolOop ();
    SmiOop & asSmiOop ();
};
#include "Lowlevel/MVEndPackStruct.h"

class SmiOop : public Oop
{
  public:
    SmiOop (intptr_t value) : Oop (true, value)
    {
    }

    void print (int in);
};

class OopOop : public Oop
{
  public:
    Oop * vonNeumannSpace ();

    /**
     * Return a reference to the Oop value at /a index.
     * 1-based indexing.
     */
    Oop & basicAt (size_t index);

    /**
     * At index /a index, put Oop value /a value.
     * 1-based indexing.
     * @return Reference to the value placed at the index.
     */
    Oop & basicatPut (size_t index, Oop value);
};

class ByteOop : public Oop
{
  public:
    uint8_t * vonNeumannSpace ();

    /**
     * Return a reference to the byte value at /a index.
     * 1-based indexing.
     */
    uint8_t & basicAt (size_t index);

    /**
     * At index /a index, put byte value /a value.
     * 1-based indexing.
     * @return Reference to value placed at the index.
     */
    uint8_t & basicatPut (size_t index, uint8_t value);
};

class ClassOop : public OopOop
{
  public:
    /*
     * For a system class (i.e. one that can't be altered by the running
     * Smalltalk image), defines the size of one of its instances.
     */
    static const int clsInstLength = 5;

    inline SymbolOop & name ();
    inline ClassOop & superClass ();
    inline DictionaryOop & methods ();
    inline SmiOop & nstSize ();
    inline ArrayOop & nstVars ();

    /**
     * Allocates a raw class. Does NOT set up its name, superclass, methods
     * dictionary, size, or instance variables fields.
     */
    static ClassOop allocateRawClass ();
};

class MemoryManager
{
    friend class Oop;

    size_t lowestFreeSlot;

    struct TableEntry
    {
        ActualObject * obj;
    };
    std::vector<TableEntry> _table;

  public:
    Oop objNil ();
    Oop objTrue ();
    Oop objFalse ();

    /**
     * Retrieves the dictionary of symbols, indexed by string hash value.
     */
    DictionaryOop objSymbolTable ();
    /*
     * Retrieves the dictionary of global variables, indexed by object table
     * entry of the symbol. Known as 'Smalltalk' in-world.
     */
    DictionaryOop objGlobals ();

    ClassOop clsObjectMeta ();
    ClassOop clsObject ();
    ClassOop clsSmi ();
    ClassOop clsSymbol ();
    ClassOop clsString ();
    ClassOop clsArray ();
    ClassOop clsByteArray ();
    ClassOop clsMethod ();
    ClassOop clsProcess ();
    ClassOop clsUndefinedObject ();
    ClassOop clsDictionary ();
    ClassOop clsLink ();

    inline ActualObject * actualObjectForOop (Oop anOop);

    /**
     * Allocates a raw classpair (no iVar vector, no method dictionary.)
     */
    ClassPair allocateRawClassPair (std::string name);

    /**
     * Allocates an object composed of object pointers.
     */
    OopOop allocateOopObj (size_t len);

    /**
     * Allocates an object composed of bytes.
     */
    ByteOop allocateByteObj (size_t len);

    /**
     * Performs a cold boot of the Object Manager. Essential objects are
     * provisionally set up such that it is possible to define classes.
     */
    void coldBoot ();
};

#include "Lowlevel/MVBeginPackStruct.h"
class ActualObject
{
  protected:
    friend class Oop;
    friend class OopOop;
    friend class ByteOop;
    friend class MemoryManager;

    enum
    {
        kByte,
        kOopRef,
        kWeakOopRef,
    } kind : 3;
    uintptr_t size : 29;

    /* Oop to this object's class. */
    ClassOop isa PACKSTRUCT;
    /* Space for the object's fields. */
    union {
        uint8_t bytes[0];
        Oop oops[0];
    } vonNeumannSpace PACKSTRUCT;
};
#include "Lowlevel/MVEndPackStruct.h"

extern MemoryManager memMgr;

#include "ObjectMemory.inl.hxx"

#endif