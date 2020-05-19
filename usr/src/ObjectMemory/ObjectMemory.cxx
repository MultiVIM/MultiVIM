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
        kObjsmalltalk = 5,
        kObjUnused1 = 6,
        kObjUnused2 = 7,
        kObjUnused3 = 8,
        kObjMinClass = 9,
        kClassPairEntry (Object, 10),
        kClassPairEntry (Symbol, 12),
        kClassPairEntry (Integer, 14),
        kClassPairEntry (Array, 16),
        kClassPairEntry (ByteArray, 18),
        kClassPairEntry (String, 20),
        kClassPairEntry (Method, 22),
        kClassPairEntry (Process, 24),
        kClassPairEntry (UndefinedObject, 26),
        kClassPairEntry (True, 28),
        kClassPairEntry (False, 30),
        kClassPairEntry (Link, 32),
        kClassPairEntry (Dictionary, 34),
        kClassPairEntry (Block, 36),
        kClassPairEntry (Context, 38),
        kClassPairEntry (SymbolTable, 40),
        kClassPairEntry (SystemDictionary, 42),
        kClassPairEntry (Float, 44),
        kClassPairEntry (VM, 46),
        kMax,
    };
};

static const char * classNames[] = {"Object",
                                    "Symbol",
                                    "Integer",
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
                                    "Float",
                                    "VM"};

OopOop MemoryManager::allocateOopObj (size_t len)
{
    size_t slot = lowestFreeSlot++;
    // printf ("Slot: %d\n", slot);
    ActualObject * obj =
        (ActualObject *)calloc (1, sizeof (ActualObject) + sizeof (Oop) * len);
    Oop result (false, slot);
    obj->kind = ActualObject::kOopRef;
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
    obj->kind = ActualObject::kByte;
    obj->size = len;
    _table[slot] = {obj};
    return *static_cast<ByteOop *> (&result);
}

Oop MemoryManager::copyObj (Oop obj)
{
    assert (!obj.isInteger ());

    if (obj.actualObject ()->kind == ActualObject::kByte)
    {
        ByteOop newObj = allocateByteObj (obj.size ());
        newObj.setIsa (obj.isa ());
        memcpy (newObj.vonNeumannSpace (),
                obj.asByteArrayOop ().vonNeumannSpace (),
                obj.size ());
        return newObj;
    }
    else
    {
        OopOop newObj = allocateOopObj (obj.size ());
        newObj.setIsa (obj.isa ());
        memcpy (newObj.vonNeumannSpace (),
                obj.asOopOop ().vonNeumannSpace (),
                obj.size () * sizeof (Oop));
        return newObj;
    }
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
    CreateObj (smalltalk, 0);
    CreateObj (Unused1, 0);
    CreateObj (Unused2, 0);
    CreateObj (Unused3, 0);
    CreateObj (MinClass, 0);

#define CreateClassPair(Name)                                                  \
    assert (ClassOop::allocateRawClass ().index () ==                          \
            CoreObjects::kCls##Name##Meta);                                    \
    assert (ClassOop::allocateRawClass ().index () == CoreObjects::kCls##Name)

    CreateClassPair (Object);
    CreateClassPair (Symbol);
    CreateClassPair (Integer);
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
    CreateClassPair (SymbolTable);
    CreateClassPair (SystemDictionary);
    CreateClassPair (Float);
    CreateClassPair (VM);

    objSymbolTable ().basicatPut (1, ArrayOop::newWithSize (3 * 53));
    objGlobals ().basicatPut (1, ArrayOop::newWithSize (3 * 53));

#define AddGlobal(name, obj)                                                   \
    objGlobals ().symbolInsert (SymbolOop::fromString ("name"), obj)

    AddGlobal ("nil", objNil ());
    AddGlobal ("true", objTrue ());
    AddGlobal ("false", objFalse ());

    objNil ().setIsa (clsUndefinedObject ());
    objTrue ().setIsa (clsTrue ());
    objFalse ().setIsa (clsFalse ());
    objSymbolTable ().setIsa (clsDictionary ());
    objGlobals ().setIsa (clsSystemDictionary ());
    // objVM ().setIsa (clsVM ());

    printf ("Symtab %d.isa: %d\n",
            objSymbolTable ().index (),
            objSymbolTable ().isa ().index ());
    printf ("Globals %d .isa: %d\n",
            objGlobals ().index (),
            objGlobals ().isa ().index ());

    for (int i = CoreObjects::kClsObjectMeta; i < CoreObjects::kMax; i++)
    {
        ClassOop metaCls = Oop (false, i++).asClassOop (),
                 cls = Oop (false, i).asClassOop ();
        int index = (i - 1 - CoreObjects::kObjMinClass) / 2;
        /* Set the metaclass instance length to the length of a class... */
        metaCls.nstSize () = SmiOop (ClassOop::clsInstLength);
        metaCls.setName (
            SymbolOop::fromString (std::string (classNames[index]) + "Meta"));
        metaCls.setIsa (clsObjectMeta ());
        cls.setName (SymbolOop::fromString (classNames[index]));
        cls.setIsa (metaCls);
        printf ("Do %s\n", classNames[index]);
        objGlobals ().symbolInsert (cls.name (), cls);
    }

    clsBlock ().print (5);

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
GetClassFun (Integer);
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
GetClassFun (True);
GetClassFun (False);
GetClassFun (Float);
GetClassFun (VM);
GetClassFun (Link);

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
