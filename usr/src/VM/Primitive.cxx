#include <cmath>
#include <csetjmp>

#include "Interpreter.hxx"

Oop unsupportedPrim (ProcessOop proc, ArrayOop args)
{
    return (Oop::nilObj ());
}

/*
Prints the number of available object table entries.
Always fails.
Called from Scheduler>>initialize
*/
Oop primAvailCount (ProcessOop proc, ArrayOop args)
{
    // fprintf (stderr, "free: %d\n", availCount ());
    return (Oop::nilObj ());
}

/*
Returns a pseudo-random integer.
Called from
  Random>>next
  Random>>randInteger:
*/
Oop primRandom (ProcessOop proc, ArrayOop args)
{
    short i;
    /* this is hacked because of the representation */
    /* of integers as shorts */
    i = rand () >> 8; /* strip off lower bits */
    if (i < 0)
        i = -i;
    return (SmiOop (i >> 1));
}

extern bool watching;

/*
Inverts the state of a switch.  The switch controls, in part, whether or
not "watchWith:" messages are sent to Methods during execution.
Returns the Boolean representation of the switch value after the invert.
Called from Smalltalk>>watch
*/
Oop primFlipWatching (ProcessOop proc, ArrayOop args)
{
    /* fixme */
    bool watching = !watching;
    return (
        (Oop) (watching ? MemoryManager::objTrue : MemoryManager::objFalse));
}

/*
Terminates the interpreter.
Never returns.
Not called from the image.
*/
Oop primExit (ProcessOop proc, ArrayOop args)
{
    exit (0);
}

/*
Returns the class of which the receiver is an instance.
Called from Object>>class
*/
Oop primClass (ProcessOop proc, ArrayOop args)
{
    return (args->basicAt (1).isa ());
}

/*
Returns the field count of the von Neumann space of the receiver.
Called from Object>>basicSize
*/
Oop primSize (ProcessOop proc, ArrayOop args)
{
    int i;
    if (args->basicAt (1).isInteger ())
        i = 0;
    else
        i = args->basicAt (1)->asMemOop ()->size ();
    return (SmiOop (i));
}

/*
Returns a hashed representation of the receiver.
Called from Object>>hash
*/
Oop primHash (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).isInteger ())
        return (args->basicAt (1));
    else
        return (SmiOop (args->basicAt (1).index ()));
}

/*
Changes the active process stack if appropriate.  The change causes
control to be returned (eventually) to the context which sent the
message which created the context which invoked this primitive.
Returns true if the change was made; false if not.
Called from Context>>blockReturn
N.B.:  This involves some tricky code.  The compiler generates the
message which invokes Context>>blockReturn.  Context>>blockReturn is a
normal method.  It processes the true/false indicator.  Its result is
discarded when it returns, exposing the value to be returned from the
context which invokes this primitive.  Only then is the process stack
change effective.
*/
Oop primBlockReturn (ProcessOop proc, ArrayOop args)
{
    int i;
    int j;

    /* FIXME:
    // first get previous link pointer
    i = intValueOf (orefOf (processStack, linkPointer).val);
    // then creating context pointer
    j = intValueOf (orefOf (args->basicAt (1)->ptr, 1).val);
    if (ptrNe (orefOf (processStack, j + 1), args->basicAt (1)))
        return ((Oop)MemoryManager::objFalse);
    // first change link pointer to that of creator
    orefOfPut (processStack, i, orefOf (processStack, j));
    // then change return point to that of creator
    orefOfPut (processStack, i + 2, orefOf (processStack, j + 2)); */
    return ((Oop)MemoryManager::objTrue);
}

jmp_buf jb = {};

void brkfun (int sig)
{
    longjmp (jb, 1);
}

void brkignore (int sig)
{
}

// bool execute (encPtr aProcess, int maxsteps);

/*
Executes the receiver until its time slice is ended or terminated.
Returns true in the former case; false in the latter.
Called from Process>>execute
*/
Oop primExecute (ProcessOop proc, ArrayOop args)
{
    /*encPtr saveProcessStack;
    int saveLinkPointer;
    int * saveCounterAddress;*/
    Oop returnedObject;
    // first save the values we are about to clobber
    /* FIXME: saveProcessStack = processStack;
    saveLinkPointer = linkPointer;
    saveCounterAddress = counterAddress;
    // trap control-C
    signal (SIGINT, brkfun);
    if (setjmp (jb))
        returnedObject = (Oop)MemoryManager::objFalse;
    else if (execute (args->basicAt (1)->ptr, 1 << 12))
        returnedObject = (Oop)MemoryManager::objTrue;
    else
        returnedObject = (Oop)MemoryManager::objFalse;
    signal (SIGINT, brkignore);
    // then restore previous environment
    processStack = saveProcessStack;
    linkPointer = saveLinkPointer;
    counterAddress = saveCounterAddress;*/
    return (returnedObject);
}

