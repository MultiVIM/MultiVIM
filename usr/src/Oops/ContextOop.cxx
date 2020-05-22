#include "Oops.hxx"

bool ContextOopDesc::isBlockContext ()
{
    return methodOrBlock ().isa () == MemoryManager::clsBlock;
}

ContextOop ContextOopDesc::newWithBlock (BlockOop aMethod)
{
    ContextOop ctx = memMgr.allocateOopObj (clsNstLength)->asContextOop ();

    ctx.setIsa (MemoryManager::clsContext);
    ctx->init ();
    ctx->setBytecode (aMethod->bytecode ());
    ctx->setReceiver (aMethod->receiver ());
    ctx->setMethodOrBlock (aMethod->asOopOop ());
    ctx->setArguments (
        ArrayOopDesc::newWithSize (aMethod->argumentCount ().intValue ()));
    ctx->setTemporaries (
        ArrayOopDesc::newWithSize (aMethod->temporarySize ().intValue ()));
    ctx->setHeapVars (
        ArrayOopDesc::newWithSize (aMethod->heapVarsSize ().intValue ()));
    ctx->setParentHeapVars (aMethod->parentHeapVars ());
    ctx->setStack (
        ArrayOopDesc::newWithSize (aMethod->stackSize ().intValue () +
                                   1 /* nil pushed by execblock prim */));
    ctx->setProgramCounter (SmiOop ((intptr_t)0));
    ctx->setStackPointer (SmiOop ((intptr_t)0));

    return ctx;
}

ContextOop ContextOopDesc::newWithMethod (Oop receiver, MethodOop aMethod)
{
    ContextOop ctx = memMgr.allocateOopObj (clsNstLength)->asContextOop ();

    ctx.setIsa (MemoryManager::clsContext);
    ctx->setBytecode (aMethod->bytecode ());
    ctx->setReceiver (receiver);
    ctx->setMethodOrBlock (aMethod->asOopOop ());
    ctx->setArguments (
        ArrayOopDesc::newWithSize (aMethod->argumentCount ().intValue ()));
    ctx->setTemporaries (
        ArrayOopDesc::newWithSize (aMethod->temporarySize ().intValue ()));
    ctx->setHeapVars (
        ArrayOopDesc::newWithSize (aMethod->heapVarsSize ().intValue ()));
    ctx->setStack (ArrayOopDesc::newWithSize (
        aMethod->stackSize ().intValue () + 1 /* nil pushed */));
    ctx->setProgramCounter (SmiOop ((intptr_t)0));
    ctx->setStackPointer (SmiOop ((intptr_t)0));

    return ctx;
}
