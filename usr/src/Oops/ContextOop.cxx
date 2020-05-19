#include "Oops.hxx"

bool ContextOop::isBlockContext ()
{
    return methodOrBlock ().isa () == memMgr.clsBlock ();
}

ContextOop ContextOop::newWithBlock (BlockOop aMethod)
{
    ContextOop ctx = memMgr.allocateOopObj (clsNstLength).asContextOop ();

    ctx.setIsa (memMgr.clsContext ());
    ctx.setBytecode (aMethod.bytecode ());
    ctx.setReceiver (aMethod.receiver ());
    ctx.setMethodOrBlock (aMethod);
    ctx.setArguments (
        ArrayOop::newWithSize (aMethod.argumentCount ().intValue ()));
    ctx.setTemporaries (
        ArrayOop::newWithSize (aMethod.temporarySize ().intValue ()));
    ctx.setHeapVars (
        ArrayOop::newWithSize (aMethod.heapVarsSize ().intValue ()));
    ctx.setParentHeapVars (aMethod.parentHeapVars ());
    ctx.setStack (ArrayOop::newWithSize (aMethod.stackSize ().intValue () +
                                         1 /* nil pushed by execblock prim */));
    ctx.setProgramCounter (SmiOop (1));
    ctx.setStackPointer (SmiOop (0));

    return ctx;
}

ContextOop ContextOop::newWithMethod (Oop receiver, MethodOop aMethod)
{
    ContextOop ctx = memMgr.allocateOopObj (clsNstLength).asContextOop ();

    ctx.setIsa (memMgr.clsContext ());
    ctx.setBytecode (aMethod.bytecode ());
    ctx.setReceiver (receiver);
    ctx.setMethodOrBlock (aMethod);
    ctx.setArguments (
        ArrayOop::newWithSize (aMethod.argumentCount ().intValue ()));
    ctx.setTemporaries (
        ArrayOop::newWithSize (aMethod.temporarySize ().intValue ()));
    ctx.setHeapVars (
        ArrayOop::newWithSize (aMethod.heapVarsSize ().intValue ()));
    ctx.setStack (ArrayOop::newWithSize (aMethod.stackSize ().intValue () +
                                         1 /* nil pushed */));
    ctx.setProgramCounter (SmiOop (1));
    ctx.setStackPointer (SmiOop (0));

    return ctx;
}
