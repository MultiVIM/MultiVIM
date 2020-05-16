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

  public:
    void interpret (ProcessOop proc);
    static void coldBootMainProcessor ();
};

#endif