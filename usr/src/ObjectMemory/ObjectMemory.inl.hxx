#ifndef OBJECTMEMORYINL_HXX_
#define OBJECTMEMORYINL_HXX_

#include <cassert>

#include "ObjectMemory.hxx"

inline bool Oop::operator!= (const Oop & anOop)
{
    return !operator== (anOop);
}

inline bool Oop::operator== (const Oop & anOop)
{
    return (isIntegerFlag == anOop.isIntegerFlag) && (value == anOop.value);
}

inline bool Oop::isNil ()
{
    return !isIntegerFlag && !value;
}

inline size_t Oop::index ()
{
    assert (!isIntegerFlag);
    return value;
}

inline bool Oop::isInteger ()
{
    return isIntegerFlag;
}

inline SmiOop & Oop::asSmiOop ()
{
    // assert (isIntegerFlag);
    return *static_cast<SmiOop *> (this);
}

inline ActualObject * MemoryManager::actualObjectForOop (Oop anOop)
{
    assert (!anOop.isInteger ());
    return _table[anOop.index ()].obj;
}

inline ActualObject * Oop::actualObject ()
{
    return memMgr.actualObjectForOop (*this);
}

inline size_t Oop::size ()
{
    return actualObject ()->size;
}

inline int SmiOop::postInc ()
{
    return value++;
}

inline int SmiOop::preInc ()
{
    return ++value;
}

inline int SmiOop::postDec ()
{
    return value--;
}

inline int SmiOop::preDec ()
{
    return --value;
}

inline Oop * OopOop::vonNeumannSpace ()
{
    return actualObject ()->vonNeumannSpace.oops;
}

inline Oop & OopOop::basicAt (size_t index)
{
    return actualObject ()->vonNeumannSpace.oops[index - 1];
}

inline Oop & OopOop::basicatPut (size_t index, Oop value)
{
    return memMgr.actualObjectForOop (*this)->vonNeumannSpace.oops[index - 1] =
               value;
}

inline uint8_t * ByteOop::vonNeumannSpace ()
{
    return memMgr.actualObjectForOop (*this)->vonNeumannSpace.bytes;
}

inline uint8_t & ByteOop::basicAt (size_t index)
{
    return memMgr.actualObjectForOop (*this)->vonNeumannSpace.bytes[index - 1];
}

inline uint8_t & ByteOop::basicatPut (size_t index, uint8_t value)
{
    return memMgr.actualObjectForOop (*this)->vonNeumannSpace.bytes[index - 1] =
               value;
}

inline ClassOop Oop::isa ()
{
    if (isIntegerFlag)
        return memMgr.clsInteger ();
    return memMgr.actualObjectForOop (*this)->isa;
}

inline ClassOop Oop::setIsa (ClassOop val)
{
    assert (!isIntegerFlag);
    return (memMgr.actualObjectForOop (*this)->isa = val);
}

#endif