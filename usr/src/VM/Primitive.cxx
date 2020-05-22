#include <cmath>
#include <csetjmp>

#include "Interpreter.hxx"

Oop unsupportedPrim (ExecState & es, ArrayOop args)
{
    return (Oop::nil ());
}

/*
Prints the number of available object table entries.
Always fails.
Called from Scheduler>>initialize
*/
Oop primAvailCount (ExecState & es, ArrayOop args)
{
    // fprintf (stderr, "free: %d\n", availCount ());
    return (Oop::nil ());
}

/*
Returns a pseudo-random integer.
Called from
  Random>>next
  Random>>randInteger:
*/
Oop primRandom (ExecState & es, ArrayOop args)
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
Oop primFlipWatching (ExecState & es, ArrayOop args)
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
Oop primExit (ExecState & es, ArrayOop args)
{
    exit (0);
}

/*
Returns the class of which the receiver is an instance.
Called from Object>>class
*/
Oop primClass (ExecState & es, ArrayOop args)
{
    return (args->basicAt (1).isa ());
}

/*
Returns the field count of the von Neumann space of the receiver.
Called from Object>>basicSize
*/
Oop primSize (ExecState & es, ArrayOop args)
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
Oop primHash (ExecState & es, ArrayOop args)
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
Oop primBlockReturn (ExecState & es, ArrayOop args)
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
Oop primExecute (ExecState & es, ArrayOop args)
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
Oop primIdent (ExecState & es, ArrayOop args)
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
Oop primClassOfPut (ExecState & es, ArrayOop args)
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
Oop primStringCat (ExecState & es, ArrayOop args)
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
Oop primBasicAt (ExecState & es, ArrayOop args)
{
    int i;
    if (args->basicAt (1).isInteger ())
    {
        printf ("*************\n\n\n\nARG 1 is an integer!\n\n\n\n******\n\n");
        return (Oop::nil ());
    }
    /* if (!args->basicAt (1)->kind == OopsRefObj)
        return (Oop::nil ()); */
    if (!args->basicAt (2).isInteger ())
    {
        printf ("*************\n\n\n\nARG 2 isn't integer!\n\n\n\n******\n\n");
        return (Oop::nil ());
    }
    i = args->basicAt (2).asSmiOop ().intValue ();
    if (i < 1 || i > args->basicAt (1)->asMemOop ()->size ())
    {
        printf ("*************\n\n\n\nARG II (%d) out of "
                "goose!!!\n\n\n\n******\n\n",
                i);
        return (Oop::nil ());
    }
    printf ("*************\nBasicAt: %d\n******\n\n", i);
    // args->basicAt (1)->asOopOop ()->basicAt (i)->print (10);

    return args->basicAt (1)->asOopOop ()->basicAt (i);
}

/*
Returns an encoded representation of the byte of the receiver denoted by
the argument.
Called from ByteArray>>basicAt:
*/
Oop primByteAt (ExecState & es, ArrayOop args) /*fix*/
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
Oop primSymbolAssign (ExecState & es, ArrayOop args) /*fix*/
{
    MemoryManager::objGlobals->symbolInsert (args->basicAt (1)->asSymbolOop (),
                                             args->basicAt (2));
    return (args->basicAt (1));
}

/*
Changes the active process stack.  The change causes control to be
returned in the method containing the block controlled by the receiver
rather than the method which sent the message (e.g. Block>>value) which
created the context which invoked this primitive.  Execution will resume
at the location denoted by the first argument.
Called from Context>>returnToBlock:
N.B.:  The code involved here isn't quite as tricky as that involved
in primBlockReturn (q.v.).
*/
Oop primBlockCall (ExecState & es, ArrayOop args) /*fix*/
{
    int i;
    /* first get previous link */
    // FIXME:  i = intValueOf (orefOf (processStack, linkPointer).val);
    /* change context and byte pointer */
    /// orefOfPut (processStack, i + 1, args->basicAt (1));
    // orefOfPut (processStack, i + 4, args->basicAt (2));
    return (args->basicAt (1));
}

