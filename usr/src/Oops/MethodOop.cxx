#include <iostream>

#include "Lowlevel/MVPrinting.hxx"
#include "Oops.hxx"
#include "VM/Bytecode.hxx"

void MethodOop::print (int in)
{
    std::cout << blanks (in) + "Method\n" << blanks (in) << "{\n";
    in += 1;
    std::cout << blanks (in) + "Bytecode:\n";
    printBytecode (bytecode (), in + 1);
    std::cout << blanks (in) + "Literals:\n";
    for (int i = 1; i <= literals ().size (); i++)
    {
        std::cout << blanks (in) + "Literal " << i << ":\n";
        literals ().basicAt (i).print (i + 2);
    }

    /*   DeclareAccessorPair (ByteArrayOop, bytecode, setBytecode);
       DeclareAccessorPair (ArrayOop, literals, setLiterals);
       DeclareAccessorPair (StringOop, sourceText, setSourceText);
       DeclareAccessorPair (SymbolOop, selector, setSelector);
       DeclareAccessorPair (SmiOop, stackSize, setStackSize);
       DeclareAccessorPair (SmiOop, temporarySize, setTemporarySize);
       DeclareAccessorPair (Oop, receiver, setReceiver);
       DeclareAccessorPair (SmiOop, argumentCount, setArgumentCount);*/
    in -= 1;
    std::cout << blanks (in) << "}\n";
}
