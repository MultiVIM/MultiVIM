#include <iostream>

#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "ObjectMemory.hxx"
#include "Oops/Oops.hxx"
#include "VM/Bytecode.hxx"

MemoryManager memMgr;

#define kClassPairEntry(Name, Pos) kCls##Name##Meta = Pos, kCls##Name = Pos + 1

struct CoreObjects
{
    enum
    {
        kObjNil = 0,
        kObjTrue = 1,
        kObjFalse = 2,
        kObjSymbolTable = 3,
        kObjGlobals = 4,
        kObjUnused = 5,
        kClassPairEntry (Object, 6),
        kClassPairEntry (Symbol, 8),
        kClassPairEntry (Smi, 10),
        kClassPairEntry (Array, 12),
        kClassPairEntry (ByteArray, 14),
        kClassPairEntry (String, 16),
        kClassPairEntry (Method, 18),
        kClassPairEntry (Process, 20),
        kClassPairEntry (UndefinedObject, 22),
        kClassPairEntry (True, 24),
        kClassPairEntry (False, 26),
        kClassPairEntry (Link, 28),
        kClassPairEntry (Dictionary, 30),
        kClassPairEntry (Block, 32),
        kClassPairEntry (Context, 34),
        kClassPairEntry (SymbolTable, 36),
        kClassPairEntry (SystemDictionary, 38),
        kMax,
    };
};

static const char * classNames[] = {
    "Object",
    "Symbol",
    "Smi",
    "Array",
    "ByteArray",
    "String",
    "Method",
    "Process",
    "UndefinedObject",
    "True",
    "False",
    "Link",
    "Dictionary",
    "Block",
    "Context",
    "SymbolTable",
    "SystemDictionary",
};

OopOop MemoryManager::allocateOopObj (size_t len)
{
    size_t slot = lowestFreeSlot++;
    ActualObject * obj =
        (ActualObject *)calloc (1, sizeof (ActualObject) + sizeof (Oop) * len);
    Oop result (false, slot);
    obj->size = len;
    _table[slot] = {obj};
    return *static_cast<OopOop *> (&result);
}

ByteOop MemoryManager::allocateByteObj (size_t len)
{
    size_t slot = lowestFreeSlot++;
    ActualObject * obj =
        (ActualObject *)calloc (1, sizeof (ActualObject) + sizeof (Oop) * len);
    Oop result (false, slot);
    obj->size = len;
    _table[slot] = {obj};
    return *static_cast<ByteOop *> (&result);
}

void MemoryManager::setupInitialObjects ()
{
    std::cout << "Object Memory: Setting up initial objects..\n";
    MethodNode * bootMeth;

    _table = (TableEntry *)calloc (65535, sizeof (*_table));

    lowestFreeSlot = 0;

    /* First, the basic objects and classes must be created. Using the usual
     * constructors is forbidden for now because they do things like allocate
     * symbols - which changes the object table's current allocation-top. */

#define CreateObj(Name, Size)                                                  \
    assert (((allocateOopObj (Size)).index () == CoreObjects::kObj##Name))
    CreateObj (Nil, 0);
    CreateObj (True, 0);
    CreateObj (False, 0);
    CreateObj (SymbolTable, 1);
    CreateObj (Globals, 1);
    CreateObj (Unused, 0);

#define CreateClassPair(Name)                                                  \
    assert (ClassOop::allocateRawClass ().index () ==                          \
            CoreObjects::kCls##Name##Meta);                                    \
    assert (ClassOop::allocateRawClass ().index () == CoreObjects::kCls##Name)

    CreateClassPair (Object);
    CreateClassPair (Symbol);
    CreateClassPair (Smi);
    CreateClassPair (Array);
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

    objSymbolTable ().basicatPut (1, allocateOopObj (3 * 53));
    objGlobals ().basicatPut (1, allocateOopObj (3 * 53));

#define AddGlobal(name, obj)                                                   \
    objGlobals ().symbolInsert (SymbolOop::fromString ("name"), obj)

    AddGlobal ("nil", objNil ());
    AddGlobal ("true", objTrue ());
    AddGlobal ("false", objFalse ());

    for (int i = CoreObjects::kClsObjectMeta; i < kMax; i++)
    {
        ClassOop metaCls = Oop (false, i++).asClassOop (),
                 cls = Oop (false, i).asClassOop ();
        int index = (i - 1 - CoreObjects::kObjUnused) / 2;
        /* Set the metaclass instance length to the length of a class... */
        metaCls.nstSize () = SmiOop (ClassOop::clsInstLength);
        metaCls.name () =
            SymbolOop::fromString (std::string (classNames[index]) + "Meta");
        cls.name () = SymbolOop::fromString (classNames[index]);
        cls.setIsa (metaCls);
        objGlobals ().symbolInsert (cls.name (), cls);
    }

    // bootMeth->generate ().print (0);
    // printBytecode (bootMeth->generate ().bytecode ());
    // objSymbolTable ().print (0);
}

Oop MemoryManager::objNil ()
{
    return Oop (false, CoreObjects::kObjNil);
}
Oop MemoryManager::objTrue ()
{
    return Oop (false, CoreObjects::kObjTrue);
}
Oop MemoryManager::objFalse ()
{
    return Oop (false, CoreObjects::kObjFalse);
}

DictionaryOop MemoryManager::objSymbolTable ()
{
    return Oop (false, CoreObjects::kObjSymbolTable).asDictionaryOop ();
}

DictionaryOop MemoryManager::objGlobals ()
{
    return Oop (false, CoreObjects::kObjGlobals).asDictionaryOop ();
}

#define GetClassFun(Name)                                                      \
    ClassOop MemoryManager::cls##Name ()                                       \
    {                                                                          \
        return Oop (false, CoreObjects::kCls##Name).asClassOop ();             \
    }

GetClassFun (ObjectMeta);
GetClassFun (Object);
GetClassFun (Smi);
GetClassFun (Symbol);
GetClassFun (String);
GetClassFun (Array);
GetClassFun (ByteArray);
GetClassFun (Method);
GetClassFun (Process);
GetClassFun (UndefinedObject);
GetClassFun (Dictionary);
GetClassFun (Block);
GetClassFun (Context);
GetClassFun (SymbolTable);
GetClassFun (SystemDictionary);

ClassOop MemoryManager::findOrCreateClass (ClassOop superClass,
                                           std::string name)
{
    ClassOop result = lookupClass (name);

    if (result.isNil ())
        result = ClassOop::allocate (superClass, name);
    else
        result.setupClass (superClass, name);

    return result;
}

ClassOop MemoryManager::lookupClass (std::string name)
{
    return objGlobals ().symbolLookup (name).asClassOop ();
}
