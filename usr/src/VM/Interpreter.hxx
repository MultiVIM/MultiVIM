#ifndef INTERPRETER__HXX__
#define INTERPRETER__HXX__

#include "Oops/Oops.hxx"

struct ExecState
{
    ProcessOop proc;
};

typedef Oop PrimitiveMethod (ExecState & es, ArrayOop args);
extern PrimitiveMethod * primVec[];

class Processor
{
    static Processor mainProcessor;
    static SymbolOop binSels[28];

  public:
    void interpret (ProcessOop proc);
    static void coldBootMainProcessor ();
    /* returns -1 if NOT optimised */
    static int optimisedBinopSym (SymbolOop sym);
};

#endif