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
    kPushSmalltalk,

    /* These take the next byte as their argument - which index of iVar/arg/etc
       to push. - 1-based indexing. */
    kPushInstanceVar,
    kPushArgument,
    kPushLocal,
    kPushLiteral,
    kPushParentHeapVar,
    kPushMyHeapVar,
    kPushBlockCopy,

    /* As above, storing top of stack into the location. (Globals are
       implemented by sending the `value` mess*/
    kStoreInstanceVar,
    kStoreLocal,
    kStoreParentHeapVar,
    kStoreMyHeapVar,
    kStoreGlobal,

    /* Following bytecode is number of arguments (nA);
     * TOS is the selector. TOS - 1 is last arg, TOS -2 is 2nd-to-last arg, etc.
     * TOS - nArgs is receiver.
     */
    kSend,
    kSendSuper,

    /* Following bytecode is index of special binop */
    kBinOp,

    kDuplicate,
    kPop,

    kReturn,
    kBlockReturn,

    /* All below this take two args. */
    kMinTwoArgs,

    /* These take two arguments. They move a variable (index is argAt: 1) to
       myHeapVar slot (index is argAt: 2). */
    kMoveParentHeapVarToMyHeapVars,
    kMoveArgumentToMyHeapVars,
    kMoveLocalToMyHeapVars,
    kMoveMyHeapVarToParentHeapVars,

    kPrimitive,

    kMax,
};

void printBytecode (uint8_t code, uint8_t arg, uint8_t arg2);
void printAllBytecode (ByteArrayOop arr, int indent);

#endif