#include <cstdlib>
#include <gc/gc.h>
#include <iostream>

#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "ObjectMemory.hxx"
#include "Oops/Oops.hxx"
#include "VM/Bytecode.hxx"

MemoryManager memMgr;

Oop MemoryManager::objNil;
Oop MemoryManager::objTrue;
Oop MemoryManager::objFalse;
/* Indexed by string hash */
DictionaryOop MemoryManager::objSymbolTable;
/* Indexed by value */
DictionaryOop MemoryManager::objGlobals;
Oop MemoryManager::objsmalltalk;
Oop MemoryManager::objUnused1;
Oop MemoryManager::objUnused2;
Oop MemoryManager::objUnused3;
Oop MemoryManager::objMinClass;
ClassOop MemoryManager::clsObjectMeta;
ClassOop MemoryManager::clsObject;
ClassOop MemoryManager::clsSymbol;
ClassOop MemoryManager::clsInteger;
ClassOop MemoryManager::clsArray;
ClassOop MemoryManager::clsByteArray;
ClassOop MemoryManager::clsString;
ClassOop MemoryManager::clsMethod;
ClassOop MemoryManager::clsProcess;
ClassOop MemoryManager::clsUndefinedObject;
ClassOop MemoryManager::clsTrue;
ClassOop MemoryManager::clsFalse;
ClassOop MemoryManager::clsLink;
ClassOop MemoryManager::clsDictionary;
ClassOop MemoryManager::clsBlock;
ClassOop MemoryManager::clsContext;
ClassOop MemoryManager::clsSymbolTable;
ClassOop MemoryManager::clsSystemDictionary;
ClassOop MemoryManager::clsFloat;
ClassOop MemoryManager::clsVM;
ClassOop MemoryManager::clsChar;
ClassOop MemoryManager::clsProcessor;
ClassOop MemoryManager::clsNativeCode;
ClassOop MemoryManager::clsNativePointer;
SymbolOop symIfTrueIfFalse;
SymbolOop symValue;

//#define calloc(a, b) GC_debug_malloc ((b), __FILE__, __LINE__)
#define calloc(a, b) GC_malloc (b)

OopOop MemoryManager::allocateOopObj (size_t len)
{
    OopOopDesc * obj =
        (OopOopDesc *)calloc (1, sizeof (OopOopDesc) + sizeof (Oop) * len);
    obj->_size = len;
    obj->_kind = VT_Mem_kOop;
    return OopOop (obj);
}

ByteOop MemoryManager::allocateByteObj (size_t len)
{
    ByteOopDesc * obj = (ByteOopDesc *)calloc (
        1, sizeof (ByteOopDesc) + sizeof (uint8_t) * len);
    obj->_size = len;
    obj->_kind = VT_Mem_kByte;
    return ByteOop (obj);
}

Oop MemoryManager::copyObj (Oop oldObj)
{
    MemOop obj = oldObj->asMemOop ();

    if (obj->_kind == VT_Mem_kByte)
    {
        ByteOop newObj = allocateByteObj (obj->size ());
        newObj.setIsa (obj.isa ());
        memcpy (newObj->vonNeumannSpace (),
                obj->asByteArrayOop ()->vonNeumannSpace (),
                obj->size ());
        return newObj;
    }
    else
    {
        OopOop newObj = allocateOopObj (obj->size ());
        newObj.setIsa (obj.isa ());
        memcpy (newObj->vonNeumannSpace (),
                obj->asOopOop ()->vonNeumannSpace (),
                obj->size () * sizeof (Oop));
        return newObj;
    }
}

void MemoryManager::setupInitialObjects ()
{
    std::cout << "Object Memory: Setting up initial objects..\n";
    MethodNode * bootMeth;

    /* First, the basic objects and classes must be created. Using the usual
     * constructors is forbidden for now because they do things like allocate
     * symbols - which changes the object table's current allocation-top. */

#define CreateObj(Name, Size) obj##Name = allocateOopObj (Size)
    CreateObj (Nil, 0);
    CreateObj (True, 0);
    CreateObj (False, 0);
    objSymbolTable = allocateOopObj (1)->asDictionaryOop ();
    objGlobals = allocateOopObj (1)->asDictionaryOop ();
    CreateObj (smalltalk, 0);

    clsSymbol = ClassOopDesc::allocateRawClass (); //
    clsArray = ClassOopDesc::allocateRawClass ();  // 7c2b70 - receiver.isa()

    objSymbolTable->basicatPut (1, ArrayOopDesc::newWithSize (3 * 53));
    objGlobals->basicatPut (
        1, ArrayOopDesc::newWithSize (3 * 53)); //  0x7ec400 - receiver

    objGlobals->symbolInsert (SymbolOopDesc::fromString ("Symbol"), clsSymbol);
    objGlobals->symbolInsert (SymbolOopDesc::fromString ("Array"), clsArray);

#define CreateClassPair(Name)                                                  \
    cls##Name = ClassOopDesc::allocateRawClass ();                             \
    cls##Name->setName (SymbolOopDesc::fromString (#Name));                    \
    objGlobals->symbolInsert (SymbolOopDesc::fromString (#Name), cls##Name)

    CreateClassPair (ObjectMeta);
    CreateClassPair (Object);
    clsObject.setIsa (clsObjectMeta);
    CreateClassPair (Integer);
    CreateClassPair (ByteArray);
    CreateClassPair (String);
    CreateClassPair (Method);
    CreateClassPair (Process);
    CreateClassPair (UndefinedObject);
    CreateClassPair (True);
    CreateClassPair (False);
    CreateClassPair (Link);
    CreateClassPair (Dictionary);
    CreateClassPair (Block);
    CreateClassPair (Context);
    CreateClassPair (SymbolTable);
    CreateClassPair (SystemDictionary);
    CreateClassPair (Float);
    CreateClassPair (VM);
    CreateClassPair (Char);
    CreateClassPair (NativeCode);
    CreateClassPair (NativePointer);
    //  CreateClassPair (Processor);

#define AddGlobal(name, obj)                                                   \
    objGlobals->symbolInsert (SymbolOopDesc::fromString (name), obj)

    AddGlobal ("nil", objNil);
    AddGlobal ("true", objTrue);
    AddGlobal ("false", objFalse);

    objNil.setIsa (clsUndefinedObject);
    objTrue.setIsa (clsTrue);
    objFalse.setIsa (clsFalse);
    objSymbolTable.setIsa (clsDictionary);
    objGlobals.setIsa (clsSystemDictionary);

    symIfTrueIfFalse = SymbolOopDesc::fromString ("ifTrue:ifFalse:");
    symValue = SymbolOopDesc::fromString ("value");

    clsBlock->print (5);
}

ClassOop MemoryManager::findOrCreateClass (ClassOop superClass,
                                           std::string name)
{
    ClassOop result = lookupClass (name);

    if (result.isNil ())
    {
        printf ("Nil - allocating new class %s\n", name.c_str ());
        result = ClassOopDesc::allocate (superClass, name);
    }
    else
        result->setupClass (superClass, name);

    return result;
}

ClassOop MemoryManager::lookupClass (std::string name)
{
    return objGlobals->symbolLookup (name)->asClassOop ();
}
