#ifndef INTERPRETER__HXX__
#define INTERPRETER__HXX__

#include "Oops/Oops.hxx"

class Processor
{
    static Processor mainProcessor;

  public:
    void interpret (ProcessOop proc);
    static void coldBootMainProcessor ();
};

#endif