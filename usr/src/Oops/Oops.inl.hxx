#ifndef OOP_INL_HXX_
#define OOP_INL_HXX_

#include <cstring>

#include "Oops.hxx"

#define CastFun(To)                                                            \
    inline To OopDesc::as##To ()                                               \
    {                                                                          \
        return To (this);                                                      \
    }

CastFun (ArrayOop);
CastFun (BlockOop);
CastFun (ByteArrayOop);
CastFun (CharOop);
CastFun (ClassOop);
CastFun (ContextOop);
CastFun (DictionaryOop);
CastFun (LinkOop);
CastFun (MemOop);
CastFun (MethodOop);
CastFun (OopOop);
CastFun (ProcessOop);
CastFun (StringOop);
CastFun (SymbolOop);

#define SmiAccessorDef(Index, FunName, SetFunName)                             \
    inline SmiOop accessorsFor::FunName ()                                     \
    {                                                                          \
        return basicAt (Index).asSmiOop ();                                    \
    }                                                                          \
    inline SmiOop accessorsFor::SetFunName (SmiOop value)                      \
    {                                                                          \
        return basicatPut (Index, value).asSmiOop ();                          \
    }

#define AccessorDef(Type, Index, FunName, SetFunName)                          \
    inline Type accessorsFor::FunName ()                                       \
    {                                                                          \
        return basicAt (Index)->as##Type ();                                   \
    }                                                                          \
    inline Type accessorsFor::SetFunName (Type value)                          \
    {                                                                          \
        return basicatPut (Index, value)->as##Type ();                         \
    }

#define accessorsFor CharOopDesc
SmiAccessorDef (1, value, setValue);
#undef accessorsFor

#define accessorsFor ClassOopDesc
inline SymbolOop accessorsFor::name ()
{
    return basicAt (1)->asSymbolOop ();
}
inline SymbolOop accessorsFor::setName (SymbolOop value)
{
    return basicatPut (1, value)->asSymbolOop ();
}

AccessorDef (ClassOop, 2, superClass, setSuperClass);
AccessorDef (DictionaryOop, 3, methods, setMethods);
SmiAccessorDef (4, nstSize, setNstSize);
AccessorDef (ArrayOop, 5, nstVars, setNstVars);
#undef accessorsFor

#define accessorsFor LinkOopDesc
AccessorDef (Oop, 1, one, setOne);
AccessorDef (Oop, 2, two, setTwo);
AccessorDef (LinkOop, 3, nextLink, setNextLink);
#undef accessorsFor

inline bool StringOopDesc::strEquals (std::string aString)
{
    return !strcmp ((char *)vonNeumannSpace (), aString.c_str ());
}

inline const char * StringOopDesc::asCStr ()
{
    return (const char *)vonNeumannSpace ();
}

inline std::string StringOopDesc::asString ()
{
    return std::string ((char *)vonNeumannSpace ());
}

#define accessorsFor MethodOopDesc
AccessorDef (ByteArrayOop, 1, bytecode, setBytecode);
AccessorDef (ArrayOop, 2, literals, setLiterals);
SmiAccessorDef (3, argumentCount, setArgumentCount);
SmiAccessorDef (4, heapVarsSize, setHeapVarsSize);
SmiAccessorDef (5, temporarySize, setTemporarySize);
SmiAccessorDef (6, stackSize, setStackSize);
AccessorDef (StringOop, 7, sourceText, setSourceText);
AccessorDef (SymbolOop, 8, selector, setSelector);
AccessorDef (ClassOop, 9, methodClass, setMethodClass);
SmiAccessorDef (10, watch, setWatch);
#undef accessorsFor

#define accessorsFor BlockOopDesc
AccessorDef (ByteArrayOop, 1, bytecode, setBytecode);
AccessorDef (ArrayOop, 2, literals, setLiterals);
SmiAccessorDef (3, argumentCount, setArgumentCount);
SmiAccessorDef (4, heapVarsSize, setHeapVarsSize);
SmiAccessorDef (5, temporarySize, setTemporarySize);
SmiAccessorDef (6, stackSize, setStackSize);
AccessorDef (StringOop, 7, sourceText, setSourceText);
AccessorDef (Oop, 8, receiver, setReceiver);
AccessorDef (ArrayOop, 9, parentHeapVars, setParentHeapVars);
AccessorDef (ContextOop, 10, homeMethodContext, setHomeMethodContext);
#undef accessorsFor

#define accessorsFor ContextOopDesc
AccessorDef (ContextOop, 1, previousContext, setPreviousContext);
SmiAccessorDef (2, programCounter, setProgramCounter);
SmiAccessorDef (3, stackPointer, setStackPointer);
AccessorDef (Oop, 4, receiver, setReceiver);
AccessorDef (ArrayOop, 5, arguments, setArguments);
AccessorDef (ArrayOop, 6, temporaries, setTemporaries);
AccessorDef (ArrayOop, 7, heapVars, setHeapVars);
AccessorDef (ArrayOop, 8, parentHeapVars, setParentHeapVars);
AccessorDef (ArrayOop, 9, stack, setStack);
AccessorDef (ByteArrayOop, 10, bytecode, setBytecode);
AccessorDef (OopOop, 11, methodOrBlock, setMethodOrBlock);
AccessorDef (ContextOop, 12, homeMethodContext, setHomeMethodContext);
#undef accessorsFor

#define accessorsFor ProcessOopDesc
AccessorDef (ContextOop, 1, context, setContext);
#undef accessorsFor

inline void ContextOopDesc::init ()
{
    setProgramCounter ((intptr_t)0);
    setStackPointer ((intptr_t)0);
}

inline uint8_t ContextOopDesc::fetchByte ()
{
    int pos;
    pos = setProgramCounter (programCounter ().increment ()).intValue ();
    return bytecode ()->basicAt (pos);
}

inline void ContextOopDesc::dup ()
{
    int oldSP = stackPointer ().intValue ();
    int newSP = setStackPointer (stackPointer ().increment ()).intValue ();
    stack ()->basicatPut (newSP, stack ()->basicAt (oldSP));
}

inline void ContextOopDesc::push (Oop obj)
{
    int newSP = setStackPointer (stackPointer ().increment ()).intValue ();
    stack ()->basicatPut (newSP, obj);
}

inline Oop ContextOopDesc::pop ()
{
    Oop result = stack ()->basicAt (stackPointer ().intValue ());
    int newSP = setStackPointer (stackPointer ().decrement ()).intValue ();
    // assert (newSP);
    return result;
}

inline Oop ContextOopDesc::top ()
{
    return stack ()->basicAt (stackPointer ().intValue ());
}

#endif