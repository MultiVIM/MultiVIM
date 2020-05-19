#pragma once

#include <stack>
#include <string>
#include <vector>

#include "Oops/Oops.hxx"

class AbstractScope;

class CodeGen
{
    int _curStackHeight, _highestStackHeight;
    std::vector<uint8_t> _bytecode;
    std::vector<Oop> _literals;
    bool _isBlock;
    std::stack<AbstractScope *> _currentScope;

    void willPush (int n = 1);
    void willPop (int n = 1);

  public:
    bool isBlock ()
    {
        return _isBlock;
    }

    CodeGen (bool isBlock = false)
        : _isBlock (isBlock), _curStackHeight (0), _highestStackHeight (0)
    {
    }

    std::vector<uint8_t> bytecode ()
    {
        if (_curStackHeight != 0)
            printf ("!!!!!Cur stack height: %d, max %d, isBlock "
                    "%d\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
                    _curStackHeight,
                    _highestStackHeight,
                    _isBlock);
        return _bytecode;
    }
    std::vector<Oop> literals ()
    {
        return _literals;
    }
    AbstractScope * currentScope ()
    {
        return _currentScope.top ();
    }
    void pushCurrentScope (AbstractScope * aScope)
    {
        _currentScope.push (aScope);
    }
    void popCurrentScope ()
    {
        _currentScope.pop ();
    }

    void genCode (uint8_t code);

    void genInstruction (uint8_t instruction, uint8_t argument = 0);
    int genLiteral (Oop aLiteral);

    void genDup ();

    void genMoveParentHeapVarToMyHeapVars (uint8_t index,
                                           uint8_t promotedIndex);
    void genMoveArgumentToMyHeapVars (uint8_t index, uint8_t promotedIndex);
    void genMoveLocalToMyHeapVars (uint8_t index, uint8_t promotedIndex);
    void genMoveMyHeapVarToParentHeapVars (uint8_t myIndex,
                                           uint8_t parentIndex);

    void genPushArgument (uint8_t index);
    void genPushGlobal (std::string name);
    void genPushInstanceVar (uint8_t index);
    void genPushLocal (uint8_t index);
    void genPushParentHeapVar (uint8_t index);
    void genPushMyHeapVar (uint8_t index);
    void genPushSelf ();
    void genPushNil ();
    void genPushTrue ();
    void genPushFalse ();
    void genPushSmalltalk ();
    void genPushLiteral (uint8_t num);
    void genPushLiteralObject (Oop anObj);
    void genPushInteger (int val);
    void genPushBlockCopy (BlockOop block);

    void genPop ();

    void genPrimitive (uint8_t primNum, uint8_t nArgs);

    void genStoreInstanceVar (uint8_t index);
    void genStoreGlobal (std::string name);
    void genStoreLocal (uint8_t index);
    void genStoreParentHeapVar (uint8_t index);
    void genStoreMyHeapVar (uint8_t index);

    void genMessage (bool isSuper, size_t numArgs, std::string selector);

    void genBlockReturn ();
    void genReturn ();

    int maxStackSize ()
    {
        return _highestStackHeight;
    }
};