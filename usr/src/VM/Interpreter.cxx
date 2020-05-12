#include "Interpreter.hxx"
#include "Bytecode.hxx"
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
       implemented by sending the `value` mess*/
    "StoreInstanceVar",
    "StoreLocal",
    "StoreParentHeapVar",
    "StoreMyHeapVar",

    /* Following bytecode is number of arguments;
     * Top of stack is selector;
     * while (argCount--) *args++ =  topOfStack. I.e. ToS is first arg. */
    "Send",
    "SendSuper",

    "Duplicate",
    "Pop",

    "Return",
    "BlockReturn",

    /* These take two arguments. They move a variable (index is argAt: 1) to
       myHeapVar slot (index is argAt: 2). */
    "MoveParentHeapVarToMyHeapVars",
    "MoveArgumentToMyHeapVars",
    "MoveLocalToMyHeapVars",

    "Primitive",
};

void printBytecode (ByteArrayOop arr, int indent)
{
    for (int i = 1; i < arr.size ();)
    {
        uint8_t code = arr.basicAt (i++);
        uint8_t arg = arr.basicAt (i++);
        uint8_t arg2;

        if (code >= kMoveParentHeapVarToMyHeapVars &&
            code <= kMoveLocalToMyHeapVars)
        {
            printf ("%s%-16s(%d,%d)\n",
                    blanks (indent).c_str (),
                    bytecodeNames[code],
                    arg,
                    arr.basicAt (i++));
        }
        else
        {
            printf ("%s%-16s(%d)\n",
                    blanks (indent).c_str (),
                    bytecodeNames[code],
                    arg);
        }
    }
}

void Interpreter::interpret (ProcessOop proc)
{
    while (1)
    {
        uint8_t code = proc.context ().fetchByte ();
        uint8_t arg = proc.context ().fetchByte ();
    }
}