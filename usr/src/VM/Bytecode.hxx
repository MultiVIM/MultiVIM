#ifndef BYTECODE_HXX__
#define BYTECODE_HXX__

enum Opcode
{
    kNone = 0,

    kPushSelf,
    kPushNil,
    kPushTrue,
    kPushFalse,
    kPushContext,

    /* These take the next byte as their argument - which index of iVar/arg/etc
       to push. - 1-based indexing. */
    kPushInstanceVar,
    kPushArgument,
    kPushLocal,
    kPushLiteral,
    kPushParentHeapVar,
    kPushMyHeapVar,

    /* As above, storing top of stack into the location. (Globals are
       implemented by sending the `value` mess*/
    kStoreInstanceVar,
    kStoreLocal,
    kStoreParentHeapVar,
    kStoreMyHeapVar,

    /* Following bytecode is number of arguments;
     * Top of stack is selector;
     * while (argCount--) *args++ =  topOfStack. I.e. ToS is first arg. */
    kSend,
    kSendSuper,

    kDuplicate,
    kPop,

    kReturn,
    kBlockReturn,

    /* These take two arguments. They move a variable (index is argAt: 1) to
       myHeapVar slot (index is argAt: 2). */
    kMoveParentHeapVarToMyHeapVars,
    kMoveArgumentToMyHeapVars,
    kMoveLocalToMyHeapVars,

    kPrimitive,
};

void printBytecode (ByteArrayOop arr, int indent);

#endif