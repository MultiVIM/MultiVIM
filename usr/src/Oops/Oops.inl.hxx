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
CastFun (BlockOop);
CastFun (ByteArrayOop);
CastFun (ClassOop);
CastFun (ContextOop);
CastFun (DictionaryOop);
CastFun (LinkOop);
CastFun (MethodOop);
CastFun (OopOop);
CastFun (ProcessOop);
CastFun (StringOop);
CastFun (SymbolOop);

#define AccessorDef(Type, Index, FunName, SetFunName)                          \
    inline Type & accessorsFor::FunName ()                                     \
    {                                                                          \
        return basicAt (Index).as##Type ();                                    \
    }                                                                          \
    inline Type accessorsFor::SetFunName (Type value)                          \
    {                                                                          \
        return basicatPut (Index, value).as##Type ();                          \
    }

#define accessorsFor ClassOop
inline SymbolOop & accessorsFor::name ()
{
    assert (!isNil ());

    return basicAt (1).asSymbolOop ();
}
inline SymbolOop accessorsFor::setName (SymbolOop value)
{
    assert (!isNil ());
    return basicatPut (1, value).asSymbolOop ();
}

// AccessorDef (SymbolOop, 1, name, setName);
AccessorDef (ClassOop, 2, superClass, setSuperClass);
AccessorDef (DictionaryOop, 3, methods, setMethods);
AccessorDef (SmiOop, 4, nstSize, setNstSize);
AccessorDef (ArrayOop, 5, nstVars, setNstVars);
#undef accessorsFor

#define accessorsFor LinkOop
AccessorDef (Oop, 1, one, setOne);
AccessorDef (Oop, 2, two, setTwo);
AccessorDef (LinkOop, 3, nextLink, setNextLink);
#undef accessorsFor

inline bool StringOop::strEquals (std::string aString)
{
    return !strcmp ((char *)vonNeumannSpace (), aString.c_str ());
}

inline std::string StringOop::asString ()
{
    return std::string ((char *)vonNeumannSpace ());
}

#define accessorsFor MethodOop
AccessorDef (ByteArrayOop, 1, bytecode, setBytecode);
AccessorDef (ArrayOop, 2, literals, setLiterals);
AccessorDef (StringOop, 3, sourceText, setSourceText);
AccessorDef (SymbolOop, 4, selector, setSelector);
AccessorDef (SmiOop, 5, stackSize, setStackSize);
AccessorDef (SmiOop, 6, temporarySize, setTemporarySize);
AccessorDef (ClassOop, 7, methodClass, setMethodClass);
AccessorDef (SmiOop, 8, watch, setWatch);
#undef accessorsFor

#define accessorsFor BlockOop
AccessorDef (ByteArrayOop, 1, bytecode, setBytecode);
AccessorDef (ArrayOop, 2, literals, setLiterals);
AccessorDef (StringOop, 3, sourceText, setSourceText);
AccessorDef (SymbolOop, 4, selector, setSelector);
AccessorDef (SmiOop, 5, stackSize, setStackSize);
AccessorDef (SmiOop, 6, temporarySize, setTemporarySize);
AccessorDef (Oop, 7, receiver, setReceiver);
AccessorDef (SmiOop, 8, argumentCount, setArgumentCount);
#undef accessorsFor

#define accessorsFor ContextOop
AccessorDef (ContextOop, 1, previousContext, setPreviousContext);
AccessorDef (SmiOop, 2, programCounter, setProgramCounter);
AccessorDef (Oop, 3, receiver, setReceiver);
AccessorDef (ArrayOop, 4, arguments, setArguments);
AccessorDef (ArrayOop, 5, temporaries, setTemporaries);
AccessorDef (ByteArrayOop, 6, bytecode, setBytecode);
AccessorDef (OopOop, 7, methodOrBlock, setMethodOrBlock);
#undef accessorsFor

#define accessorsFor ProcessOop
AccessorDef (ContextOop, 1, context, setContext);
#undef accessorsFor

inline uint8_t ContextOop::fetchByte ()
{
    int pos = programCounter ().postInc ();
    return bytecode ().basicAt (pos);
}

#endif