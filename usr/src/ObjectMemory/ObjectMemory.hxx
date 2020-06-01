#ifndef OBJECTMEMORY_HXX_
#define OBJECTMEMORY_HXX_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Oops/Oop.hxx"

class ClassOopDesc : public OopOopDesc
{
  public:
    /*
     * For a system class (i.e. one that can't be altered by the running
     * Smalltalk image), defines the size of one of its instances.
     */
    static const int clsInstLength = 6;

    DeclareAccessorPair (SymbolOop, name, setName);
    DeclareAccessorPair (ClassOop, superClass, setSuperClass);
    DeclareAccessorPair (DictionaryOop, methods, setMethods);
    DeclareAccessorPair (SmiOop, nstSize, setNstSize);
    DeclareAccessorPair (ArrayOop, nstVars, setNstVars);
    DeclareAccessorPair (DictionaryOop, dictionary, setDictionary);

    inline ClassOop isa ();
    inline ClassOop setIsa (ClassOop val);

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
     * Sets the superclass and metaclass' superclass for a given superclass.
     */
    void setupSuperclass (ClassOop superClass);

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
    friend class OopDesc;

  public:
#pragma mark system objects
    static Oop objNil;
    static Oop objTrue;
    static Oop objFalse;
    /* Indexed by string hash */
    static DictionaryOop objSymbolTable;
    /* Indexed by value */
    static DictionaryOop objGlobals;
    static Oop objsmalltalk;
    static Oop objUnused1;
    static Oop objUnused2;
    static Oop objUnused3;
    static Oop objMinClass;
    static ClassOop clsObjectMeta;
    static ClassOop clsObject;
    static ClassOop clsSymbol;
    static ClassOop clsInteger;
    static ClassOop clsArray;
    static ClassOop clsByteArray;
    static ClassOop clsString;
    static ClassOop clsMethod;
    static ClassOop clsProcess;
    static ClassOop clsUndefinedObject;
    static ClassOop clsTrue;
    static ClassOop clsFalse;
    static ClassOop clsLink;
    static ClassOop clsDictionary;
    static ClassOop clsBlock;
    static ClassOop clsContext;
    static ClassOop clsSymbolTable;
    static ClassOop clsSystemDictionary;
    static ClassOop clsFloat;
    static ClassOop clsVM;
    static ClassOop clsChar;
    static ClassOop clsProcessor;
    static ClassOop clsNativeCode;
    static ClassOop clsNativePointer;

#pragma mark memory

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
#include "Lowlevel/MVEndPackStruct.h"

extern MemoryManager memMgr;
extern SymbolOop symIfTrueIfFalse;
extern SymbolOop symValue;

#include "ObjectMemory.inl.hxx"

#endif