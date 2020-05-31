#ifndef OBJECTMEMORYINL_HXX_
#define OBJECTMEMORYINL_HXX_

#include <cassert>

#include "ObjectMemory.hxx"

template <typename T> inline ClassOop OopRef<T>::isa ()
{
    if (isInteger ())
        return MemoryManager::clsInteger;
    else if (isNil ())
        return MemoryManager::clsUndefinedObject;
    return dat->_isa;
}

template <typename T> inline ClassOop OopRef<T>::setIsa (ClassOop val)
{
    assert (!isInteger ());
    return ClassOop (dat->_isa = (VT_Oop)val);
}

template <typename T> inline SmiOop OopRef<T>::asSmiOop ()
{
    if (!isInteger ())
        return SmiOop ((intptr_t)dat);
    else
        return *reinterpret_cast<SmiOop *> (this);
}

template <typename T> inline FloatOop OopRef<T>::asFloatOop ()
{
    /*   if (TagPtr (dat).f.tag == TagPtr::kFloat)
           return FloatOop (TagPtr (dat).f.flo);
       else*/
    return FloatOop ((intptr_t)dat);
}

inline bool OopDesc::operator!= (const OopDesc * anOop)
{
    return !operator== (anOop);
}

inline bool OopDesc::operator== (const OopDesc * anOop)
{
    return this == anOop;
}

template <typename T> inline bool OopRef<T>::isNil ()
{
    return dat == NULL;
}

template <typename T> inline bool OopRef<T>::isInteger ()
{
    return VT_isSmallInteger (dat);
}

inline size_t MemOopDesc::size ()
{
    return _size;
}

inline intptr_t SmiOop::intValue ()
{
    return Oop_intValue (dat);
}

inline float FloatOop::floatValue ()
{
    return 0; /// TagPtr (dat).f.flo;
}

inline SmiOop SmiOop::increment ()
{
    return SmiOop (intValue () + 1);
}

inline SmiOop OopRef<SmiOopDesc>::decrement ()
{
    return SmiOop (intValue () - 1);
}
inline Oop * OopOopDesc::vonNeumannSpace ()
{
    return (Oop *)_vonNeumannSpace.oops;
}

inline Oop & OopOopDesc::basicAt (size_t index)
{
    return (Oop &)_vonNeumannSpace.oops[index - 1];
}

inline Oop & OopOopDesc::basicatPut (size_t index, Oop value)
{
    return (Oop &)_vonNeumannSpace.oops[index - 1] = value;
}

inline uint8_t * ByteOopDesc::vonNeumannSpace ()
{
    return _vonNeumannSpace.bytes;
}

inline uint8_t & ByteOopDesc::basicAt (size_t index)
{
    return _vonNeumannSpace.bytes[index - 1];
}

inline uint8_t & ByteOopDesc::basicatPut (size_t index, uint8_t value)
{
    return _vonNeumannSpace.bytes[index - 1] = value;
}

inline ClassOop ClassOopDesc::isa ()
{
    return ClassOop (_isa);
}

inline ClassOop ClassOopDesc::setIsa (ClassOop val)
{
    return ClassOop (_isa = val);
}

#endif