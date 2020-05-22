#ifndef OBJECTMEMORYINL_HXX_
#define OBJECTMEMORYINL_HXX_

#include <cassert>

#include "ObjectMemory.hxx"

/*template <typename T> bool OopRef<T>::isNil ()
{
    return dat == 0 || this == MemoryManager::objNil;
}

template <typename T> bool OopRef<T>::isInteger ()
{
    return TagPtr (dat).i.tag == tagPtr::kInt;
}*/

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
    return (dat->_isa = val);
}

template <typename T> SmiOop OopRef<T>::asSmiOop ()
{
    if (TagPtr (dat).i.tag == TagPtr::kInt)
        return SmiOop (TagPtr (dat).i.num);
    else
        return SmiOop ((intptr_t)dat);
}

template <typename T> FloatOop OopRef<T>::asFloatOop ()
{
    if (TagPtr (dat).f.tag == TagPtr::kFloat)
        return FloatOop (TagPtr (dat).f.flo);
    else
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

template <typename T> bool OopRef<T>::isNil ()
{
    return dat == NULL;
}

template <typename T> bool OopRef<T>::isInteger ()
{
    return TagPtr (dat).i.tag == TagPtr::kInt;
}

inline size_t MemOopDesc::size ()
{
    return _size;
}

inline intptr_t SmiOop::intValue ()
{
    return TagPtr (dat).i.num;
}

inline float FloatOop::floatValue ()
{
    return TagPtr (dat).f.flo;
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
    return _vonNeumannSpace.oops;
}

inline Oop & OopOopDesc::basicAt (size_t index)
{
    return _vonNeumannSpace.oops[index - 1];
}

inline Oop & OopOopDesc::basicatPut (size_t index, Oop value)
{
    return _vonNeumannSpace.oops[index - 1] = value;
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

#endif