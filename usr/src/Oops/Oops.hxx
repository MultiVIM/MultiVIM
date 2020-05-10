#ifndef CLASSOOP_HXX_
#define CLASSOOP_HXX_

#include "ObjectMemory/ObjectMemory.hxx"

class LinkOop : public OopOop
{
  public:
    Oop & atOne ();
    Oop & atTwo ();
    LinkOop & nextLink ();

    inline Oop atOnePut (Oop aVal);
    inline Oop atTwoPut (Oop aVal);
    inline LinkOop nextLinkPut (LinkOop aVal);

    static LinkOop newWith (Oop a, Oop b);
};

class ArrayOop : public OopOop
{
};

class DictionaryOop : public OopOop
{
  public:
    /**
     * Inserts /a value under /a key under the hash /a hash.
     */
    void insert (int hash, Oop key, Oop value);

    /**
     * Looks for a key-value pair by searching for the given hash, then running
     * /a fun on each element that matches, with the value attached to the key
     * as the first parameter and /a extraVal as the second parameter. When
     * /a fun returns true, that key-value pair is returned.
     */
    template <typename ExtraType>
    std::pair<Oop, Oop> findPairByFun (int hash, ExtraType extraVal,
                                       int (*fun) (Oop, ExtraType));

    void print (int in);
};

/* The class method named new: is used to allocate a new byte array with a given
size. new: size " < 20 ByteArray size > The remaining two functions are the
methods at:ifAbsent: and at:put:, which do the actual access to byte values.
Note that these dier from the methods in class Array only in the primitive
operations they employ.
*/

class ByteArrayOop : public ByteOop
{
  public:
    static ByteArrayOop newWithSize (size_t size);
};

class StringOop : public ByteArrayOop
{
  public:
    static StringOop fromString (std::string aString);

    void setString (std::string aString);

    bool strEquals (std::string aString);
};

class SymbolOop : public StringOop
{
  public:
    static SymbolOop fromString (std::string aString);

    void print (int in);
};

class ClassPair
{
  public:
    ClassOop cls;
    ClassOop metaCls;

    ClassPair (ClassOop cls, ClassOop metaCls) : cls (cls), metaCls (metaCls)
    {
    }

    /**
     * Allocates a raw classpair: no name, no iVar vector, no method dictionary.
     */
    static ClassPair allocateRaw ();
};

#include "Oops.inl.hxx"

#endif