/*
Returns a modified copy of the receiver.  The receiver is a block.  The
modification defines the controlling context of the clone to be the
argument.  The argument is the current context and is the target of any
"^" return eventually invoked by the receiver.
This primitive is called by compiler-generated code.
N.B.:  The code involved here isn't quite as tricky as that involved
in primBlockReturn (q.v.).
*/
Oop primBlockClone (ExecState & es, ArrayOop args) /*fix*/
{
    Oop returnedObject;
    // FIXME: returnedObject = (Oop)newBlock ();
    // orefOfPut (returnedObject.ptr, 1, args->basicAt (2));
    // orefOfPut (returnedObject.ptr, 2, orefOf (args->basicAt (1)->ptr, 2));
    // orefOfPut (returnedObject.ptr, 3, orefOf (args->basicAt (1)->ptr, 3));
    // orefOfPut (returnedObject.ptr, 4, orefOf (args->basicAt (1)->ptr, 4));
    return (returnedObject);
}

/*
Defines the Oop of the receiver denoted by the first argument to be
the second argument.
Returns the receiver.
Called from Object>>basicAt:put:
*/
Oop primBasicAtPut (ExecState & es, ArrayOop args)
{
    int i;
    if (args->basicAt (1).isInteger ())
        return (Oop::nil ());
    /* if (!args->basicAt (1)->kind == OopsRefObj)
        return (Oop::nil ()); */
    if (!args->basicAt (2).isInteger ())
        return (Oop::nil ());
    i = args->basicAt (2).asSmiOop ().intValue ();
    if (i < 1 || i > args->basicAt (1)->asMemOop ()->size ())
        return (Oop::nil ());
    args->basicAt (1)->asOopOop ()->basicatPut (i, args->basicAt (3));
    return args->basicAt (1);
}

