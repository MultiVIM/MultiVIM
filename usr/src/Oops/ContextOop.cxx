#include "Oops.hxx"

ContextOop ContextOop::newWithMethod (Oop receiver, MethodOop aMethod)
{
    ContextOop ctx = memMgr.allocateOopObj (clsNstLength).asContextOop ();

    ctx.setIsa (memMgr.clsContext ());
    ctx.setReceiver (receiver);
    ctx.setMethodOrBlock (aMethod);
    ctx.setTemporaries (
        ArrayOop::newWithSize (aMethod.temporarySize ().intValue ()));
    ctx.setBytecode (aMethod.bytecode ());
    ctx.setProgramCounter (SmiOop (1));

    return ctx;
}
