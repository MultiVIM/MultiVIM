#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define DeclareAccessorPair(Type, GetName, SetName)                            \
    inline Type GetName ();                                                    \
    inline Type SetName (Type toValue)

class OopDesc;

class ArrayOopDesc;
class ByteOopDesc;
class ByteArrayOopDesc;
class BlockOopDesc;
class CharOopDesc;
class ClassOopDesc;
class ContextOopDesc;
class DictionaryOopDesc;
class FloatOopDesc;
class LinkOopDesc;
class SmiOopDesc;
class StringOopDesc;
class SymbolOopDesc;
class MemOopDesc;
class MethodOopDesc;
class OopOopDesc;
class ProcessOopDesc;

/**
 * N.b. this is totally illegal, but works on every platform and compiler
 * I've tried.
 */
#include "Lowlevel/MVBeginPackStruct.h"
template <typename T> class OopRef
{
  protected:
    friend class OopDesc;
    friend class OopRef;
    friend class OopRef<OopDesc>;
    friend class OopRef<ClassOopDesc>;
    friend class OopRef<T>;

  public:
    T * dat;

    OopRef () : dat (NULL)
    {
    }
    OopRef (OopDesc *);

    T * addr ();
    bool isNil ();
    bool isInteger ();
    inline OopRef<ClassOopDesc> isa ();
    inline OopRef<ClassOopDesc> setIsa (OopRef<ClassOopDesc> val);

    operator OopRef<OopDesc> ();
    inline bool operator!= (const OopRef<OopDesc> & anOop)
    {
        return !operator== (anOop);
    }

    inline bool operator== (const OopRef<OopDesc> & anOop)
    {
        return dat == anOop.dat;
    }
    T operator* () const
    {
        return dat;
    }
    T * operator-> () const
    {
        return dat;
    }

    OopRef<SmiOopDesc> asSmiOop ();
    OopRef<FloatOopDesc> asFloatOop ();
    intptr_t index ()
    {
        return (intptr_t)dat;
    }

    static OopRef<T> nil ()
    {
        return OopRef<T> ();
    }
};

template <> class OopRef<SmiOopDesc> : public OopRef<OopDesc>
{
  public:
    OopRef () = delete;
    OopRef (OopDesc *) = delete;
    OopRef (SmiOopDesc *);
    OopRef (intptr_t);
    inline intptr_t intValue ();
    inline OopRef<SmiOopDesc> increment ();
    inline OopRef<SmiOopDesc> decrement ();
    SmiOopDesc operator* () const;
    SmiOopDesc * operator-> () const;
};

template <> class OopRef<FloatOopDesc> : public OopRef<OopDesc>
{
  public:
    OopRef () = delete;
    OopRef (OopDesc *) = delete;
    OopRef (float);
    inline float floatValue ();
    FloatOopDesc operator* () const;
    FloatOopDesc * operator-> () const;
};

/* clang-format off */
typedef OopRef <OopDesc> 				Oop;
typedef OopRef  <SmiOopDesc>			SmiOop;
typedef OopRef  <FloatOopDesc> 			FloatOop;
typedef OopRef  <MemOopDesc> 			MemOop;
typedef OopRef   <OopOopDesc> 			OopOop;
typedef OopRef    <ArrayOopDesc>		ArrayOop;
typedef OopRef    <BlockOopDesc>		BlockOop;
typedef OopRef    <CharOopDesc>			CharOop;
typedef OopRef    <ClassOopDesc>		ClassOop;
typedef OopRef    <ContextOopDesc>		ContextOop;
typedef OopRef    <DictionaryOopDesc>	DictionaryOop;
typedef OopRef    <LinkOopDesc> 		LinkOop;
typedef OopRef    <MethodOopDesc>		MethodOop;
typedef OopRef    <ProcessOopDesc>		ProcessOop;
typedef OopRef    <StringOopDesc>		StringOop;
typedef OopRef    <SymbolOopDesc>		SymbolOop;
typedef OopRef   <ByteOopDesc>			ByteOop;
typedef OopRef    <ByteArrayOopDesc>	ByteArrayOop;
/* clang-format on */