/*
Returns true if the content of the receiver's Oop is equal to that
of the first argument's; false otherwise.
Called from Object>>==
*/
Oop primIdent (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1) == args->basicAt (2))
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Defines the receiver to be an instance of the first argument.
Returns the receiver.
Called from
  BlockNode>>newBlock
  ByteArray>>asString
  ByteArray>>size:
  Class>>new:
*/
Oop primClassOfPut (ProcessOop proc, ArrayOop args)
{
    fprintf (stderr,
             "Setting ClassOf %d to %d\n, ",
             args->basicAt (1),
             args->basicAt (2));
    args->basicAt (1).setIsa (args->basicAt (2)->asClassOop ());
    return (args->basicAt (1));
}

/*
Creates a new String.  The von Neumann space of the new String is that
of the receiver, up to the left-most null, followed by that of the first
argument, up to the left-most null, followed by a null.
Returns the new String.
Called from
  String>>,
  Symbol>>asString
*/
Oop primStringCat (ProcessOop proc, ArrayOop args)
{
    uint8_t * src1 = args->basicAt (1)->asStringOop ()->vonNeumannSpace ();
    size_t len1 = strlen ((char *)src1);
    uint8_t * src2 = args->basicAt (2)->asStringOop ()->vonNeumannSpace ();
    size_t len2 = strlen ((char *)src2);
    StringOop ans = memMgr.allocateByteObj (len1 + len2 + 1)->asStringOop ();
    uint8_t * tgt = ans->vonNeumannSpace ();
    (void)memcpy (tgt, src1, len1);
    (void)memcpy (tgt + len1, src2, len2);
    ans.setIsa (MemoryManager::clsString);
    return ((Oop)ans);
}

/*
Returns the Oop of the receiver denoted by the argument.
Called from Object>>basicAt:
*/
Oop primBasicAt (ProcessOop proc, ArrayOop args)
{
    int i;
    if (args->basicAt (1).isInteger ())
    {
        printf ("integer receiver of basicAt:");
        return (Oop::nilObj ());
    }
    /* if (!args->basicAt (1)->kind == OopsRefObj)
        return (Oop::nilObj ()); */
    if (!args->basicAt (2).isInteger ())
    {
        printf ("non-integer argument of basicAt:");
        return (Oop::nilObj ());
    }
    i = args->basicAt (2).asSmiOop ().intValue ();
    if (i < 1 || i > args->basicAt (1)->asMemOop ()->size ())
    {
        printf ("#basicAt: argument out of bounds (%d)", i);
        return (Oop::nilObj ());
    }

    return args->basicAt (1)->asOopOop ()->basicAt (i);
}

/*
Returns an encoded representation of the byte of the receiver denoted by
the argument.
Called from ByteArray>>basicAt:
*/
Oop primByteAt (ProcessOop proc, ArrayOop args) /*fix*/
{
    int i;
    if (!args->basicAt (2).isInteger ())
        perror ("non integer index byteAt:");
    i = args->basicAt (1)->asByteArrayOop ()->basicAt (
        args->basicAt (2).asSmiOop ().intValue ());
    if (i < 0)
        i += 256;
    return (SmiOop (i));
}

/*
Defines the global value of the receiver to be the first argument.
Returns the receiver.
Called from Symbol>>assign:
*/
Oop primSymbolAssign (ProcessOop proc, ArrayOop args) /*fix*/
{
    MemoryManager::objGlobals->symbolInsert (args->basicAt (1)->asSymbolOop (),
                                             args->basicAt (2));
    return (args->basicAt (1));
}

