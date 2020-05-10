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

#define AccessorDef(ClassName, Type, Index, FunName, SetFunName)               \
    inline Type & ClassName::FunName ()                                        \
    {                                                                          \
        return basicAt (Index).as##Type ();                                    \
    }                                                                          \
    inline Type ClassName::SetFunName (Type value)                             \
    {                                                                          \
        return basicatPut (Index, value).as##Type ();                          \
    }

AccessorDef (ClassOop, SymbolOop, 1, name, setName);
AccessorDef (ClassOop, ClassOop, 2, superClass, setSuperClass);
AccessorDef (ClassOop, DictionaryOop, 3, methods, setMethods);
AccessorDef (ClassOop, SmiOop, 4, nstSize, setNstSize);
AccessorDef (ClassOop, ArrayOop, 5, nstVars, setNstVars);

AccessorDef (LinkOop, Oop, 1, one, setOne);
AccessorDef (LinkOop, Oop, 2, two, setTwo);
AccessorDef (LinkOop, LinkOop, 3, nextLink, setNextLink);

inline bool StringOop::strEquals (std::string aString)
{
    return !strcmp ((char *)vonNeumannSpace (), aString.c_str ());
}

#endif