/*
Defines the byte of the receiver denoted by the first argument to be a
decoded representation of the second argument.
Returns the receiver.
Called from ByteArray>>basicAt:put:
*/
Oop primByteAtPut (ExecState & es, ArrayOop args) /*fix*/
{
    int i;
    printf ("ByteAtPut %d %d\n", args->basicAt (2).asSmiOop ().intValue ());
    if (!args->basicAt (2).isInteger ())
        perror ("non integer index byteAt:");
    if (!args->basicAt (3).isInteger ())
        perror ("assigning non int to byte");
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
Oop primCopyFromTo (ExecState & es, ArrayOop args) /*fix*/
{
    if ((!args->basicAt (2).isInteger () || (!args->basicAt (3).isInteger ())))
        perror ("non integer index / copyFromTo");
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

Oop primParse (ExecState & es, ArrayOop args) /*del*/
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
Oop primAsFloat (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nil ());
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
Oop primSetTimeSlice (ExecState & es, ArrayOop args)
{
    /*FIXME: if (!args->basicAt (1).isInteger ()))
        return (Oop::nil ());
    *counterAddress = args->basicAt (1).asSmiOop ().intValue ();*/
    return (Oop::nil ());
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of Oops.  The number is denoted by the
receiver.
Called from
  BlockNode>>newBlock
  Class>>new:
*/
Oop primAllocOrefObj (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nil ());
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
Oop primAllocByteObj (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger ())
        return (Oop::nil ());
    return ((Oop)memMgr.allocateByteObj (
        args->basicAt (1).asSmiOop ().intValue ()));
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Integer>>+
Also called for SendBinary bytecodes.
*/
Oop primAdd (ExecState & es, ArrayOop args)
{
    long longresult;

    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult += args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: bounds test SMI 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nil ());
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Integer>>-
Also called for SendBinary bytecodes.
*/
Oop primSubtract (ExecState & es, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult -= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: smi boundcheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nil ());
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Integer>><
Also called for SendBinary bytecodes.
*/
Oop primLessThan (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primGreaterThan (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primLessOrEqual (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primGreaterOrEqual (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primEqual (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primNotEqual (ExecState & es, ArrayOop args)
{
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primMultiply (ExecState & es, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult *= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nil ());
}

/*
Returns the quotient of the result of dividing the receiver's value by
the argument's value.
Called from Integer>>quo:
Also called for SendBinary bytecodes.
*/
Oop primQuotient (ExecState & es, ArrayOop args)
{
    long longresult;
    printf ("PRIMQUO\n\n");
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
    if (args->basicAt (2).asSmiOop ().intValue () == 0)
        return (Oop::nil ());
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult /= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nil ());
}

/*
Returns the remainder of the result of dividing the receiver's value by
the argument's value.
Called for SendBinary bytecodes.
*/
Oop primRemainder (ExecState & es, ArrayOop args)
{
    long longresult;
    printf ("PRIMREM\n\n");

    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
    {
        printf ("Unacceptable! Args:\n");

        // args->basicAt (1)->print (15);
        // args->basicAt (2)->print (15);
        return (Oop::nil ());
    }
    if (args->basicAt (2).asSmiOop ().intValue () == 0)
    {
        printf ("Unacceptable! Argsat 2 is 0.\n");
        return (Oop::nil ());
    }
    longresult = args->basicAt (1).asSmiOop ().intValue ();
    longresult %= args->basicAt (2).asSmiOop ().intValue ();
    if (1) // FIXME: boundscheck smi
        return (SmiOop (longresult));
    else
        return (Oop::nil ());
}

/*
Returns the bit-wise "and" of the receiver's value and the argument's
value.
Called from Integer>>bitAnd:
Also called for SendBinary bytecodes.
*/
Oop primBitAnd (ExecState & es, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primBitXor (ExecState & es, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primBitShift (ExecState & es, ArrayOop args)
{
    long longresult;
    if (!args->basicAt (1).isInteger () || !args->basicAt (2).isInteger ())
        return (Oop::nil ());
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
Oop primStringSize (ExecState & es, ArrayOop args)
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
Oop primStringHash (ExecState & es, ArrayOop args)
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
Oop primAsSymbol (ExecState & es, ArrayOop args)
{
    return ((Oop)SymbolOopDesc::fromString (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ()));
}

/*
Returns the object associated with the receiver in the global symbol
table.
Called from Symbol>>value
*/
Oop primGlobalValue (ExecState & es, ArrayOop args)
{
    printf ("Requested global value of %s\n",
            (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ());
    return (Oop::nil ());
    // FIXME: (Oop)globalValue ((char *)vonNeumannSpaceOf (args->basicAt
    // (1)->ptr)));
}

/*
Passes the von Neumann space of the receiver to the host's "system"
function.  Returns what that function returns.
Called from String>>unixCommand
*/
Oop primHostCommand (ExecState & es, ArrayOop args)
{
    return (SmiOop (system (
        (char *)args->basicAt (1)->asStringOop ()->vonNeumannSpace ())));
}

/*
Returns the equivalent of the receiver's value in a printable character
representation.
Called from Float>>printString
*/
Oop primAsString (ExecState & es, ArrayOop args)
{
    char buffer[32];
    (void)sprintf (buffer, "%g", args->basicAt (1).asFloatOop ().floatValue ());
    return ((Oop)StringOopDesc::fromString (buffer));
}

/*
Returns the natural logarithm of the receiver's value.
Called from Float>>ln
*/
Oop primNaturalLog (ExecState & es, ArrayOop args)
{
    return (
        (Oop)FloatOop (log (args->basicAt (1).asFloatOop ().floatValue ())));
}

/*
Returns "e" raised to a power denoted by the receiver's value.
Called from Float>>exp
*/
Oop primERaisedTo (ExecState & es, ArrayOop args)
{
    return (
        (Oop)FloatOop (exp (args->basicAt (1).asFloatOop ().floatValue ())));
}

/*
Returns a new Array containing two integers n and m such that the
receiver's value can be expressed as n * 2**m.
Called from Float>>integerPart
*/
Oop primIntegerPart (ExecState & es, ArrayOop args)
{
    double temp;
    int i;
    int j;
    ArrayOop returnedObject = Oop::nil ()->asArrayOop ();
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
Oop primFloatAdd (ExecState & es, ArrayOop args)
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
Oop primFloatSubtract (ExecState & es, ArrayOop args)
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
Oop primFloatLessThan (ExecState & es, ArrayOop args)
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
Oop primFloatGreaterThan (ExecState & es, ArrayOop args)
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
Oop primFloatLessOrEqual (ExecState & es, ArrayOop args)
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
Oop primFloatGreaterOrEqual (ExecState & es, ArrayOop args)
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
Oop primFloatEqual (ExecState & es, ArrayOop args)
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
Oop primFloatNotEqual (ExecState & es, ArrayOop args)
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
Oop primFloatMultiply (ExecState & es, ArrayOop args)
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
Oop primFloatDivide (ExecState & es, ArrayOop args)
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
Oop primFileOpen (ExecState & es, ArrayOop args)
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
        return (Oop::nil ());
    else
        return (SmiOop (i));
}

/*
Closes the file denoted by the receiver.
Always fails.
Called from File>>close
*/
Oop primFileClose (ExecState & es, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    if (fp[i])
        (void)fclose (fp[i]);
    fp[i] = NULL;
    return (Oop::nil ());
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
Oop primFileIn (ExecState & es, ArrayOop args)
{
    /*int i = args->basicAt (1).asSmiOop ().intValue ();
    if (fp[i])
        coldFileIn (args->basicAt (1)->val);
    return (Oop::nil ());*/
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
Oop primGetString (ExecState & es, ArrayOop args)
{
    int i = args->basicAt (1).asSmiOop ().intValue ();
    int j;
    char buffer[4096];
    if (!fp[i])
        return (Oop::nil ());
    j = 0;
    buffer[j] = '\0';
    while (1)
    {
        if (fgets (&buffer[j], 512, fp[i]) == NULL)
        {
            if (fp[i] == stdin)
                (void)fputc ('\n', stdout);
            return (Oop::nil ()); /* end of file */
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
Oop primPrintWithoutNL (ExecState & es, ArrayOop args)
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
Oop primPrintWithNL (ExecState & es, ArrayOop args)
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

Oop primExecBlock (ExecState & es, ArrayOop args)
{
    ContextOop ctx =
        ContextOopDesc::newWithBlock (args->basicAt (1)->asBlockOop ());
    for (int i = 2; i <= args->asMemOop ()->size (); i++)
    {
        printf ("add argument %d\n", i - 1);
        ctx->arguments ()->basicatPut (i - 1, args->basicAt (i));
    }

    ctx->setPreviousContext (es.proc->context ()->previousContext ());
    es.proc->setContext (ctx);
    printf ("=> Entering block\n");
    return Oop ();
}

Oop primDumpVariable (ExecState & es, ArrayOop args)
{
    ContextOop ctx = es.proc->context ();
    printf ("\n\n\n\n!!!!!!!!!!!!!!!!!!!!\n%d\n", args->basicAt (1));

    args->basicAt (1)->print (20);
    args->basicAt (1).isa ()->print (20);
    printf ("!!!!!!!!!!!!!!!!!!!!\n\n\n\n\n");
    printf (
        "          --> %s>>%s\n",
        ctx->receiver ().isa ()->name ()->asCStr (),
        ctx->isBlockContext ()
            ? "<block>"
            : ctx->methodOrBlock ()->asMethodOop ()->selector ()->asCStr ());
    while ((ctx = ctx->previousContext ()) != Oop::nil ())
        printf ("          --> %s>>%s\n",
                ctx->receiver ().isa ()->name ()->asCStr (),
                ctx->isBlockContext () ? "<block>"
                                       : ctx->methodOrBlock ()
                                             ->asMethodOop ()
                                             ->selector ()
                                             ->asCStr ());
    return Oop ();
}

Oop primMsg (ExecState & es, ArrayOop args)
{
    printf ("!!\n\nMessage: '%s'\n\n\n!!\n\n",
            args->basicAt (1)->asStringOop ()->asCStr ());
    return Oop ();
}

Oop primFatal (ExecState & es, ArrayOop args)
{
    printf ("!!\n\nFatal error: '%s'\n\n\n!!\n\n",
            args->basicAt (1)->asStringOop ()->asCStr ());
    abort ();
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
    /*028*/ &primBlockCall,
    /*029*/ &primBlockClone,
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
    /*087*/ &primGlobalValue,
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
};