/*
Defines the Oop of the receiver denoted by the first argument to be
the second argument.
Returns the receiver.
Called from Object>>basicAt:put:
*/
Oop primBasicAtPut (ProcessOop proc, ArrayOop args)
{
    int i;
    if (args->basicAt (1).isInteger ())
        return (Oop::nilObj ());
    if (!args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    i = args->basicAt (2).asSmiOop ().intValue ();
    if (i < 1 || i > args->basicAt (1)->asMemOop ()->size ())
        return (Oop::nilObj ());
    args->basicAt (1)->asOopOop ()->basicatPut (i, args->basicAt (3));
    return args->basicAt (1);
}

/*
Defines the byte of the receiver denoted by the first argument to be a
decoded representation of the second argument.
Returns the receiver.
Called from ByteArray>>basicAt:put:
*/
Oop primByteAtPut (ProcessOop proc, ArrayOop args) /*fix*/
{
    int i;
    if (!args->basicAt (2).isInteger ())
        perror ("#byteAt: non integer index");
    if (!args->basicAt (3).isInteger ())
        perror ("#byteAt: non integer assignee");
    args->basicAt (1)->asByteArrayOop ()->basicatPut (
        args->basicAt (2).asSmiOop ().intValue (),
        args->basicAt (3).asSmiOop ().intValue ());
    return (args->basicAt (1));
}

inline intptr_t min (intptr_t one, intptr_t two)
{
    return (one <= two ? one : two);
}

/*
Creates a new String.  The von Neumann space of the new String is
usually that of a substring of the receiver, from the byte denoted by
the first argument through the byte denoted by the second argument,
followed by a null.  However, if the denoted substring is partially
outside the space of the receiver, only that portion within the space of
the receiver is used.  Also, if the denoted substring includes a null,
only that portion up to the left-most null is used.  Further, if the
denoted substring is entirely outside the space of the receiver or its
length is less than one, none of it is used.
Returns the new String.
Called from String>>copyFrom:to:
*/
Oop primCopyFromTo (ProcessOop proc, ArrayOop args) /*fix*/
{
    if ((!args->basicAt (2).isInteger () || (!args->basicAt (3).isInteger ())))
        perror ("#copyFromTo: non integer index");
    {
        uint8_t * src = args->basicAt (1)->asStringOop ()->vonNeumannSpace ();
        size_t len = strlen ((char *)src);
        int pos1 = args->basicAt (2).asSmiOop ().intValue ();
        int pos2 = args->basicAt (2).asSmiOop ().intValue ();
        int req = pos2 + 1 - pos1;
        size_t act;
        StringOop ans;
        uint8_t * tgt;
        if (pos1 >= 1 && pos1 <= len && req >= 1)
            act = min (req, strlen (((char *)src) + (pos1 - 1)));
        else
            act = 0;
        ans = memMgr.allocateByteObj (act + 1)->asStringOop ();
        tgt = ans->vonNeumannSpace ();
        (void)memcpy (tgt, src + (pos1 - 1), act);
        ans.setIsa (MemoryManager::clsString);
        return ((Oop)ans);
    }
}

Oop primParse (ProcessOop proc, ArrayOop args) /*del*/
{
    /*setInstanceVariables (args->basicAt(1)->ptr);
    if (parse (args->basicAt(2)->ptr, (char *)vonNeumannSpaceOf
    (args->basicAt(2)->ptr), false))
    {
        flushCache (orefOf (args->basicAt(2)->ptr, messageInMethod).ptr,
    args->basicAt(1)->ptr); return ((Oop)memMgr.objTrue());
    }
    else
        return ((Oop)memMgr.objFalse());*/
}

/*
Returns the equivalent of the receiver's value in a floating-point
representation.
Called from Integer>>asFloat
*/
Oop primAsFloat (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nilObj ());
    return (args->basicAt (1)); // FIXME:(Oop)FloatOop((double)args->basicAt
                                // (1).asSmiOop ().intValue ()));
}

