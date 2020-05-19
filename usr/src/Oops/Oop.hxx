#pragma once

#include <stddef.h>
#include <stdint.h>

#define DeclareAccessorPair(Type, GetName, SetName)                            \
    inline Type & GetName ();                                                  \
    inline Type SetName (Type toValue)

class ActualObject;
class StringOop;
class ByteArrayOop;
class BlockOop;
class ClassOop;
class ClassPair;
class ArrayOop;
class DictionaryOop;
class FloatOop;
class LinkOop;
class SymbolOop;
class SmiOop;
class MethodOop;
class ContextOop;
class Oop;
class OopOop;
class ProcessOop;

class Klass
{
    virtual void print (int in);
};

#include "Lowlevel/MVBeginPackStruct.h"
class Oop
{
  protected:
    friend class MemoryManager;
    friend class ActualObject;

    PACKSTRUCT bool isIntegerFlag : 1;
    PACKSTRUCT intptr_t value : 31;

    ActualObject * actualObject ();

  public:
    Oop () : value (0), isIntegerFlag (0)
    {
    }
    Oop (bool isIntegerFlag, intptr_t value)
        : value (value), isIntegerFlag (isIntegerFlag)
    {
    }

    inline bool operator!= (const Oop & anOop);
    inline bool operator== (const Oop & anOop);

    /**
     * Nil is defined a pointer to object table entry 0.
     */
    inline bool isNil ();
    inline bool isInteger ();

    /**
     * Gets the index into the object table of this Oop.
     */
    size_t index ();
    /**
     * The size of the object's von Neumann space, i.e. the number of Oops or
     * Bytes.
     */
    inline size_t size ();
    inline ClassOop isa ();

    inline ClassOop setIsa (ClassOop val);

#pragma mark conversions
    ArrayOop & asArrayOop ();
    BlockOop & asBlockOop ();
    ByteArrayOop & asByteArrayOop ();
    ClassOop & asClassOop ();
    DictionaryOop & asDictionaryOop ();
    FloatOop & asFloatOop ();
    LinkOop & asLinkOop ();
    StringOop & asStringOop ();
    SymbolOop & asSymbolOop ();
    SmiOop & asSmiOop ();
    ContextOop & asContextOop ();
    MethodOop & asMethodOop ();
    ProcessOop & asProcessOop ();
    OopOop & asOopOop ();
    Oop & asOop ()
    {
        return *this;
    }

    void print (int in);
};
#include "Lowlevel/MVEndPackStruct.h"

class SmiOop : public Oop
{
  public:
    SmiOop (intptr_t value) : Oop (true, value)
    {
    }

    int intValue ()
    {
        return value;
    }

    inline int postInc ();
    inline int preInc ();
    inline int postDec ();
    inline int preDec ();

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