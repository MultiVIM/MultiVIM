#ifndef OBJECTMEMORY_HXX_
#define OBJECTMEMORY_HXX_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Oops/Oop.hxx"

class ClassOop : public OopOop
{
  public:
    /*
     * For a system class (i.e. one that can't be altered by the running
     * Smalltalk image), defines the size of one of its instances.
     */
    static const int clsInstLength = 5;

    DeclareAccessorPair (SymbolOop, name, setName);
    DeclareAccessorPair (ClassOop, superClass, setSuperClass);
    DeclareAccessorPair (DictionaryOop, methods, setMethods);
    DeclareAccessorPair (SmiOop, nstSize, setNstSize);
    DeclareAccessorPair (ArrayOop, nstVars, setNstVars);

    /**
     * Adds an instance method to the class. Creates the method dictionary if
     * necessary. Sets the method's class pointer accordingly.
     */
    void addMethod (MethodOop method);

    /**
     * Adds a class method to the class. Creates the metaclass' method
     * dictionary if necessary. Sets the method's class pointer accordingly.
     */
    void addClassMethod (MethodOop method);

    /**
     * For a class that was raw-allocated, sets it up in the same way as
     * allocate() does. Requires that the class has an allocated metaclass
     * stored into its isa field.
     */
    void setupClass (ClassOop superClass, std::string name);

    /**
     * Allocates a raw single class. Does NOT set up its name, superclass,
     * methods dictionary, size, or instance variables fields.
     */
    static ClassOop allocateRawClass ();

    /**
     * Allocates an empty classpair with the given superclass and name.
     */
    static ClassOop allocate (ClassOop superClass, std::string name);

    void print (int in);
};

class MemoryManager
{
    friend class Oop;

    size_t lowestFreeSlot;

    struct TableEntry
    {
        ActualObject * obj;
    };

    TableEntry * _table;
    size_t _tableSize;

  public:
#pragma mark system objects
    Oop objNil ();
    Oop objTrue ();
    Oop objFalse ();

    /**
     * Retrieves the dictionary of symbols, indexed by string hash value.
     */
    DictionaryOop objSymbolTable ();
    /*
     * Retrieves the dictionary of global variables, indexed by object table
     * entry of the symbol. Known as 'Smalltalk' in-world.
     */
    DictionaryOop objGlobals ();

    ClassOop clsBlock ();
    ClassOop clsObjectMeta ();
    ClassOop clsObject ();
    ClassOop clsInteger ();
    ClassOop clsSymbol ();
    ClassOop clsString ();
    ClassOop clsArray ();
    ClassOop clsByteArray ();
    ClassOop clsMethod ();
    ClassOop clsProcess ();
    ClassOop clsUndefinedObject ();
    ClassOop clsDictionary ();
    ClassOop clsLink ();
    ClassOop clsContext ();
    ClassOop clsSymbolTable ();
    ClassOop clsSystemDictionary ();
    ClassOop clsTrue ();
    ClassOop clsFalse ();
    ClassOop clsFloat ();
    ClassOop clsVM ();

#pragma mark memory
    inline ActualObject * actualObjectForOop (Oop anOop);

    /**
     * Allocates a raw classpair (no iVar vector, no method dictionary.)
     */
    ClassPair allocateRawClassPair (std::string name);
    /**
     * Allocates an object composed of object pointers.
     */
    OopOop allocateOopObj (size_t len);
    /**
     * Allocates an object composed of bytes.
     */
    ByteOop allocateByteObj (size_t len);

    /**
     * Copies an object. Does not deep-copy anything at all.
     */
    Oop copyObj (Oop anObj);

#pragma mark classes
    ClassOop findOrCreateClass (ClassOop superClass, std::string name);
    ClassOop lookupClass (std::string name);

#pragma mark init

    /**
     * Performs a cold boot of the Object Manager. Essential objects are
     * provisionally set up such that it is possible to define classes.
     */
    void setupInitialObjects ();
};

#include "Lowlevel/MVBeginPackStruct.h"
class ActualObject
{
  protected:
    friend class Oop;
    friend class OopOop;
    friend class ByteOop;
    friend class MemoryManager;

    enum
    {
        kByte,
        kOopRef,
        kWeakOopRef,
    } kind : 3;
    /**
     * Has this object been fully marked by the garbage collector?
     */
    bool marked : 1;
    /**
     * Is this object (and all objects transitively accessible from it) to be
     * preserved even if not transitively accessible from the roots?
     */
    bool volatil : 1;
    uintptr_t size : 29;

    /* Oop to this object's class. */
    ClassOop isa PACKSTRUCT;
    /* Space for the object's fields. */
    union {
        uint8_t bytes[0];
        Oop oops[0];
    } vonNeumannSpace PACKSTRUCT;
};
#include "Lowlevel/MVEndPackStruct.h"

extern MemoryManager memMgr;

#include "ObjectMemory.inl.hxx"

#endif