class Klass
{
    virtual void print (int in);
};

class OopDesc
{
  protected:
    friend class MemoryManager;
    friend class OopRef<OopDesc>;
    friend class OopRef<SmiOopDesc>;
    friend class OopRef<FloatOopDesc>;

  public:
    /**
     * Oop to this object's class.
     * N.b. only MemOops have this. An isa is always a real pointer, so we use
     * its tag bits specially.
     */
    ClassOop _isa;

    inline bool operator!= (const OopDesc * anOopDesc);
    inline bool operator== (const OopDesc * anOopDesc);

    /**
     * Nil is defined as the NULL pointer.
     */
    inline bool isNil ();
    inline bool isInteger ();

#pragma mark conversions
    ArrayOop asArrayOop ();
    BlockOop asBlockOop ();
    ByteArrayOop asByteArrayOop ();
    CharOop asCharOop ();
    ClassOop asClassOop ();
    DictionaryOop asDictionaryOop ();
    LinkOop asLinkOop ();
    StringOop asStringOop ();
    SymbolOop asSymbolOop ();
    ContextOop asContextOop ();
    MemOop asMemOop ();
    MethodOop asMethodOop ();
    ProcessOop asProcessOop ();
    OopOop asOopOop ();
    Oop asOop ()
    {
        return this;
    }

    inline operator Oop ()
    {
        return this;
    }

    void print (int in);
};

class SmiOopDesc : public OopDesc
{
  public:
    void print (int in);
};

class FloatOopDesc : public OopDesc
{
  public:
    static FloatOop fromFloat (float val);
};

class MemOopDesc : public OopDesc
{
  public:
    size_t _size : 30;
    enum
    {
        kOop,
        kByte,
    } kind : 2;
    /* Space for the object's fields. */
    union {
        uint8_t bytes[0];
        Oop oops[0];
    } _vonNeumannSpace;

    /* Size of this object's von Neumann space */
    inline size_t size ();
};

class OopOopDesc : public MemOopDesc
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

class ByteOopDesc : public MemOopDesc
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

union TagPtr {
    enum Kind
    {
        kOop = 0,
        kInt = 1,
        kFloat = 2,
    };

    struct
    {
        intptr_t num : 62 PACKSTRUCT;
        uintptr_t tag : 2 PACKSTRUCT;
    } i PACKSTRUCT;

    struct
    {
        intptr_t flo : 32 PACKSTRUCT;
        intptr_t padding : 30 PACKSTRUCT;
        uintptr_t tag : 2 PACKSTRUCT;
    } f PACKSTRUCT;

    Oop o PACKSTRUCT;

    TagPtr (Oop anOop) : o (anOop)
    {
    }
    TagPtr (float aFloat)
    {
        f.flo = aFloat;
        f.tag = kFloat;
    }
    TagPtr (intptr_t anInt)
    {
        i.num = anInt;
        i.tag = kInt;
    }
};

#include "Lowlevel/MVEndPackStruct.h"

template <typename T> OopRef<T>::operator OopRef<OopDesc> ()
{
    return OopRef<OopDesc> ((OopDesc *)dat);
}

template <typename T> OopRef<T>::OopRef (OopDesc * obj)
{
    dat = (T *)obj;
}

template <typename T> T * OopRef<T>::addr ()
{
    return dat;
}

/*SmiOopDesc OopRef<SmiOopDesc>::operator* () const
{
    return *dat;
}*/

inline OopRef<SmiOopDesc>::OopRef (SmiOopDesc * smi)
{
    dat = smi;
}

inline SmiOopDesc * OopRef<SmiOopDesc>::operator-> () const
{
    return reinterpret_cast<SmiOopDesc *> (dat);
}

inline OopRef<SmiOopDesc>::OopRef (intptr_t i)
{
    TagPtr tag (i);
    dat = tag.o.addr ();
}

inline OopRef<FloatOopDesc>::OopRef (float f)
{
    TagPtr tag (f);
    dat = tag.o.addr ();
}