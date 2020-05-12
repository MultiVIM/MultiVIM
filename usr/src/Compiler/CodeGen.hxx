#pragma once

#include <string>
#include <vector>

#include "Oops/Oops.hxx"

class CodeGen
{
    std::vector<uint8_t> _bytecode;
    std::vector<Oop> _literals;
    bool _isBlock;

  public:
    bool isBlock ()
    {
        return _isBlock;
    }

    CodeGen (bool isBlock = false) : _isBlock (isBlock)
    {
    }

    std::vector<uint8_t> bytecode ()
    {
        return _bytecode;
    }
    std::vector<Oop> literals ()
    {
        return _literals;
    }

    void genCode (uint8_t code);

    void genInstruction (uint8_t instruction, uint8_t argument = 0);
    void genInteger (int val);
    int genLiteral (Oop aLiteral);

    void genMoveParentHeapVarToMyHeapVars (uint8_t index,
                                           uint8_t promotedIndex);
    void genMoveArgumentToMyHeapVars (uint8_t index, uint8_t promotedIndex);
    void genMoveLocalToMyHeapVars (uint8_t index, uint8_t promotedIndex);

    void genPushInstanceVar (uint8_t index);
    void genPushArgument (uint8_t index);
    void genPushLocal (uint8_t index);
    void genPushParentHeapVar (uint8_t index);
    void genPushMyHeapVar (uint8_t index);

    void genStoreInstanceVar (uint8_t index);
    void genStoreLocal (uint8_t index);
    void genStoreParentHeapVar (uint8_t index);
    void genStoreMyHeapVar (uint8_t index);

    void genMessage (bool isSuper, size_t numArgs, std::string selector);
    void genPushLiteral (uint8_t num);
    void genPushLiteralObject (Oop anObj);
};