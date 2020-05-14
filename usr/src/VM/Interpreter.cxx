#include "Interpreter.hxx"
#include "Bytecode.hxx"
#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "Lowlevel/MVPrinting.hxx"
#include "Oops/Oops.hxx"

static const char * bytecodeNames[] = {
    "<no-opcode>",
    "PushSelf",
    "PushNil",
    "PushTrue",
    "PushFalse",
    "PushContext",

    /* These take the next byte as their argument - which index of iVar/arg/etc
       to push. - 1-based indexing. */
    "PushInstanceVar",
    "PushArgument",
    "PushLocal",
    "PushLiteral",
    "PushParentHeapVar",
    "PushMyHeapVar",

    /* As above", storing top of stack into the location. (Globals are
       implemented by sending the `value` message; TOS is what to store, arg is
       number of literal denoting name of global.)*/
    "StoreInstanceVar",
    "StoreLocal",
    "StoreParentHeapVar",
    "StoreMyHeapVar",
    "StoreGlobal",

    /* Following bytecode is number of arguments;
     * Top of stack is selector;
     * while (argCount--) *args++ =  topOfStack. I.e. ToS is first arg. */
    "Send",
    "SendSuper",

    "Duplicate",
    "Pop",

    "Return",
    "BlockReturn",

    "<minTwoArgs>"

    /* These take two arguments. They move a variable (index is argAt: 1) to
       myHeapVar slot (index is argAt: 2). */
    "MoveParentHeapVarToMyHeapVars",
    "MoveArgumentToMyHeapVars",
    "MoveLocalToMyHeapVars",

    "Primitive",

    "<maximum>",
};

void printBytecode (uint8_t code, uint8_t arg, uint8_t arg2)
{
    if (code >= kMinTwoArgs && code <= kMax)
    {
        printf ("%-16s(%d,%d)\n", bytecodeNames[code], arg, arg2);
    }
    else
    {
        printf ("%-16s(%d)\n", bytecodeNames[code], arg);
    }
}

void printAllBytecode (ByteArrayOop arr, int indent)
{
    for (int i = 1; i < arr.size ();)
    {
        uint8_t code = arr.basicAt (i++);
        uint8_t arg = arr.basicAt (i++);

        if (code >= kMinTwoArgs && code <= kMax)
        {
            printBytecode (code, arg, arr.basicAt (i++));
        }
        else
        {
            printBytecode (code, arg, 0);
        }
    }
}

static MethodOop lookupMethod (Oop object, SymbolOop selector, bool super)
{
    ClassOop lookupClass = super ? object.isa ().superClass () : object.isa ();
    MethodOop meth =
        lookupClass.methods ().symbolLookup (selector).asMethodOop ();

    if (meth.isNil ())
    {
        printf ("Failed to find method %s in class %s\n",
                selector.asString ().c_str (),
                object.isa ().name ().asString ().c_str ());
    }

    return meth;
}

struct ExecState
{
    ProcessOop proc;
};

void opSend (ExecState & es, bool toSuper)
{
    // SymbolOop selector = es.proc.context ().stack ().pop ();
}

void Processor::interpret (ProcessOop proc)
{
    ExecState es;
    uint8_t opcode;
    bool shouldRun;

    // proc.context ().methodOrBlock ().print (5);

    while (opcode = proc.context ().fetchByte ())
    {
        uint8_t arg = proc.context ().fetchByte ();
        uint8_t arg2;

        if (opcode > kMinTwoArgs && opcode < kMax)
            arg2 = proc.context ().fetchByte ();

        printBytecode (opcode, arg, arg2);

        switch (opcode)
        {
        case kSend:
            opSend (es, false);
        case kSendSuper:
            break;
        }
    }
}

void Processor::coldBootMainProcessor ()
{
    Processor mainProc;
    MethodOop start =
        MVST_Parser::parseText ("1 coldBoot; coldBoot; coldBoot; coldBoot")
            ->synthInClassScope (nullptr)
            ->generate ();
    ProcessOop firstProcess = ProcessOop::allocate ();
    firstProcess.setContext (
        ContextOop::newWithMethod (memMgr.objNil (), start));

    mainProc.interpret (firstProcess);
}