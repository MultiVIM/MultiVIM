#ifndef INTERPRETER__HXX__
#define INTERPRETER__HXX__

#include "Oops/Oops.hxx"

typedef Oop PrimitiveMethod (ProcessOop proc, ArrayOop args);
extern PrimitiveMethod * primVec[];

class ProcessorOopDesc : public OopOopDesc
{
    static const int clsNstLength = 3;
    /* Multipled by four. */
    static const int cacheSize = 221;
    static ProcessorOop mainProcessor;
    static SymbolOop binSels[28];

    MethodOop lookupMethodInClass (ProcessOop proc, Oop receiver, ClassOop cls,
                                   SymbolOop selector, bool super);
    MethodOop lookupMethodInCache (int hash, SymbolOop selector,
                                   ClassOop receiverClass);
    void setMethodInCache (int hash, SymbolOop selector, ClassOop receiverClass,
                           MethodOop meth);
    void doSend (ProcessOop proc, Oop receiver, SymbolOop selector,
                 ArrayOop arguments, bool toSuper);
    void opSend (ProcessOop proc, int numArgs, bool toSuper);

  public:
    DeclareAccessorPair (SymbolOop, name, setName);
    DeclareAccessorPair (ProcessOop, process, setProcess);
    /* An array of {Symbol selector, Class receiverClass, Method meth} */
    DeclareAccessorPair (ArrayOop, cache, setCache);

    Oop runUntilCompletion (std::string code);
    void interpret ();
    static void coldBootMainProcessor ();

    static ProcessorOop allocate ();

    /* returns -1 if NOT optimised */
    static int optimisedBinopSym (SymbolOop sym);
};

#define accessorsFor ProcessorOopDesc
AccessorDef (SymbolOop, 1, name, setName);
AccessorDef (ProcessOop, 2, process, setProcess);
AccessorDef (ArrayOop, 3, cache, setCache);
#undef accessorsFor

#endif