#ifndef OOP_INL_HXX_
#define OOP_INL_HXX_

#include <cstring>

#include "Oops.hxx"

inline double FloatOop::floatValue ()
{
    return *(double *)vonNeumannSpace ();
}

inline FloatOop FloatOop::fromDouble (double value)
{
    FloatOop newFloat = memMgr.allocateByteObj (sizeof (double)).asFloatOop ();
    *(double *)newFloat.vonNeumannSpace () = value;
    return newFloat;
}

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
CastFun (FloatOop);
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
AccessorDef (SmiOop, 3, argumentCount, setArgumentCount);
AccessorDef (SmiOop, 4, heapVarsSize, setHeapVarsSize);
AccessorDef (SmiOop, 5, temporarySize, setTemporarySize);
AccessorDef (SmiOop, 6, stackSize, setStackSize);
AccessorDef (StringOop, 7, sourceText, setSourceText);
AccessorDef (SymbolOop, 8, selector, setSelector);
AccessorDef (ClassOop, 9, methodClass, setMethodClass);
AccessorDef (SmiOop, 10, watch, setWatch);
#undef accessorsFor

#define accessorsFor BlockOop
AccessorDef (ByteArrayOop, 1, bytecode, setBytecode);
AccessorDef (ArrayOop, 2, literals, setLiterals);
AccessorDef (SmiOop, 3, argumentCount, setArgumentCount);
AccessorDef (SmiOop, 4, heapVarsSize, setHeapVarsSize);
AccessorDef (SmiOop, 5, temporarySize, setTemporarySize);
AccessorDef (SmiOop, 6, stackSize, setStackSize);
AccessorDef (StringOop, 7, sourceText, setSourceText);
AccessorDef (Oop, 8, receiver, setReceiver);
AccessorDef (ArrayOop, 9, parentHeapVars, setParentHeapVars);
#undef accessorsFor

#define accessorsFor ContextOop
AccessorDef (ContextOop, 1, previousContext, setPreviousContext);
AccessorDef (SmiOop, 2, programCounter, setProgramCounter);
AccessorDef (SmiOop, 3, stackPointer, setStackPointer);
AccessorDef (Oop, 4, receiver, setReceiver);
AccessorDef (ArrayOop, 5, arguments, setArguments);
AccessorDef (ArrayOop, 6, temporaries, setTemporaries);
AccessorDef (ArrayOop, 7, heapVars, setHeapVars);
AccessorDef (ArrayOop, 8, parentHeapVars, setParentHeapVars);
AccessorDef (ArrayOop, 9, stack, setStack);
AccessorDef (ByteArrayOop, 10, bytecode, setBytecode);
AccessorDef (OopOop, 11, methodOrBlock, setMethodOrBlock);
#undef accessorsFor

#define accessorsFor ProcessOop
AccessorDef (ContextOop, 1, context, setContext);
#undef accessorsFor

inline uint8_t ContextOop::fetchByte ()
{
    int pos = programCounter ().postInc ();
    return bytecode ().basicAt (pos);
}

inline void ContextOop::dup ()
{
    int oldVal = stackPointer ().intValue ();
    stack ().basicatPut (stackPointer ().preInc (), stack ().basicAt (oldVal));
}

inline void ContextOop::push (Oop obj)
{
    printf ("StackPointer: %d\n", stackPointer ().preInc ());
    stack ().basicatPut (stackPointer ().intValue (), obj);
}

inline Oop ContextOop::pop ()
{
    assert (stackPointer ().intValue ());
    return stack ().basicAt (stackPointer ().postDec ());
}

inline Oop ContextOop::top ()
{
    return stack ().basicAt (stackPointer ().intValue ());
}

#endif