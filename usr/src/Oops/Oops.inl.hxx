#ifndef OOP_INL_HXX_
#define OOP_INL_HXX_

#include <cstring>

#include "Oops.hxx"

#define CastFun(To)                                                            \
    inline To & Oop::as##To ()                                                 \
    {                                                                          \
        return *static_cast<To *> (this);                                      \
    }

CastFun (ArrayOop);
CastFun (ClassOop);
CastFun (DictionaryOop);
CastFun (LinkOop);
CastFun (SymbolOop);
CastFun (StringOop);

#define Accessor(ClassName, Type, FunName, Index)                              \
    inline Type & ClassName::FunName ()                                        \
    {                                                                          \
        return basicAt (Index).as##Type ();                                    \
    }
/*                                                                       \
inline Type & ClassName::set##FunName (Type & value)                           \
{                                                                              \
return basicatPut (Index, value);                                              \
}*/

Accessor (ClassOop, SymbolOop, name, 1);
Accessor (ClassOop, ClassOop, superClass, 1);
Accessor (ClassOop, DictionaryOop, methods, 1);
Accessor (ClassOop, SmiOop, nstSize, 1);
Accessor (ClassOop, ArrayOop, nstVars, 1);

inline Oop & LinkOop::atOne ()
{
    return basicAt (1);
}

inline Oop LinkOop::atOnePut (Oop aVal)
{
    return basicatPut (1, aVal);
}

inline Oop & LinkOop::atTwo ()
{
    return basicAt (2);
}

inline Oop LinkOop::atTwoPut (Oop aVal)
{
    return basicatPut (2, aVal);
}

inline LinkOop & LinkOop::nextLink ()
{
    return basicAt (3).asLinkOop ();
}

inline LinkOop LinkOop::nextLinkPut (LinkOop aVal)
{
    return basicatPut (3, aVal).asLinkOop ();
}

inline bool StringOop::strEquals (std::string aString)
{
    return !strcmp ((char *)vonNeumannSpace (), aString.c_str ());
}

inline void StringOop::setString (std::string aString)
{
    assert (0);
}

#endif