/*
Defines a counter to be the argument's value.  When this counter is
less than 1, a Process time slice is finished.
Always fails.
Called from
  Scheduler>>critical:
  Scheduler>>yield
*/
Oop primSetTimeSlice (ProcessOop proc, ArrayOop args)
{
    /*FIXME: if (!args->basicAt (1).isInteger ()))
        return (Oop::nilObj ());
    *counterAddress = args->basicAt (1).asSmiOop ().intValue ();*/
    return (Oop::nilObj ());
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of Oops.  The number is denoted by the
receiver.
Called from
  BlockNode>>newBlock
  Class>>new:
*/
Oop primAllocOrefObj (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nilObj ());
    return (
        (Oop)memMgr.allocateOopObj (args->basicAt (1).asSmiOop ().intValue ()));
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of bytes.  The number is denoted by the
receiver.
Called from
  ByteArray>>size:
*/
Oop primAllocByteObj (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nilObj ());
    return ((Oop)memMgr.allocateByteObj (
        args->basicAt (1).asSmiOop ().intValue ()));
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Integer>>+
Also called for SendBinary bytecodes.
*/
Oop primAdd (ProcessOop proc, ArrayOop args)
{
    long longresult;

    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult += args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: bounds test SMI 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nilObj ());
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Integer>>-
Also called for SendBinary bytecodes.
*/
Oop primSubtract (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult -= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: smi boundcheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nilObj ());
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Integer>><
Also called for SendBinary bytecodes.
*/
Oop primLessThan (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () <
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Called from Integer>>>
Also called for SendBinary bytecodes.
*/
Oop primGreaterThan (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () >
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop primLessOrEqual (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () <=
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop primGreaterOrEqual (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () >=
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called for SendBinary bytecodes.
*/
Oop primEqual (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () ==
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop primNotEqual (ProcessOop proc, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (1).asSmiOop ().intValue () !=
        args->basicAt (2).asSmiOop ().intValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Integer>>*
Also called for SendBinary bytecodes.
*/
Oop primMultiply (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult *= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nilObj ());
}

/*
Returns the quotient of the result of dividing the receiver's value by
the argument's value.
Called from Integer>>quo:
Also called for SendBinary bytecodes.
*/
Oop primQuotient (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    if (args->basicAt (2).asSmiOop ().intValue () == 0)
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult /= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nilObj ());
}

/*
Returns the remainder of the result of dividing the receiver's value by
the argument's value.
Called for SendBinary bytecodes.
*/
Oop primRemainder (ProcessOop proc, ArrayOop args)
{
    long longresult;

    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
    {
        printf ("#primRem: receiver or arg not integer");
        return (Oop::nilObj ());
    }
    if (args->basicAt (2).asSmiOop ().intValue () == 0)
    {
        printf ("#primRem: division by zero");
        return (Oop::nilObj ());
    }
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult %= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nilObj ());
}

/*
Returns the bit-wise "and" of the receiver's value and the argument's
value.
Called from Integer>>bitAnd:
Also called for SendBinary bytecodes.
*/
Oop primBitAnd (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult &= args->basicAt (2).asSmiOop ().intValue ();
    return (SmiOop (longresult));
}

/*
Returns the bit-wise "exclusive or" of the receiver's value and the
argument's value.
Called from Integer>>bitXor:
Also called for SendBinary bytecodes.
*/
Oop primBitXor (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult ^= args->basicAt (2).asSmiOop ().intValue ();
    return (SmiOop (longresult));
}

/*
Returns the result of shifting the receiver's value a number of bit
positions denoted by the argument's value.  Positive arguments cause
left shifts.  Negative arguments cause right shifts.  Note that the
result is truncated to the range of embeddable values.
Called from Integer>>bitXor:
*/
Oop primBitShift (ProcessOop proc, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nilObj ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    if (args->basicAt (2).asSmiOop ().intValue () < 0)
        longresult >>= -args->basicAt (2).asSmiOop ().intValue ();
    else
        longresult <<= args->basicAt (2).asSmiOop ().intValue ();
    return (SmiOop (longresult));
}

/*
Returns the field count of the von Neumann space of the receiver up to
the left-most null.
Called from String>>size
*/
Oop primStringSize (ProcessOop proc, ArrayOop args)
{
    return (SmiOop (strlen (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ())));
}

/*
Returns a hashed representation of the von Neumann space of the receiver
up to the left-most null.
Called from
  String>>hash
  Symbol>>stringHash
*/
Oop primStringHash (ProcessOop proc, ArrayOop args)
{
    return (SmiOop (strHash (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ())));
}

/*
Returns a unique object.  Here, "unique" is determined by the
von Neumann space of the receiver up to the left-most null.  A copy will
either be found in or added to the global symbol table.  The returned
object will refer to the copy.
Called from String>>asSymbol
*/
Oop primAsSymbol (ProcessOop proc, ArrayOop args)
{
    return ((Oop)SymbolOopDesc::fromString (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ()));
}

/*
Passes the von Neumann space of the receiver to the host's "system"
function.  Returns what that function returns.
Called from String>>unixCommand
*/
Oop primHostCommand (ProcessOop proc, ArrayOop args)
{
    return (SmiOop (system (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ())));
}

/*
Returns the equivalent of the receiver's value in a printable character
representation.
Called from Float>>printString
*/
Oop primAsString (ProcessOop proc, ArrayOop args)
{
    char buffer[32];
    (void)sprintf (buffer, "%g", args->basicAt (1).asFloatOop ().floatValue ());
    return ((Oop)StringOopDesc::fromString (buffer));
}

/*
Returns the natural logarithm of the receiver's value.
Called from Float>>ln
*/
Oop primNaturalLog (ProcessOop proc, ArrayOop args)
{
    return (
        (Oop)FloatOop (log (args->basicAt (1).asFloatOop ().floatValue ())));
}

/*
Returns "e" raised to a power denoted by the receiver's value.
Called from Float>>exp
*/
Oop primERaisedTo (ProcessOop proc, ArrayOop args)
{
    return (
        (Oop)FloatOop (exp (args->basicAt (1).asFloatOop ().floatValue ())));
}

/*
Returns a new Array containing two integers n and m such that the
receiver's value can be expressed as n * 2**m.
Called from Float>>integerPart
*/
Oop primIntegerPart (ProcessOop proc, ArrayOop args)
{
    double temp;
    int i;
    int j;
    ArrayOop returnedObject = Oop::nilObj ()->asArrayOop ();
#define ndif 12
    temp = frexp (args->basicAt (1).asFloatOop ().floatValue (), &i);
    if ((i >= 0) && (i <= ndif))
    {
        temp = ldexp (temp, i);
        i = 0;
    }
    else
    {
        i -= ndif;
        temp = ldexp (temp, ndif);
    }
    j = (int)temp;
    returnedObject = ArrayOopDesc::newWithSize (2);
    returnedObject->basicatPut (1, SmiOop (j));
    returnedObject->basicatPut (2, SmiOop (i));
#ifdef trynew
    /* if number is too big it can't be integer anyway */
    if (args->basicAt (1).asFloatOop ().floatValue () > 2e9)
        returnedObject = nilObj;
    else
    {
        (void)modf (args->basicAt (1).asFloatOop ().floatValue (), &temp);
        ltemp = (long)temp;
        if (canEmbed (ltemp))
            returnedObject = encValueOf ((int)temp);
        else
            returnedObject = FloatOop (temp);
    }
#endif
    return ((Oop)returnedObject);
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Float>>+
*/
Oop primFloatAdd (ProcessOop proc, ArrayOop args)
{
    double result;
    result = args->basicAt (1).asFloatOop ().floatValue ();
    result += args->basicAt (2).asFloatOop ().floatValue ();
    return ((Oop)FloatOop (result));
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Float>>-
*/
Oop primFloatSubtract (ProcessOop proc, ArrayOop args)
{
    double result;
    result = args->basicAt (1).asFloatOop ().floatValue ();
    result -= args->basicAt (2).asFloatOop ().floatValue ();
    return ((Oop)FloatOop (result));
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Float>><
*/
Oop primFloatLessThan (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () <
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Not called from the image.
*/
Oop primFloatGreaterThan (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () >
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
Oop primFloatLessOrEqual (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () <=
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
Oop primFloatGreaterOrEqual (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () >=
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called from Float>>=
*/
Oop primFloatEqual (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () ==
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Not called from the image.
*/
Oop primFloatNotEqual (ProcessOop proc, ArrayOop args)
{
    if (args->basicAt (1).asFloatOop ().floatValue () !=
        args->basicAt (2).asFloatOop ().floatValue ())
        return ((Oop)MemoryManager::objTrue);
    else
        return ((Oop)MemoryManager::objFalse);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Float>>*
*/
Oop primFloatMultiply (ProcessOop proc, ArrayOop args)
{
    double result;
    result = args->basicAt (1).asFloatOop ().floatValue ();
    result *= args->basicAt (2).asFloatOop ().floatValue ();
    return ((Oop)FloatOop (result));
}

/*
Returns the result of dividing the receiver's value by the argument's
value.
Called from Float>>/
*/
Oop primFloatDivide (ProcessOop proc, ArrayOop args)
{
    double result;
    result = args->basicAt (1).asFloatOop ().floatValue ();
    result /= args->basicAt (2).asFloatOop ().floatValue ();
    return ((Oop)FloatOop (result));
}

#define MAXFILES 32

FILE * fp[MAXFILES] = {};

/*
Opens the file denoted by the first argument, if necessary.  Some of the
characteristics of the file and/or the operations permitted on it may be
denoted by the second argument.
Returns non-nil if successful; nil otherwise.
Called from File>>open
*/
Oop primFileOpen (ProcessOop proc, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    char * p = (char *)args->basicAt (2)->asStringOop ()->vonNeumannSpace ();
    if (!strcmp (p, "stdin"))
        fp[i] = stdin;
    else if (!strcmp (p, "stdout"))
        fp[i] = stdout;
    else if (!strcmp (p, "stderr"))
        fp[i] = stderr;
    else
    {
        char * q =
            (char *)args->basicAt (2)->asStringOop ()->vonNeumannSpace ();
        char * r = strchr (q, 'b');
        ByteOop s;
        if (r == NULL)
        {
            int t = strlen (q);
            s = memMgr.allocateByteObj (t + 2);
            r = (char *)s->vonNeumannSpace ();
            memcpy (r, q, t);
            *(r + t) = 'b';
            q = r;
        }
        fp[i] = fopen (p, q);
        /* FIXME: if (r == NULL)
            isVolatilePut (s, false);*/
    }
    if (fp[i] == NULL)
        return (Oop::nilObj ());
    else
        return (SmiOop (i));
}

/*
Closes the file denoted by the receiver.
Always fails.
Called from File>>close
*/
Oop primFileClose (ProcessOop proc, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    if (fp[i])
        (void)fclose (fp[i]);
    fp[i] = NULL;
    return (Oop::nilObj ());
}

// void coldFileIn (encVal tagRef);

/*
Applies the built-in "fileIn" function to the file denoted by the
receiver.
Always fails.
Not called from the image.
N.B.:  The built-in function uses the built-in compiler.  Both should be
used only in connection with building an initial image.
*/
Oop primFileIn (ProcessOop proc, ArrayOop args)
{
    /*int i = args->basicAt (1).asSmiOop ().intValue ();
    if (fp[i])
        coldFileIn (args->basicAt (1)->val);
    return (Oop::nilObj ());*/
}

/*
Reads the next line of characters from the file denoted by the receiver.
This line usually starts with the character at the current file position
and ends with the left-most newline.  However, if reading from standard
input, the line may be continued by immediately preceding the newline
with a backslash, both of which are deleted.  Creates a new String.  The
von Neumann space of the new String is usually the characters of the
complete line followed by a null.  However, if reading from standard
input, the trailing newline is deleted.  Also, if the line includes a
null, only that portion up to the left-most null is used.
Returns the new String if successful, nil otherwise.
Called from File>>getString
*/
Oop primGetString (ProcessOop proc, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    int j;
    char buffer[4096];
    if (!fp[i])
        return (Oop::nilObj ());
    j = 0;
    buffer[j] = '\0';
    while (1)
    {
        if (fgets (&buffer[j], 512, fp[i]) == NULL)
        {
            if (fp[i] == stdin)
                (void)fputc ('\n', stdout);
            return (Oop::nilObj ()); /* end of file */
        }
        if (fp[i] == stdin)
        {
            /* delete the newline */
            j = strlen (buffer);
            if (buffer[j - 1] == '\n')
                buffer[j - 1] = '\0';
        }
        j = strlen (buffer) - 1;
        if (buffer[j] != '\\')
            break;
        /* else we loop again */
    }
    return ((Oop)StringOopDesc::fromString (buffer));
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver.
Always fails.
Called from File>>printNoReturn:
*/
Oop primPrintWithoutNL (ProcessOop proc, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue (); // intValueOf
                                                       // (arg[0].val);
    if (!fp[i])
        return (Oop ());
    (void)fputs (
        (char *)args->basicAt (2)->asByteArrayOop ()->vonNeumannSpace (),
        fp[i]);
    (void)fflush (fp[i]);
    return (Oop ());
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver and appends a newline.
Always fails.
Called from File>>print:
*/
Oop primPrintWithNL (ProcessOop proc, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    if (!fp[i])
        return (Oop ());
    (void)fputs (
        (char *)args->basicAt (2)->asByteArrayOop ()->vonNeumannSpace (),
        fp[i]);
    (void)fputc ('\n', fp[i]);
    return (Oop ());
}

Oop primExecBlock (ProcessOop proc, ArrayOop args)
{
    ContextOop ctx =
        ContextOopDesc::newWithBlock (args->basicAt (1)->asBlockOop ());
    for (int i = 2; i <= args->asMemOop ()->size (); i++)
    {
        printf ("add argument %d\n", i - 1);
        ctx->arguments ()->basicatPut (i - 1, args->basicAt (i));
    }

    ctx->setPreviousContext (proc->context ()->previousContext ());
    proc->setContext (ctx);
    // printf ("=> Entering block\n");
    return Oop ();
}

Oop primDumpVariable (ProcessOop proc, ArrayOop args)
{
    ContextOop ctx = proc->context ();

    args->basicAt (1)->print (20);
    args->basicAt (1).isa ()->print (20);
    printf ("Dump variable:\n");
    printf (
        "          --> %s>>%s\n",
        ctx->receiver ().isa ()->name ()->asCStr (),
        ctx->isBlockContext ()
            ? "<block>"
            : ctx->methodOrBlock ()->asMethodOop ()->selector ()->asCStr ());
    while ((ctx = ctx->previousContext ()) != Oop::nilObj ())
        printf ("          --> %s>>%s\n",
                ctx->receiver ().isa ()->name ()->asCStr (),
                ctx->isBlockContext () ? "<block>"
                                       : ctx->methodOrBlock ()
                                             ->asMethodOop ()
                                             ->selector ()
                                             ->asCStr ());
    return Oop ();
}

Oop primMsg (ProcessOop proc, ArrayOop args)
{
    printf ("Message: %s", args->basicAt (1)->asStringOop ()->asCStr ());
    return Oop ();
}

Oop primFatal (ProcessOop proc, ArrayOop args)
{
    printf ("Fatal error: %s\n", args->basicAt (1)->asStringOop ()->asCStr ());
    abort ();
    return Oop ();
}

Oop primExecuteNative (ProcessOop proc, ArrayOop args)
{
    args->basicAt (1)->asNativeCodeOop ()->fun () ((void *)proc.index ());
    return Oop ();
}

PrimitiveMethod * primVec[255] = {
    /*000*/ &unsupportedPrim,
    /*001*/ &unsupportedPrim,
    /*002*/ &primAvailCount,
    /*003*/ &primRandom,
    /*004*/ &unsupportedPrim,
    /*005*/ &primFlipWatching,
    /*006*/ &unsupportedPrim,
    /*007*/ &unsupportedPrim,
    /*008*/ &unsupportedPrim,
    /*009*/ &primExit,
    /*010*/ &unsupportedPrim,
    /*011*/ &primClass,
    /*012*/ &primSize,
    /*013*/ &primHash,
    /*014*/ &unsupportedPrim,
    /*015*/ &unsupportedPrim,
    /*016*/ &unsupportedPrim,
    /*017*/ &unsupportedPrim,
    /*018*/ &primBlockReturn,
    /*019*/ &primExecute,
    /*020*/ &unsupportedPrim,
    /*021*/ &primIdent,
    /*022*/ &primClassOfPut,
    /*023*/ &unsupportedPrim,
    /*024*/ &primStringCat,
    /*025*/ &primBasicAt,
    /*026*/ &primByteAt,
    /*027*/ &primSymbolAssign,
    /*028*/ &unsupportedPrim,
    /*029*/ &unsupportedPrim,
    /*030*/ &unsupportedPrim,
    /*031*/ &primBasicAtPut,
    /*032*/ &primByteAtPut,
    /*033*/ &primCopyFromTo,
    /*034*/ &unsupportedPrim,
    /*035*/ &unsupportedPrim,
    /*036*/ &unsupportedPrim,
    /*037*/ &unsupportedPrim,
    /*038*/ &unsupportedPrim, //&primFlushCache,
    /*039*/ &primParse,
    /*040*/ &unsupportedPrim,
    /*041*/ &unsupportedPrim,
    /*042*/ &unsupportedPrim,
    /*043*/ &unsupportedPrim,
    /*044*/ &unsupportedPrim, //&primSpecial,
    /*045*/ &unsupportedPrim,
    /*046*/ &unsupportedPrim,
    /*047*/ &unsupportedPrim,
    /*048*/ &unsupportedPrim,
    /*049*/ &unsupportedPrim,
    /*050*/ &unsupportedPrim,
    /*051*/ &primAsFloat,
    /*052*/ &unsupportedPrim,
    /*053*/ &primSetTimeSlice,
    /*054*/ &unsupportedPrim,
    /*055*/ &unsupportedPrim, //&primSetSeed,
    /*056*/ &unsupportedPrim,
    /*057*/ &unsupportedPrim,
    /*058*/ &primAllocOrefObj,
    /*059*/ &primAllocByteObj,
    /*060*/ &primAdd,
    /*061*/ &primSubtract,
    /*062*/ &primLessThan,
    /*063*/ &primGreaterThan,
    /*064*/ &primLessOrEqual,
    /*065*/ &primGreaterOrEqual,
    /*066*/ &primEqual,
    /*067*/ &primNotEqual,
    /*068*/ &primMultiply,
    /*069*/ &primQuotient,
    /*070*/ &primRemainder,
    /*071*/ &primBitAnd,
    /*072*/ &primBitXor,
    /*073*/ &unsupportedPrim,
    /*074*/ &unsupportedPrim,
    /*075*/ &unsupportedPrim,
    /*076*/ &unsupportedPrim,
    /*077*/ &unsupportedPrim,
    /*078*/ &unsupportedPrim,
    /*079*/ &primBitShift,
    /*080*/ &unsupportedPrim,
    /*081*/ &primStringSize,
    /*082*/ &primStringHash,
    /*083*/ &primAsSymbol,
    /*084*/ &unsupportedPrim,
    /*085*/ &unsupportedPrim,
    /*086*/ &unsupportedPrim,
    /*087*/ &unsupportedPrim,
    /*088*/ &primHostCommand,
    /*089*/ &unsupportedPrim,
    /*090*/ &unsupportedPrim,
    /*091*/ &unsupportedPrim,
    /*092*/ &unsupportedPrim,
    /*093*/ &unsupportedPrim,
    /*094*/ &unsupportedPrim,
    /*095*/ &unsupportedPrim,
    /*096*/ &unsupportedPrim,
    /*097*/ &unsupportedPrim,
    /*098*/ &unsupportedPrim,
    /*099*/ &unsupportedPrim,
    /*100*/ &unsupportedPrim,
    /*101*/ &primAsString,
    /*102*/ &primNaturalLog,
    /*103*/ &primERaisedTo,
    /*104*/ &unsupportedPrim,
    /*105*/ &unsupportedPrim,
    /*106*/ &primIntegerPart,
    /*107*/ &unsupportedPrim,
    /*108*/ &unsupportedPrim,
    /*109*/ &unsupportedPrim,
    /*110*/ &primFloatAdd,
    /*111*/ &primFloatSubtract,
    /*112*/ &primFloatLessThan,
    /*113*/ &primFloatGreaterThan,
    /*114*/ &primFloatLessOrEqual,
    /*115*/ &primFloatGreaterOrEqual,
    /*116*/ &primFloatEqual,
    /*117*/ &primFloatNotEqual,
    /*118*/ &primFloatMultiply,
    /*119*/ &primFloatDivide,
    /*120*/ &primFileOpen,
    /*121*/ &primFileClose,
    /*122*/ &unsupportedPrim,
    /*123*/ &primFileIn,
    /*124*/ &unsupportedPrim,
    /*125*/ &primGetString,
    /*126*/ &unsupportedPrim,
    /*127*/ &unsupportedPrim, //&primImageWrite,
    /*128*/ &primPrintWithoutNL,
    /*129*/ &primPrintWithNL,
    /*130*/ &unsupportedPrim,
    /*131*/ &unsupportedPrim,
    /*132*/ &unsupportedPrim,
    /*133*/ &unsupportedPrim,
    /*134*/ &unsupportedPrim,
    /*135*/ &unsupportedPrim,
    /*136*/ &unsupportedPrim,
    /*137*/ &unsupportedPrim,
    /*138*/ &unsupportedPrim,
    /*139*/ &unsupportedPrim,
    /*140*/ &unsupportedPrim,
    /*141*/ &unsupportedPrim,
    /*142*/ &unsupportedPrim,
    /*143*/ &unsupportedPrim,
    /*144*/ &unsupportedPrim,
    /*145*/ &unsupportedPrim,
    /*146*/ &unsupportedPrim,
    /*147*/ &unsupportedPrim,
    /*148*/ &unsupportedPrim,
    /*149*/ &unsupportedPrim,
    /*150*/ &unsupportedPrim,
    /*151*/ &unsupportedPrim, //&primSetTrace,
    /*152*/ &unsupportedPrim, //&primError,
    /*153*/ &unsupportedPrim, //&primReclaim,
    /*154*/ &unsupportedPrim, //&primLogChunk,
    /*155*/ &unsupportedPrim,
    /*156*/ &unsupportedPrim,
    /*157*/ &unsupportedPrim, //&primGetChunk,
    /*158*/ &unsupportedPrim, //&primPutChunk,
    /*159*/ &unsupportedPrim,
    /*160*/ &primExecBlock,
    /*161*/ &primDumpVariable,
    /*162*/ &primMsg,
    /*163*/ &primFatal,
    /*164*/ &primExecuteNative,
};