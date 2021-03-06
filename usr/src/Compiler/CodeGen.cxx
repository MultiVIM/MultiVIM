#include "CodeGen.hxx"
#include "VM/Bytecode.hxx"
#include "VM/Interpreter.hxx"

void CodeGen::willPush (int n)
{
    _curStackHeight += n;

    if (_curStackHeight > _highestStackHeight)
        _highestStackHeight = _curStackHeight;
}

void CodeGen::willPop (int n)
{
    _curStackHeight -= n;
}

void CodeGen::genCode (uint8_t code)
{
    _bytecode.push_back (code);
}

void CodeGen::genInstruction (uint8_t opcode, uint8_t arg)
{
    // printBytecode (opcode, arg, 0);
    // printf ("Gen instruction %d %d\n", opcode, arg);
    _bytecode.push_back (opcode);
    _bytecode.push_back (arg);
}

void CodeGen::genPushInteger (int val)
{
    genPushLiteralObject (SmiOop (val));
}

void CodeGen::genPushBlockCopy (BlockOop block)
{
    willPush ();
    genInstruction (Opcode::kPushBlockCopy, genLiteral (block));
}

int CodeGen::genLiteral (Oop aLiteral)
{
    _literals.push_back (aLiteral);
    return _literals.size ();
}

void CodeGen::genDup ()
{
    willPush ();
    genInstruction (Opcode::kDuplicate);
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

void CodeGen::genMoveMyHeapVarToParentHeapVars (uint8_t myIndex,
                                                uint8_t parentIndex)

{
    genInstruction (Opcode::kMoveMyHeapVarToParentHeapVars, myIndex);
    genCode (parentIndex);
}

void CodeGen::genPushInstanceVar (uint8_t index)
{
    willPush ();
    genInstruction (Opcode::kPushInstanceVar, index);
}

void CodeGen::genPushArgument (uint8_t index)
{
    willPush ();
    genInstruction (Opcode::kPushArgument, index);
}

void CodeGen::genPushGlobal (std::string name)
{
    SymbolOop sym = SymbolOopDesc::fromString (name);
    /*genPushLiteralObject (MemoryManager::objGlobals);
    genPushLiteralObject (());
    genMessage (false, 1, "at:");*/
    willPush ();
    genInstruction (Opcode::kPushGlobal, genLiteral (sym));
}

void CodeGen::genPushLocal (uint8_t index)
{
    willPush ();
    genInstruction (Opcode::kPushLocal, index);
}

void CodeGen::genPushParentHeapVar (uint8_t index)
{
    willPush ();
    genInstruction (Opcode::kPushParentHeapVar, index);
}

void CodeGen::genPushMyHeapVar (uint8_t index)
{
    willPush ();
    genInstruction (Opcode::kPushMyHeapVar, index);
}

void CodeGen::genPushSelf ()
{
    willPush ();
    genInstruction (Opcode::kPushSelf);
}

void CodeGen::genPushNil ()
{
    willPush ();
    genInstruction (Opcode::kPushNil);
}

void CodeGen::genPushTrue ()
{
    willPush ();
    genInstruction (Opcode::kPushTrue);
}

void CodeGen::genPushFalse ()
{
    willPush ();
    genInstruction (Opcode::kPushFalse);
}

void CodeGen::genPushSmalltalk ()
{
    willPush ();
    genInstruction (Opcode::kPushSmalltalk);
}

void CodeGen::genPushThisContext ()
{
    willPush ();
    genInstruction (Opcode::kPushContext);
}

void CodeGen::genPop ()
{
    genInstruction (Opcode::kPop);
    willPop ();
}

void CodeGen::genPrimitive (uint8_t primNum, uint8_t nArgs)
{
    genInstruction (Opcode::kPrimitive, primNum);
    genCode (nArgs); /* primitive is 2-arged */
    /* pops all args, but pushes result */
    willPop (nArgs - 1);
}

void CodeGen::genStoreInstanceVar (uint8_t index)
{
    genInstruction (Opcode::kStoreInstanceVar, index);
}

void CodeGen::genStoreGlobal (std::string name)
{
    genInstruction (Opcode::kStoreGlobal,
                    genLiteral (SymbolOopDesc::fromString (name)));
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

void CodeGen::genIfTrueIfFalse ()
{
    /* We use 2 for ifTrue:ifFalse. */
    genInstruction (Opcode::kIfTrueIfFalse, 2);
    willPop (2);
}

void CodeGen::genMessage (bool isSuper, size_t numArgs, std::string selector)
{
    SymbolOop messagesym = SymbolOopDesc::fromString (selector);

    if (!isSuper)
    {
        int sym;
        if ((sym = ProcessorOopDesc::optimisedBinopSym (messagesym)) != -1)
        {
            willPop (numArgs);
            genInstruction (Opcode::kBinOp, sym);
            return;
        }
    }

    genPushLiteralObject (messagesym);

    if (isSuper)
        genInstruction (Opcode::kSendSuper, numArgs);
    else
        genInstruction (Opcode::kSend, numArgs);

    /* pops nArgs + selector + receiver, but pushes result */
    willPop (numArgs + 1);
}

void CodeGen::genPushLiteral (uint8_t num)
{
    willPush ();
    genInstruction (Opcode::kPushLiteral, num);
}

void CodeGen::genPushLiteralObject (Oop obj)
{
    genPushLiteral (genLiteral (obj));
}

void CodeGen::genBlockReturn ()
{
    _blockHasBlockReturn = true;
    genInstruction (Opcode::kBlockReturn);
    willPop ();
}

void CodeGen::genReturn ()
{
    genInstruction (Opcode::kReturn);
    willPop ();
}