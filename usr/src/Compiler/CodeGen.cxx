#include "CodeGen.hxx"
#include "VM/Bytecode.hxx"

void CodeGen::genCode (uint8_t code)
{
    _bytecode.push_back (code);
}

void CodeGen::genInstruction (uint8_t opcode, uint8_t arg)
{
    printf ("Gen instruction %d %d\n", opcode, arg);
    _bytecode.push_back (opcode);
    _bytecode.push_back (arg);
}

void CodeGen::genInteger (int val)
{
    CodeGen::genPushLiteral (CodeGen::genLiteral (SmiOop (val)));
}

int CodeGen::genLiteral (Oop aLiteral)
{
    _literals.push_back (aLiteral);
    return _literals.size ();
}

void CodeGen::genMoveParentHeapVarToMyHeapVars (uint8_t index,
                                                uint8_t promotedIndex)
{
    genInstruction (Opcode::kMoveParentHeapVarToMyHeapVars, index);
    genCode (promotedIndex);
}

void CodeGen::genMoveArgumentToMyHeapVars (uint8_t index, uint8_t promotedIndex)
{
    genInstruction (Opcode::kMoveArgumentToMyHeapVars, index);
    genCode (promotedIndex);
}

void CodeGen::genMoveLocalToMyHeapVars (uint8_t index, uint8_t promotedIndex)
{
    genInstruction (Opcode::kMoveLocalToMyHeapVars, index);
    genCode (promotedIndex);
}

void CodeGen::genPushInstanceVar (uint8_t index)
{
    genInstruction (Opcode::kPushInstanceVar, index);
}

void CodeGen::genPushArgument (uint8_t index)
{
    genInstruction (Opcode::kPushArgument, index);
}

void CodeGen::genPushLocal (uint8_t index)
{
    genInstruction (Opcode::kPushLocal, index);
}

void CodeGen::genPushParentHeapVar (uint8_t index)
{
    genInstruction (Opcode::kPushParentHeapVar, index);
}

void CodeGen::genPushMyHeapVar (uint8_t index)
{
    genInstruction (Opcode::kPushMyHeapVar, index);
}

void CodeGen::genStoreInstanceVar (uint8_t index)
{
    genInstruction (Opcode::kStoreInstanceVar, index);
}

void CodeGen::genStoreLocal (uint8_t index)
{
    genInstruction (Opcode::kStoreLocal, index);
}

void CodeGen::genStoreParentHeapVar (uint8_t index)
{
    genInstruction (Opcode::kStoreParentHeapVar, index);
}

void CodeGen::genStoreMyHeapVar (uint8_t index)
{
    genInstruction (Opcode::kStoreMyHeapVar, index);
}

void CodeGen::genMessage (bool isSuper, size_t numArgs, std::string selector)
{
    SymbolOop messagesym = SymbolOop::fromString (selector);

    genInstruction (Opcode::kPushLiteral, genLiteral (messagesym));
    if (isSuper)
        genInstruction (Opcode::kSendSuper, numArgs);
    else
        genInstruction (Opcode::kSend, numArgs);
}

void CodeGen::genPushLiteral (uint8_t num)
{
    genInstruction (Opcode::kPushLiteral, num);
}

void CodeGen::genPushLiteralObject (Oop obj)
{
    genPushLiteral (genLiteral (obj));
}