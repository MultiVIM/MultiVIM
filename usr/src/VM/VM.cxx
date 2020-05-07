#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "AST.hxx"
#include "Bytecode.hxx"
#include "NewCompiler.h"
#include "VM.hxx"

#define streq(a, b) (strcmp (a, b) == 0)

extern word traceVect[];

#define traceSize 3
#define execTrace traceVect[0]
#define primTrace traceVect[1]
#define mselTrace traceVect[2]

__INLINE__ objRef unsupportedPrim (objRef arg[])
{
    return ((objRef)nilObj);
}

/*
Prints the number of available object table entries.
Always fails.
Called from Scheduler>>initialize
*/
objRef primAvailCount (objRef arg[])
{
    fprintf (stderr, "free: %d\n", availCount ());
    return ((objRef)nilObj);
}

/*
Returns a pseudo-random integer.
Called from
  Random>>next
  Random>>randInteger:
*/
objRef primRandom (objRef arg[])
{
    short i;
    /* this is hacked because of the representation */
    /* of integers as shorts */
    i = rand () >> 8; /* strip off lower bits */
    if (i < 0)
        i = -i;
    return ((objRef)encValueOf (i >> 1));
}

extern bool watching;

/*
Inverts the state of a switch.  The switch controls, in part, whether or
not "watchWith:" messages are sent to Methods during execution.
Returns the Boolean representation of the switch value after the invert.
Called from Smalltalk>>watch
*/
objRef primFlipWatching (objRef arg[])
{
    watching = !watching;
    return ((objRef) (watching ? trueObj : falseObj));
}

/*
Terminates the interpreter.
Never returns.
Not called from the image.
*/
objRef primExit (objRef arg[])
{
    exit (0);
}

/*
Returns the class of which the receiver is an instance.
Called from Object>>class
*/
objRef primClass (objRef arg[])
{
    return ((objRef)getClass (arg[0]));
}

/*
Returns the field count of the von Neumann space of the receiver.
Called from Object>>basicSize
*/
objRef primSize (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        i = 0;
    else
        i = countOf (arg[0].ptr);
    return ((objRef)encValueOf (i));
}

/*
Returns a hashed representation of the receiver.
Called from Object>>hash
*/
objRef primHash (objRef arg[])
{
    if (isValue (arg[0]))
        return (arg[0]);
    else
        return ((objRef)encValueOf (oteIndexOf (arg[0].ptr)));
}

extern encPtr processStack;
extern int linkPointer;
int * counterAddress = NULL;

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
objRef primBlockReturn (objRef arg[])
{
    int i;
    int j;
    /* first get previous link pointer */
    i = intValueOf (orefOf (processStack, linkPointer).val);
    /* then creating context pointer */
    j = intValueOf (orefOf (arg[0].ptr, 1).val);
    if (ptrNe (orefOf (processStack, j + 1), arg[0]))
        return ((objRef)falseObj);
    /* first change link pointer to that of creator */
    orefOfPut (processStack, i, orefOf (processStack, j));
    /* then change return point to that of creator */
    orefOfPut (processStack, i + 2, orefOf (processStack, j + 2));
    return ((objRef)trueObj);
}

jmp_buf jb = {};

void brkfun (int sig)
{
    longjmp (jb, 1);
}

void brkignore (int sig)
{
}

bool execute (encPtr aProcess, int maxsteps);

/*
Executes the receiver until its time slice is ended or terminated.
Returns true in the former case; false in the latter.
Called from Process>>execute
*/
objRef primExecute (objRef arg[])
{
    encPtr saveProcessStack;
    int saveLinkPointer;
    int * saveCounterAddress;
    objRef returnedObject;
    /* first save the values we are about to clobber */
    saveProcessStack = processStack;
    saveLinkPointer = linkPointer;
    saveCounterAddress = counterAddress;
    /* trap control-C */
    signal (SIGINT, brkfun);
    if (setjmp (jb))
        returnedObject = (objRef)falseObj;
    else if (execute (arg[0].ptr, 1 << 12))
        returnedObject = (objRef)trueObj;
    else
        returnedObject = (objRef)falseObj;
    signal (SIGINT, brkignore);
    /* then restore previous environment */
    processStack = saveProcessStack;
    linkPointer = saveLinkPointer;
    counterAddress = saveCounterAddress;
    return (returnedObject);
}

/*
Returns true if the content of the receiver's objRef is equal to that
of the first argument's; false otherwise.
Called from Object>>==
*/
objRef primIdent (objRef arg[])
{
    if (ptrEq (arg[0], arg[1]))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
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
objRef primClassOfPut (objRef arg[])
{
    classOfPut (arg[0].ptr, arg[1].ptr);
    return (arg[0]);
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
objRef primStringCat (objRef arg[])
{
    HostMemoryAddress src1 = vonNeumannSpaceOf (arg[0].ptr);
    word len1 = strlen ((char *)src1);
    HostMemoryAddress src2 = vonNeumannSpaceOf (arg[1].ptr);
    word len2 = strlen ((char *)src2);
    encPtr ans = allocByteObj (len1 + len2 + 1);
    HostMemoryAddress tgt = vonNeumannSpaceOf (ans);
    (void)memcpy (tgt, src1, len1);
    (void)memcpy (((byte *)tgt) + len1, src2, len2);
    if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
        stringClass = globalValue ("String");
    classOfPut (ans, stringClass);
    return ((objRef)ans);
}

/*
Returns the objRef of the receiver denoted by the argument.
Called from Object>>basicAt:
*/
objRef primBasicAt (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        return ((objRef)nilObj);
    if (!isObjRefs (arg[0].ptr))
        return ((objRef)nilObj);
    if (isIndex (arg[1]))
        return ((objRef)nilObj);
    i = intValueOf (arg[1].val);
    if (i < 1 || i > countOf (arg[0].ptr))
        return ((objRef)nilObj);
    return (orefOf (arg[0].ptr, i));
}

/*
Returns an encoded representation of the byte of the receiver denoted by
the argument.
Called from ByteArray>>basicAt:
*/
objRef primByteAt (objRef arg[]) /*fix*/
{
    int i;
    if (isIndex (arg[1]))
        sysError ("non integer index", "byteAt:");
    i = byteOf (arg[0].ptr, intValueOf (arg[1].val));
    if (i < 0)
        i += 256;
    return ((objRef)encValueOf (i));
}

/*
Defines the global value of the receiver to be the first argument.
Returns the receiver.
Called from Symbol>>assign:
*/
objRef primSymbolAssign (objRef arg[]) /*fix*/
{
    nameTableInsert (symbols,
                     strHash ((char *)vonNeumannSpaceOf (arg[0].ptr)),
                     arg[0].ptr,
                     arg[1].ptr);
    return (arg[0]);
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
objRef primBlockCall (objRef arg[]) /*fix*/
{
    int i;
    /* first get previous link */
    i = intValueOf (orefOf (processStack, linkPointer).val);
    /* change context and byte pointer */
    orefOfPut (processStack, i + 1, arg[0]);
    orefOfPut (processStack, i + 4, arg[1]);
    return (arg[0]);
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
objRef primBlockClone (objRef arg[]) /*fix*/
{
    objRef returnedObject;
    returnedObject = (objRef)newBlock ();
    orefOfPut (returnedObject.ptr, 1, arg[1]);
    orefOfPut (returnedObject.ptr, 2, orefOf (arg[0].ptr, 2));
    orefOfPut (returnedObject.ptr, 3, orefOf (arg[0].ptr, 3));
    orefOfPut (returnedObject.ptr, 4, orefOf (arg[0].ptr, 4));
    return (returnedObject);
}

/*
Defines the objRef of the receiver denoted by the first argument to be
the second argument.
Returns the receiver.
Called from Object>>basicAt:put:
*/
objRef primBasicAtPut (objRef arg[])
{
    int i;
    if (isValue (arg[0]))
        return ((objRef)nilObj);
    if (!isObjRefs (arg[0].ptr))
        return ((objRef)nilObj);
    if (isIndex (arg[1]))
        return ((objRef)nilObj);
    i = intValueOf (arg[1].val);
    if (i < 1 || i > countOf (arg[0].ptr))
        return ((objRef)nilObj);
    orefOfPut (arg[0].ptr, i, arg[2]);
    return (arg[0]);
}

/*
Defines the byte of the receiver denoted by the first argument to be a
decoded representation of the second argument.
Returns the receiver.
Called from ByteArray>>basicAt:put:
*/
objRef primByteAtPut (objRef arg[]) /*fix*/
{
    if (isIndex (arg[1]))
        sysError ("non integer index", "byteAtPut");
    if (isIndex (arg[2]))
        sysError ("assigning non int", "to byte");
    byteOfPut (arg[0].ptr, intValueOf (arg[1].val), intValueOf (arg[2].val));
    return (arg[0]);
}

__INLINE__ word min (word one, word two)
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
objRef primCopyFromTo (objRef arg[]) /*fix*/
{
    if ((isIndex (arg[1])) || (isIndex (arg[2])))
        sysError ("non integer index", "copyFromTo");
    {
        HostMemoryAddress src = vonNeumannSpaceOf (arg[0].ptr);
        word len = strlen ((char *)src);
        int pos1 = intValueOf (arg[1].val);
        int pos2 = intValueOf (arg[2].val);
        int req = pos2 + 1 - pos1;
        word act;
        encPtr ans;
        HostMemoryAddress tgt;
        if (pos1 >= 1 && pos1 <= len && req >= 1)
            act = min (req, strlen (((char *)src) + (pos1 - 1)));
        else
            act = 0;
        ans = allocByteObj (act + 1);
        tgt = vonNeumannSpaceOf (ans);
        (void)memcpy (tgt, ((byte *)src) + (pos1 - 1), act);
        if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
            stringClass = globalValue ("String");
        classOfPut (ans, stringClass);
        return ((objRef)ans);
    }
}

void flushCache (encPtr messageToSend, encPtr klass);

/*
Kills the cache slot denoted by the receiver and argument.  The receiver
should be a message selector symbol.  The argument should be a class.
Returns the receiver.
Called from Class>>install:
*/
objRef primFlushCache (objRef arg[])
{
    if (isValue (arg[0]) || isValue (arg[1]))
        return ((objRef)nilObj);
    flushCache (arg[0].ptr, arg[1].ptr);
    return (arg[0]);
}

objRef primParse (objRef arg[]) /*del*/
{
    /*setInstanceVariables (arg[0].ptr);
    if (parse (arg[2].ptr, (char *)vonNeumannSpaceOf (arg[1].ptr), false))
    {
        flushCache (orefOf (arg[2].ptr, messageInMethod).ptr, arg[0].ptr);
        return ((objRef)trueObj);
    }
    else
        return ((objRef)falseObj);*/
}

/*
Returns the equivalent of the receiver's value in a floating-point
representation.
Called from Integer>>asFloat
*/
objRef primAsFloat (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)newFloat ((double)intValueOf (arg[0].val)));
}

/*
Defines a counter to be the argument's value.  When this counter is
less than 1, a Process time slice is finished.
Always fails.
Called from
  Scheduler>>critical:
  Scheduler>>yield
*/
objRef primSetTimeSlice (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    *counterAddress = intValueOf (arg[0].val);
    return ((objRef)nilObj);
}

/*
Sets the seed for a pseudo-random number generator.
Always fails.
Called from Random>>set:
*/
objRef primSetSeed (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    (void)srand ((unsigned)intValueOf (arg[0].val));
    return ((objRef)nilObj);
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of objRefs.  The number is denoted by the
receiver.
Called from
  BlockNode>>newBlock
  Class>>new:
*/
objRef primAllocOrefObj (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)allocOrefObj (intValueOf (arg[0].val)));
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of bytes.  The number is denoted by the
receiver.
Called from
  ByteArray>>size:
*/
objRef primAllocByteObj (objRef arg[])
{
    if (isIndex (arg[0]))
        return ((objRef)nilObj);
    return ((objRef)allocByteObj (intValueOf (arg[0].val)));
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Integer>>+
Also called for SendBinary bytecodes.
*/
objRef primAdd (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult += intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Integer>>-
Also called for SendBinary bytecodes.
*/
objRef primSubtract (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult -= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Integer>><
Also called for SendBinary bytecodes.
*/
objRef primLessThan (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) < intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Called from Integer>>>
Also called for SendBinary bytecodes.
*/
objRef primGreaterThan (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) > intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primLessOrEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) <= intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primGreaterOrEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) >= intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called for SendBinary bytecodes.
*/
objRef primEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) == intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Called for SendBinary bytecodes.
*/
objRef primNotEqual (objRef arg[])
{
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[0].val) != intValueOf (arg[1].val))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Integer>>*
Also called for SendBinary bytecodes.
*/
objRef primMultiply (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult *= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the quotient of the result of dividing the receiver's value by
the argument's value.
Called from Integer>>quo:
Also called for SendBinary bytecodes.
*/
objRef primQuotient (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[1].val) == 0)
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult /= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the remainder of the result of dividing the receiver's value by
the argument's value.
Called for SendBinary bytecodes.
*/
objRef primRemainder (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    if (intValueOf (arg[1].val) == 0)
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult %= intValueOf (arg[1].val);
    if (canEmbed (longresult))
        return ((objRef)encValueOf (longresult));
    else
        return ((objRef)nilObj);
}

/*
Returns the bit-wise "and" of the receiver's value and the argument's
value.
Called from Integer>>bitAnd:
Also called for SendBinary bytecodes.
*/
objRef primBitAnd (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult &= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the bit-wise "exclusive or" of the receiver's value and the
argument's value.
Called from Integer>>bitXor:
Also called for SendBinary bytecodes.
*/
objRef primBitXor (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    longresult ^= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the result of shifting the receiver's value a number of bit
positions denoted by the argument's value.  Positive arguments cause
left shifts.  Negative arguments cause right shifts.  Note that the
result is truncated to the range of embeddable values.
Called from Integer>>bitXor:
*/
objRef primBitShift (objRef arg[])
{
    long longresult;
    if (isIndex (arg[0]) || isIndex (arg[1]))
        return ((objRef)nilObj);
    longresult = intValueOf (arg[0].val);
    if (intValueOf (arg[1].val) < 0)
        longresult >>= -intValueOf (arg[1].val);
    else
        longresult <<= intValueOf (arg[1].val);
    return ((objRef)encValueOf (longresult));
}

/*
Returns the field count of the von Neumann space of the receiver up to
the left-most null.
Called from String>>size
*/
objRef primStringSize (objRef arg[])
{
    return (
        (objRef)encValueOf (strlen ((char *)vonNeumannSpaceOf (arg[0].ptr))));
}

/*
Returns a hashed representation of the von Neumann space of the receiver
up to the left-most null.
Called from
  String>>hash
  Symbol>>stringHash
*/
objRef primStringHash (objRef arg[])
{
    return (
        (objRef)encValueOf (strHash ((char *)vonNeumannSpaceOf (arg[0].ptr))));
}

/*
Returns a unique object.  Here, "unique" is determined by the
von Neumann space of the receiver up to the left-most null.  A copy will
either be found in or added to the global symbol table.  The returned
object will refer to the copy.
Called from String>>asSymbol
*/
objRef primAsSymbol (objRef arg[])
{
    return ((objRef)newSymbol ((char *)vonNeumannSpaceOf (arg[0].ptr)));
}

/*
Returns the object associated with the receiver in the global symbol
table.
Called from Symbol>>value
*/
objRef primGlobalValue (objRef arg[])
{
    return ((objRef)globalValue ((char *)vonNeumannSpaceOf (arg[0].ptr)));
}

/*
Passes the von Neumann space of the receiver to the host's "system"
function.  Returns what that function returns.
Called from String>>unixCommand
*/
objRef primHostCommand (objRef arg[])
{
    return (
        (objRef)encValueOf (system ((char *)vonNeumannSpaceOf (arg[0].ptr))));
}

/*
Returns the equivalent of the receiver's value in a printable character
representation.
Called from Float>>printString
*/
objRef primAsString (objRef arg[])
{
    char buffer[32];
    (void)sprintf (buffer, "%g", floatValue (arg[0].ptr));
    return ((objRef)newString (buffer));
}

/*
Returns the natural logarithm of the receiver's value.
Called from Float>>ln
*/
objRef primNaturalLog (objRef arg[])
{
    return ((objRef)newFloat (log (floatValue (arg[0].ptr))));
}

/*
Returns "e" raised to a power denoted by the receiver's value.
Called from Float>>exp
*/
objRef primERaisedTo (objRef arg[])
{
    return ((objRef)newFloat (exp (floatValue (arg[0].ptr))));
}

/*
Returns a new Array containing two integers n and m such that the
receiver's value can be expressed as n * 2**m.
Called from Float>>integerPart
*/
objRef primIntegerPart (objRef arg[])
{
    double temp;
    int i;
    int j;
    encPtr returnedObject = nilObj;
#define ndif 12
    temp = frexp (floatValue (arg[0].ptr), &i);
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
    returnedObject = newArray (2);
    orefOfPut (returnedObject, 1, (objRef)encValueOf (j));
    orefOfPut (returnedObject, 2, (objRef)encValueOf (i));
#ifdef trynew
    /* if number is too big it can't be integer anyway */
    if (floatValue (arg[0].ptr) > 2e9)
        returnedObject = nilObj;
    else
    {
        (void)modf (floatValue (arg[0].ptr), &temp);
        ltemp = (long)temp;
        if (canEmbed (ltemp))
            returnedObject = encValueOf ((int)temp);
        else
            returnedObject = newFloat (temp);
    }
#endif
    return ((objRef)returnedObject);
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Float>>+
*/
objRef primFloatAdd (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result += floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Float>>-
*/
objRef primFloatSubtract (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result -= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Float>><
*/
objRef primFloatLessThan (objRef arg[])
{
    if (floatValue (arg[0].ptr) < floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Not called from the image.
*/
objRef primFloatGreaterThan (objRef arg[])
{
    if (floatValue (arg[0].ptr) > floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
objRef primFloatLessOrEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) <= floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
objRef primFloatGreaterOrEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) >= floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called from Float>>=
*/
objRef primFloatEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) == floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Not called from the image.
*/
objRef primFloatNotEqual (objRef arg[])
{
    if (floatValue (arg[0].ptr) != floatValue (arg[1].ptr))
        return ((objRef)trueObj);
    else
        return ((objRef)falseObj);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Float>>*
*/
objRef primFloatMultiply (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result *= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
}

/*
Returns the result of dividing the receiver's value by the argument's
value.
Called from Float>>/
*/
objRef primFloatDivide (objRef arg[])
{
    double result;
    result = floatValue (arg[0].ptr);
    result /= floatValue (arg[1].ptr);
    return ((objRef)newFloat (result));
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
objRef primFileOpen (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    char * p = (char *)vonNeumannSpaceOf (arg[1].ptr);
    if (streq (p, "stdin"))
        fp[i] = stdin;
    else if (streq (p, "stdout"))
        fp[i] = stdout;
    else if (streq (p, "stderr"))
        fp[i] = stderr;
    else
    {
        char * q = (char *)vonNeumannSpaceOf (arg[2].ptr);
        char * r = strchr (q, 'b');
        encPtr s = {false, 1};
        if (r == NULL)
        {
            int t = strlen (q);
            s = allocByteObj (t + 2);
            r = (char *)vonNeumannSpaceOf (s);
            memcpy (r, q, t);
            *(r + t) = 'b';
            q = r;
        }
        fp[i] = fopen (p, q);
        if (r == NULL)
            isVolatilePut (s, false);
    }
    if (fp[i] == NULL)
        return ((objRef)nilObj);
    else
        return ((objRef)encValueOf (i));
}

/*
Closes the file denoted by the receiver.
Always fails.
Called from File>>close
*/
objRef primFileClose (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        (void)fclose (fp[i]);
    fp[i] = NULL;
    return ((objRef)nilObj);
}

void coldFileIn (encVal tagRef);

/*
Applies the built-in "fileIn" function to the file denoted by the
receiver.
Always fails.
Not called from the image.
N.B.:  The built-in function uses the built-in compiler.  Both should be
used only in connection with building an initial image.
*/
objRef primFileIn (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        coldFileIn (arg[0].val);
    return ((objRef)nilObj);
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
objRef primGetString (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    int j;
    char buffer[4096];
    if (!fp[i])
        return ((objRef)nilObj);
    j = 0;
    buffer[j] = '\0';
    while (1)
    {
        if (fgets (&buffer[j], 512, fp[i]) == NULL)
        {
            if (fp[i] == stdin)
                (void)fputc ('\n', stdout);
            return ((objRef)nilObj); /* end of file */
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
    return ((objRef)newString (buffer));
}

__INLINE__ bool irf (FILE * tag, HostMemoryAddress dat, word len)
{
    return ((fread (dat, len, 1, tag) == 1) ? true : false);
}

encPtr imageRead (FILE * tag)
{
    encVal ver = encValueOf (3);
    encVal val;
    word ord;
    otbEnt * otp;
    ot2Ent * o2p;
    encPtr ptr;
    word len;
    if (irf (tag, &val, sizeof val) != true)
        goto fail;
    if (ptrNe ((objRef)val, (objRef)ver))
        goto fail;
    while (irf (tag, &val, sizeof val) == true)
    {
        ord = intValueOf (val);
        otp = &objTbl[ord];
#if 0
    if(irf(tag, (void*)otp, sizeof(HostMemoryAddress)) != true)
      goto fail;
#endif
        if (irf (tag,
                 ((char *)otp) + sizeof (HostMemoryAddress),
                 sizeof (otbEnt) - sizeof (HostMemoryAddress)) != true)
            goto fail;
        o2p = &ob2Tbl[ord];
        if (irf (tag, o2p, sizeof (ot2Ent)) != true)
            goto fail;
        ptr = encIndexOf (ord);
        if ((len = vonNeumannSpaceLengthOf (ptr)))
        {
            vonNeumannSpaceOfPut (ptr, newStorage (len));
            if (irf (tag, vonNeumannSpaceOf (ptr), len) != true)
                goto fail;
        }
    }
    return (trueObj);
fail:
    return (falseObj);
}

__INLINE__ bool iwf (FILE * tag, HostMemoryAddress dat, word len)
{
    return ((fwrite (dat, len, 1, tag) == 1) ? true : false);
}

encPtr imageWrite (FILE * tag)
{
    encVal val = encValueOf (3);
    word ord;
    encPtr ptr;
    otbEnt * otp;
    ot2Ent * o2p;
    word len;
    if (iwf (tag, &val, sizeof val) != true)
        goto fail;
    for (ord = otbLob; ord <= otbHib; ord++)
    {
        ptr = encIndexOf (ord);
        if (isAvail (ptr))
            continue;
        val = encValueOf (ord);
        if (iwf (tag, &val, sizeof val) != true)
            goto fail;
        otp = &objTbl[ord];
#if 0
    if(iwf(tag, (void*)otp, sizeof(HostMemoryAddress)) != true)
      goto fail;
#endif
        if (iwf (tag,
                 ((char *)otp) + sizeof (HostMemoryAddress),
                 sizeof (otbEnt) - sizeof (HostMemoryAddress)) != true)
            goto fail;
        o2p = &ob2Tbl[ord];
        if (iwf (tag, o2p, sizeof (ot2Ent)) != true)
            goto fail;
        if ((len = vonNeumannSpaceLengthOf (ptr)))
            if (iwf (tag, vonNeumannSpaceOf (ptr), len) != true)
                goto fail;
    }
    return (trueObj);
fail:
    return (falseObj);
}

/*
Writes the currently running set of objects in binary form to the file
denoted by the receiver.
Returns true if successful; false or nil otherwise.
Called from File>>saveImage
*/
objRef primImageWrite (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (fp[i])
        return ((objRef)imageWrite (fp[i]));
    else
        return ((objRef)nilObj);
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver.
Always fails.
Called from File>>printNoReturn:
*/
objRef primPrintWithoutNL (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (!fp[i])
        return ((objRef)nilObj);
    (void)fputs ((char *)vonNeumannSpaceOf (arg[1].ptr), fp[i]);
    (void)fflush (fp[i]);
    return ((objRef)nilObj);
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver and appends a newline.
Always fails.
Called from File>>print:
*/
objRef primPrintWithNL (objRef arg[])
{
    int i = intValueOf (arg[0].val);
    if (!fp[i])
        return ((objRef)nilObj);
    (void)fputs ((char *)vonNeumannSpaceOf (arg[1].ptr), fp[i]);
    (void)fputc ('\n', fp[i]);
    return ((objRef)nilObj);
}

/*
Defines the trace vector slot denoted by the receiver to be the value
denoted by the argument.
Returns the receiver.
Not usually called from the image.
*/
objRef primSetTrace (objRef arg[])
{
    traceVect[intValueOf (arg[0].val)] = intValueOf (arg[1].val);
    return (arg[0]);
}

/*
Prints the von Neumann space of the receiver, followed by a newline, and
causes an abort.
Not usually called from the image.
*/
objRef primError (objRef arg[])
{
    (void)fprintf (
        stderr, "error: '%s'\n", (char *)vonNeumannSpaceOf (arg[0].ptr));
    assert (false);
    return (arg[0]);
}

/*
Causes memory reclamation.
Returns the receiver.
Not usually called from the image.
N.B.:  Do not call this primitive from the image with a receiver of
false.
*/
objRef primReclaim (objRef arg[])
{
    if (ptrEq (arg[0], (objRef)trueObj) || ptrEq (arg[0], (objRef)falseObj))
    {
        reclaim (ptrEq (arg[0], (objRef)trueObj));
        return (arg[0]);
    }
    else
        return ((objRef)nilObj);
}

FILE * logTag = NULL;
encPtr logBuf = {false, 1};
HostMemoryAddress logPtr = 0;
word logSiz = 0;
word logPos = 0;

void logInit ()
{
    logPos = 0;
}

void logByte (byte val)
{
    if (logPos == logSiz)
    {
        encPtr newBuf = allocByteObj (logSiz + 128);
        HostMemoryAddress newPtr = vonNeumannSpaceOf (newBuf);
        (void)memcpy (newPtr, logPtr, logSiz);
        isVolatilePut (logBuf, false);
        logBuf = newBuf;
        logPtr = newPtr;
        logSiz = countOf (logBuf);
    }
    *(((byte *)logPtr) + logPos++) = val;
}

bool logFini ()
{
    if (logTag == NULL)
        return (false);
    if (fwrite (logPtr, logPos, 1, logTag) != 1)
        return (false);
    if (fflush (logTag) == EOF)
        return (false);
    return (true);
}

/*
Writes the von Neumann space of the receiver, except for trailing nulls,
to the transcript in "chunk" form.  A chunk is usually a sequence of
non-'!' bytes followed by a '!' byte followed by a newline.  To
support '!' bytes within a chunk, such bytes are written as pairs of
'!' bytes.
Returns the receiver if successful; nil otherwise.
Called from ByteArray>>logChunk
*/
objRef primLogChunk (objRef arg[])
{
    logInit ();
    {
        encPtr txtBuf = arg[0].ptr;
        HostMemoryAddress txtPtr = vonNeumannSpaceOf (txtBuf);
        word txtSiz = countOf (txtBuf);
        word txtPos = 0;
        while (txtSiz && *(((byte *)txtPtr) + (txtSiz - 1)) == '\0')
            txtSiz--;
        while (txtPos != txtSiz)
        {
            byte val = *(((byte *)txtPtr) + txtPos++);
            logByte (val);
            if (val == '!')
                logByte (val);
        }
    }
    logByte ('!');
    logByte ('\n');
    if (logFini () != true)
        return ((objRef)nilObj);
    return (arg[0]);
}

encPtr bwsBuf = {false, 1};
HostMemoryAddress bwsPtr = 0;
word bwsSiz = 0;
word bwsPos = 0;

void bwsInit (void)
{
    bwsPos = 0;
}

void bwsNextPut (byte val)
{
    if (bwsPos == bwsSiz)
    {
        encPtr newBuf = allocByteObj (bwsSiz + 128);
        HostMemoryAddress newPtr = vonNeumannSpaceOf (newBuf);
        (void)memcpy (newPtr, bwsPtr, bwsSiz);
        isVolatilePut (bwsBuf, false);
        bwsBuf = newBuf;
        bwsPtr = newPtr;
        bwsSiz = countOf (bwsBuf);
    }
    *(((byte *)bwsPtr) + bwsPos++) = val;
}

encPtr bwsFiniGet (void)
{
    encPtr ans = allocByteObj (bwsPos + 1);
    HostMemoryAddress tgt = vonNeumannSpaceOf (ans);
    (void)memcpy (tgt, bwsPtr, bwsPos);
    if (ptrEq ((objRef)stringClass, (objRef)nilObj)) /*fix*/
        stringClass = globalValue ("String");
    classOfPut (ans, stringClass);
    return (ans);
}

bool bwsFiniPut (FILE * tag)
{
    if (fwrite (bwsPtr, bwsPos, 1, tag) != 1)
        return (false);
    if (fflush (tag) == EOF)
        return (false);
    return (true);
}

/*
Reads the next chunk of characters from the file denoted by the
receiver.  A chunk is usually a sequence of non-'!' bytes followed by
a '!' byte followed by a newline.  To support '!' bytes within a
chunk, such bytes are read as pairs of '!' bytes.  Creates a new
String.  The von Neumann space of the new String is the bytes of the
chunk, not including the trailing '!' byte or newline, followed by a
null.
Returns the new String if successful, nil otherwise.
Called from File>>getChunk
*/
objRef primGetChunk (objRef arg[])
{
    int i;
    FILE * tag;
    int val;
    i = intValueOf (arg[0].val);
    if ((tag = fp[i]) == NULL)
        goto fail;
    bwsInit ();
    while ((val = fgetc (tag)) != EOF)
    {
        if (val == '!')
            switch ((val = fgetc (tag)))
            {
            case '\n':
                goto done;
            case '!':
                break;
            default:
                goto fail;
            }
        bwsNextPut (val);
    }
fail:
    return ((objRef)nilObj);
done:
    return ((objRef)bwsFiniGet ());
}

/*
Writes the von Neumann space of the argument, except for trailing nulls,
to the file denoted by the receiver in "chunk" form.  A chunk is usually
a sequence of non-'!' bytes followed by a '!' byte followed by a
newline.  To support '!' bytes within a chunk, such bytes are written
as pairs of '!' bytes.
Returns the receiver if successful; nil otherwise.
Called from File>>putChunk
*/
objRef primPutChunk (objRef arg[])
{
    int i;
    FILE * tag;
    i = intValueOf (arg[0].val);
    if ((tag = fp[i]) == NULL)
        goto fail;
    bwsInit ();
    {
        encPtr txtBuf = arg[1].ptr;
        HostMemoryAddress txtPtr = vonNeumannSpaceOf (txtBuf);
        word txtSiz = countOf (txtBuf);
        word txtPos = 0;
        while (txtSiz && *(((byte *)txtPtr) + (txtSiz - 1)) == '\0')
            txtSiz--;
        while (txtPos != txtSiz)
        {
            byte val = *(((byte *)txtPtr) + txtPos++);
            bwsNextPut (val);
            if (val == '!')
                bwsNextPut (val);
        }
    }
    bwsNextPut ('!');
    bwsNextPut ('\n');
    if (bwsFiniPut (tag) == true)
        goto done;
fail:
    return ((objRef)nilObj);
done:
    return (arg[0]);
}

/* If built as part of L2, then the "special" primitive (#44) maps
 * straight back to the Pascal code. Otherwise, it returns nil to the
 * Smalltalk application.
 */
#ifdef L2_SMALLTALK_EMBEDDED
objRef L2_SMALLTALK_SPECIAL (objRef arguments[]);
#define primSpecial L2_SMALLTALK_SPECIAL
#else
objRef primSpecial (objRef arguments[])
{
    return (objRef)nilObj;
}
#endif

typedef objRef primitiveMethod (objRef arg[]);

#define primVectLob 0
#define primVectHib 255
#define primVectDom ((primVectHib + 1) - primVectLob)

primitiveMethod * primitiveVector[primVectDom] = {
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
    /*038*/ &primFlushCache,
    /*039*/ &primParse,
    /*040*/ &unsupportedPrim,
    /*041*/ &unsupportedPrim,
    /*042*/ &unsupportedPrim,
    /*043*/ &unsupportedPrim,
    /*044*/ &primSpecial,
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
    /*055*/ &primSetSeed,
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
    /*127*/ &primImageWrite,
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
    /*151*/ &primSetTrace,
    /*152*/ &primError,
    /*153*/ &primReclaim,
    /*154*/ &primLogChunk,
    /*155*/ &unsupportedPrim,
    /*156*/ &unsupportedPrim,
    /*157*/ &primGetChunk,
    /*158*/ &primPutChunk,
    /*159*/ &unsupportedPrim};

__INLINE__ objRef primitive (int primitiveNumber, objRef * arguments)
{
    if (primitiveNumber >= primVectLob && primitiveNumber <= primVectHib)
    {
        primitiveMethod * primMethPtr = primitiveVector[primitiveNumber];
        if (primMethPtr)
            return ((*primMethPtr) (arguments));
    }
    return (unsupportedPrim (arguments));
}

/*
The basic scheduling unit is a Process.  We keep a separate copy of its
reference.  This interpreter is explicitly stack-based.  We use a
separate stack for each Process and keep pointers to both its bottom and
top.  Information about particular activations of a Method can be
maintained in separate Context objects.  However, if a separate object
isn't needed, this information is kept within the process stack.  A
returned object replaces the receiver and arguments of the message which
produced it.  This occurs within the process stack at an offset called
the "return point".  We treat arguments and temporaries as if they were
stored in separate spaces.  However, they may actually be kept within
the process stack.  Though the receiver can be thought of as the
"zeroth" argument and accessed from the argument space, we keep separate
copies of its reference and a pointer to its instance variable space.
We also keep separate pointers to the literal and bytecode spaces of a
Method.  The "instruction pointer" is kept as an offset into the
bytecode space.  An explicit counter supports a rudimentary multi-
programming scheme.
*/
typedef struct
{
    encPtr pcso;   /* process object */
                   /*encPtr  pso;      process stack object */
    objRef * psb;  /* process stack base address */
    objRef * pst;  /* process stack top address */
    encPtr cxto;   /* context or process stack object */
    objRef * cxtb; /* context or process stack base address */
    int rtnp;      /* offset at which to store a returned object */
    objRef * argb; /* argument base address */
    objRef * tmpb; /* temporary base address */
    objRef rcvo;   /* receiver object */
    objRef * rcvb; /* receiver base address */
                   /*encPtr  lito;     literal object */
    objRef * litb; /* literal base address */
                   /*encPtr  byto;     bytecode object */
    byte * bytb;   /* bytecode base address - 1 */
    word byteOffset;
    int timeSliceCounter;
} execState;
#define processObject pcso
#define contextObject cxto
#define returnPoint rtnp
#define receiverObject rcvo

__INLINE__ objRef processStackAt (execState * es, int n)
{
    return (*(es->psb + (n - 1)));
}

__INLINE__ objRef stackTop (execState * es)
{
    return (*es->pst);
}

__INLINE__ void stackTopPut (execState * es, objRef x)
{
    *es->pst = x;
}

__INLINE__ void stackTopFree (execState * es)
{
    *es->pst-- = (objRef)nilObj;
}

__INLINE__ int stackInUse (execState * es)
{
    return ((es->pst + 1) - es->psb);
}

__INLINE__ void ipush (execState * es, objRef x)
{
    *++es->pst = x;
}

__INLINE__ objRef ipop (execState * es)
{
    objRef x = *es->pst;
    *es->pst-- = (objRef)nilObj;
    return (x);
}

__INLINE__ objRef argumentAt (execState * es, int n)
{
    return (*(es->argb + n));
}

__INLINE__ objRef temporaryAt (execState * es, int n)
{
    return (*(es->tmpb + n));
}

__INLINE__ void temporaryAtPut (execState * es, int n, objRef x)
{
    *(es->tmpb + n) = x;
}

__INLINE__ objRef receiverAt (execState * es, int n)
{
    return (*(es->rcvb + n));
}

__INLINE__ void receiverAtPut (execState * es, int n, objRef x)
{
    *(es->rcvb + n) = x;
}

__INLINE__ objRef literalAt (execState * es, int n)
{
    return (*(es->litb + n));
}

__INLINE__ byte nextByte (execState * es)
{
    return (*(es->bytb + es->byteOffset++));
}

bool unsupportedByte (execState * es, int low)
{
    sysError ("invalid bytecode", "");
    return (false);
}

/*
Pushes the value of one of the receiver's instance variables onto the
process stack.  The instruction operand denotes which one.
*/
bool bytePushInstance (execState * es, int low)
{
    ipush (es, receiverAt (es, low));
    return (true);
}

/*
Pushes the value of one of the message's argument variables onto the
process stack.  The instruction operand denotes which one.  Note that
the receiver is accessed as the "zeroth" argument.
*/
bool bytePushArgument (execState * es, int low)
{
    ipush (es, argumentAt (es, low));
    return (true);
}

/*
Pushes the value of one of the method's temporary variables onto the
process stack.  The instruction operand denotes which one.
*/
bool bytePushTemporary (execState * es, int low)
{
    ipush (es, temporaryAt (es, low));
    return (true);
}

/*
Pushes one of the method's literal values onto the process stack.  The
instruction operand denotes which one.  See also "bytePushConstant".
*/
bool bytePushLiteral (execState * es, int low)
{
    ipush (es, literalAt (es, low));
    return (true);
}

encPtr method = {true, 0};

encPtr copyFrom (encPtr obj, int start, int size)
{
    encPtr newObj;
    int i;

    newObj = newArray (size);
    for (i = 1; i <= size; i++)
    {
        orefOfPut (newObj, i, orefOf (obj, start));
        start++;
    }
    return newObj;
}

void fetchLinkageState (execState * es)
{
    es->contextObject = processStackAt (es, linkPointer + 1).ptr;
    es->returnPoint = intValueOf (processStackAt (es, linkPointer + 2).val);
    es->byteOffset = intValueOf (processStackAt (es, linkPointer + 4).val);
    if (ptrEq ((objRef)es->contextObject, (objRef)nilObj))
    {
        es->contextObject = processStack;
        es->cxtb = es->psb;
        es->argb = es->cxtb + (es->returnPoint - 1);
        method = processStackAt (es, linkPointer + 3).ptr;
        es->tmpb = es->cxtb + linkPointer + 4;
    }
    else
    { /* read from context object */
        es->cxtb = (objRef *)vonNeumannSpaceOf (es->contextObject);
        method = orefOf (es->contextObject, methodInContext).ptr;
        es->argb = (objRef *)vonNeumannSpaceOf (
            orefOf (es->contextObject, argumentsInContext).ptr);
        es->tmpb = (objRef *)vonNeumannSpaceOf (
            orefOf (es->contextObject, temporariesInContext).ptr);
    }
}

__INLINE__ void fetchReceiverState (execState * es)
{
    es->receiverObject = argumentAt (es, 0);
    if (isIndex (es->receiverObject))
    {
        assert (ptrNe (es->receiverObject, (objRef)pointerList));
        es->rcvb = (objRef *)vonNeumannSpaceOf (es->receiverObject.ptr);
    }
    else
        es->rcvb = (objRef *)0;
}

__INLINE__ void fetchMethodState (execState * es)
{
    es->litb =
        (objRef *)vonNeumannSpaceOf (orefOf (method, literalsInMethod).ptr);
    es->bytb =
        (byte *)vonNeumannSpaceOf (orefOf (method, bytecodesInMethod).ptr) - 1;
}

/*
Pushes one of several "constant" value onto the process stack.  The
instruction operand denotes which one.  Note that a given context object
is not "constant" in that the values of its instance variables may
change.  However, the identity of a given context object is "constant"
in that it will not change.  See also "bytePushLiteral".
*/
bool bytePushConstant (execState * es, int low)
{
    switch (low)
    {
    case 0:
    case 1:
    case 2:
        ipush (es, (objRef)encValueOf (low));
        break;
    case minusOne:
        ipush (es, (objRef)encValueOf (-1));
        break;
    case contextConst:
        /* check to see if we have made a block context yet */
        if (ptrEq ((objRef)es->contextObject, (objRef)processStack))
        {
            /* not yet, do it now - first get real return point */
            es->returnPoint =
                intValueOf (processStackAt (es, linkPointer + 2).val);
            es->contextObject = newContext (
                linkPointer,
                method,
                copyFrom (processStack,
                          es->returnPoint,
                          linkPointer - es->returnPoint),
                copyFrom (
                    processStack, linkPointer + 5, methodTempSize (method)));
            orefOfPut (
                processStack, linkPointer + 1, (objRef)es->contextObject);
            ipush (es, (objRef)es->contextObject);
            /* save byte pointer then restore things properly */
            orefOfPut (processStack,
                       linkPointer + 4,
                       (objRef)encValueOf (es->byteOffset));
            fetchLinkageState (es);
            fetchReceiverState (es);
            fetchMethodState (es);
            break;
        }
        ipush (es, (objRef)es->contextObject);
        break;
    case nilConst:
        ipush (es, (objRef)nilObj);
        break;
    case trueConst:
        ipush (es, (objRef)trueObj);
        break;
    case falseConst:
        ipush (es, (objRef)falseObj);
        break;
    default:
        sysError ("unimplemented constant", "pushConstant");
        return (false);
    }
    return (true);
}

/*
Stores the value on the top of the process stack into of one of the
receiver's instance variables.  The instruction operand denotes which
one.  Note that this doesn't pop the value from the stack.
*/
bool byteAssignInstance (execState * es, int low)
{
    receiverAtPut (es, low, stackTop (es));
    return (true);
}

/*
Stores the value on the top of the process stack into of one of the
method's temporary variables.  The instruction operand denotes which
one.  Note that this doesn't pop the value from the stack.
*/
bool byteAssignTemporary (execState * es, int low)
{
    temporaryAtPut (es, low, stackTop (es));
    return (true);
}

/*
Computes the offset within the process stack at which a returned object
will replace the receiver and arguments of a message.
*/
bool byteMarkArguments (execState * es, int low)
{
    es->returnPoint = (stackInUse (es) - low) + 1;
    es->timeSliceCounter++; /* make sure we do send */
    return (true);
}

__INLINE__ encPtr firstLookupClass (execState * es)
{
    es->argb = es->psb + (es->returnPoint - 1);
    fetchReceiverState (es);
    return (getClass (es->receiverObject));
}

encPtr messageToSend = {true, 0};

int messTest (encPtr obj)
{
    return (ptrEq ((objRef)obj, (objRef)messageToSend));
}

bool findMethod (encPtr * methodClassLocation)
{
    encPtr methodTable, methodClass;

    method = nilObj;
    methodClass = *methodClassLocation;

    for (; ptrNe ((objRef)methodClass, (objRef)nilObj);
         methodClass = orefOf (methodClass, superClassInClass).ptr)
    {
        methodTable = orefOf (methodClass, methodsInClass).ptr;
        if (ptrEq ((objRef)methodTable, (objRef)nilObj))
        { /*fix*/
            methodTable = newDictionary (MethodTableSize);
            orefOfPut (methodClass, methodsInClass, (objRef)methodTable);
        }
        method =
            hashEachElement (methodTable, oteIndexOf (messageToSend), messTest);
        if (ptrNe ((objRef)method, (objRef)nilObj))
            break;
    }

    if (ptrEq ((objRef)method, (objRef)nilObj))
    { /* it wasn't found */
        methodClass = *methodClassLocation;
        return false;
    }
    *methodClassLocation = methodClass;
    return true;
}

#define cacheSize 211

struct
{
    encPtr cacheMessage; /* the message being requested */
    encPtr lookupClass;  /* the class of the receiver */
    encPtr cacheClass;   /* the class of the method */
    encPtr cacheMethod;  /* the method itself */
} methodCache[cacheSize] = {};

void flushCache (encPtr messageToSend, encPtr klass)
{
    int i;
    for (i = 0; i != cacheSize; i++)
        if (ptrEq ((objRef)methodCache[i].cacheMessage, (objRef)messageToSend))
            methodCache[i].cacheMessage = nilObj;
}

bool lookupGivenSelector (execState * es, encPtr methodClass)
{
    int hash;
    int j;
    encPtr argarray;
    objRef returnedObject;
    if (mselTrace)
        fprintf (stderr,
                 "%d: %s\n",
                 mselTrace--,
                 (char *)vonNeumannSpaceOf (messageToSend));
    /* look up method in cache */
    hash = (oteIndexOf (messageToSend) + oteIndexOf (methodClass)) % cacheSize;
    assert (hash >= 0 && hash < cacheSize);
    if (ptrEq ((objRef)methodCache[hash].cacheMessage, (objRef)messageToSend) &&
        ptrEq ((objRef)methodCache[hash].lookupClass, (objRef)methodClass))
    {
        method = methodCache[hash].cacheMethod;
        methodClass = methodCache[hash].cacheClass;
        assert (isAvail (method) == false);
    }
    else
    {
        methodCache[hash].lookupClass = methodClass;
        if (!findMethod (&methodClass))
        {
            /* not found, we invoke a smalltalk method */
            /* to recover */
            j = stackInUse (es) - es->returnPoint;
            argarray = newArray (j + 1);
            for (; j >= 0; j--)
            {
                returnedObject = ipop (es);
                orefOfPut (argarray, j + 1, returnedObject);
            }
            ipush (es, orefOf (argarray, 1)); /* push receiver back */
            ipush (es, (objRef)messageToSend);
            messageToSend = newSymbol ("message:notRecognizedWithArguments:");
            ipush (es, (objRef)argarray);
            /* try again - if fail really give up */
            if (!findMethod (&methodClass))
            {
                sysWarn ("can't find", "error recovery method");
                /* just quit */
                return false;
            }
        }
        methodCache[hash].cacheMessage = messageToSend;
        methodCache[hash].cacheMethod = method;
        methodCache[hash].cacheClass = methodClass;
    }
    return (true);
}

bool watching = 0;

bool lookupWatchSelector (execState * es)
{
    int j;
    encPtr argarray;
    objRef returnedObject;
    encPtr methodClass;
    if (watching && ptrNe (orefOf (method, watchInMethod), (objRef)nilObj))
    {
        /* being watched, we send to method itself */
        j = stackInUse (es) - es->returnPoint;
        argarray = newArray (j + 1);
        for (; j >= 0; j--)
        {
            returnedObject = ipop (es);
            orefOfPut (argarray, j + 1, returnedObject);
        }
        ipush (es, (objRef)method); /* push method */
        ipush (es, (objRef)argarray);
        messageToSend = newSymbol ("watchWith:");
        /* try again - if fail really give up */
        methodClass = classOf (method);
        if (!findMethod (&methodClass))
        {
            sysWarn ("can't find", "watch method");
            /* just quit */
            return false;
        }
    }
    return (true);
}

encPtr growProcessStack (int top, int toadd)
{
    int size, i;
    encPtr newStack;

    if (toadd < 128)
        toadd = 128;
    size = countOf (processStack) + toadd;
    newStack = newArray (size);
    for (i = 1; i <= top; i++)
    {
        orefOfPut (newStack, i, orefOf (processStack, i));
    }
    return newStack;
}

void pushStateAndEnter (execState * es)
{
    int i;
    int j;
    /* save the current byte pointer */
    orefOfPut (
        processStack, linkPointer + 4, (objRef)encValueOf (es->byteOffset));
    /* make sure we have enough room in current process */
    /* stack, if not make stack larger */
    i = 6 + methodTempSize (method) + methodStackSize (method);
    j = stackInUse (es);
    if ((j + i) > countOf (processStack))
    {
        processStack = growProcessStack (j, i);
        es->psb = (objRef *)vonNeumannSpaceOf (processStack);
        es->pst = (es->psb + j);
        orefOfPut (es->processObject, stackInProcess, (objRef)processStack);
    }
    es->byteOffset = 1;
    /* now make linkage area */
    /* position 0 : old linkage pointer */
    ipush (es, (objRef)encValueOf (linkPointer));
    linkPointer = stackInUse (es);
    /* position 1 : context obj (nil means stack) */
    ipush (es, (objRef)nilObj);
    es->contextObject = processStack;
    es->cxtb = es->psb;
    /* position 2 : return point */
    ipush (es, (objRef)encValueOf (es->returnPoint));
    es->argb = es->cxtb + (es->returnPoint - 1);
    /* position 3 : method */
    ipush (es, (objRef)method);
    /* position 4 : bytecode counter */
    ipush (es, (objRef)encValueOf (es->byteOffset));
    /* then make space for temporaries */
    es->tmpb = es->pst + 1;
    es->pst += methodTempSize (method);
    fetchMethodState (es);
#if 0
  /* break if we are too big and probably looping */
  if (countOf(processStack) > 4096)
    es->timeSliceCounter = 0;
#endif
}

__INLINE__ bool lookupAndEnter (execState * es, encPtr methodClass)
{
    if (!lookupGivenSelector (es, methodClass))
        return (false);
    if (!lookupWatchSelector (es))
        return (false);
    pushStateAndEnter (es);
    return (true);
}

/*
Looks for a Method corresponding to the combination of a prospective
receiver's class and a symbol denoting some desired behavior.  The
instruction operand denotes which symbol.  Changes the execution state
of the interpreter such that the next bytecode executed will be that of
the Method located, if possible, in an appropriate context.  See also
"byteSendUnary", "byteSendBinary" and "byteDoSpecial".
*/
bool byteSendMessage (execState * es, int low)
{
    encPtr methodClass;
    messageToSend = literalAt (es, low).ptr;
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Handles certain special cases of messages involving one object.  See
also "byteSendMessage", "byteSendBinary" and "byteDoSpecial".
*/
bool byteSendUnary (execState * es, int low)
{
    encPtr methodClass;
    /* do isNil and notNil as special cases, since */
    /* they are so common */
    if ((!watching) && (low >= 0 && low <= 1))
    {
        switch (low)
        {
        case 0: /* isNil */
            stackTopPut (es,
                         (objRef) (ptrEq (stackTop (es), (objRef)nilObj)
                                       ? trueObj
                                       : falseObj));
            return (true);
        case 1: /* notNil */
            stackTopPut (es,
                         (objRef) (ptrEq (stackTop (es), (objRef)nilObj)
                                       ? falseObj
                                       : trueObj));
            return (true);
        }
    }
    es->returnPoint = stackInUse (es);
    messageToSend = unSyms[low];
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Handles certain special cases of messages involving two objects.  See
also "byteSendMessage", "byteSendUnary" and "byteDoSpecial".
*/
bool byteSendBinary (execState * es, int low)
{
    objRef * primargs;
    objRef returnedObject;
    encPtr methodClass;
    /* optimized as long as arguments are int */
    /* and conversions are not necessary */
    /* and overflow does not occur */
    if ((!watching) && (low >= 0 && low <= 12))
    {
        if (primTrace)
            fprintf (stderr, "%d: <%d>\n", primTrace--, low + 60);
        primargs = es->pst - 1;
        returnedObject = primitive (low + 60, primargs);
        if (ptrNe (returnedObject, (objRef)nilObj))
        {
            /* pop arguments off stack , push on result */
            stackTopFree (es);
            stackTopPut (es, returnedObject);
            return (true);
        }
    }
    es->returnPoint = stackInUse (es) - 1;
    messageToSend = binSyms[low];
    methodClass = firstLookupClass (es);
    return (lookupAndEnter (es, methodClass));
}

/*
Calls a routine to evoke some desired behavior which is not implemented
in the form of a Method.
*/
bool byteDoPrimitive (execState * es, int low)
{
    objRef * primargs;
    int i;
    objRef returnedObject;
    /* low gives number of arguments */
    /* next byte is primitive number */
    primargs = (es->pst - low) + 1;
    /* next byte gives primitive number */
    i = nextByte (es);
    if (primTrace)
        fprintf (stderr, "%d: <%d>\n", primTrace--, i);
    returnedObject = primitive (i, primargs);
    /* pop off arguments */
    while (low-- > 0)
    {
        if (isIndex (stackTop (es)))
            isVolatilePut (stackTop (es).ptr, false);
        stackTopFree (es);
    }
    ipush (es, returnedObject);
    return (true);
}

bool leaveAndAnswer (execState * es, objRef returnedObject)
{
    es->returnPoint = intValueOf (orefOf (processStack, linkPointer + 2).val);
    linkPointer = intValueOf (orefOf (processStack, linkPointer).val);
    while (stackInUse (es) >= es->returnPoint)
    {
        if (isIndex (stackTop (es)))
            isVolatilePut (stackTop (es).ptr, false);
        stackTopFree (es);
    }
    ipush (es, returnedObject);
    /* now go restart old routine */
    if (linkPointer)
    {
        fetchLinkageState (es);
        fetchReceiverState (es);
        fetchMethodState (es);
        return (true);
    }
    else
        return (false); /* all done */
}

/*
Handles operations which aren't handled in other ways.  The instruction
operand denotes which operation.  Returning objects changes the
execution state of the interpreter such that the next bytecode executed
will be that of the Method which is to process the returned object, if
possible, in an appropriate context.  See also "byteSendMessage"
"byteSendUnary" and "byteSendBinary".  Various facilities such as
cascaded messages and optimized control structures involve tinkering
with the top of the process stack and the "instruction counter".
Sending messages to "super" changes the first class to be searched for a
Method from that of the prospective receiver to the superclass of that
in which the executing Method is located, if possible.
*/
bool byteDoSpecial (execState * es, int low)
{
    objRef returnedObject;
    int i;
    encPtr methodClass;
    switch (low)
    {
    case SelfReturn:
        returnedObject = argumentAt (es, 0);
        return (leaveAndAnswer (es, returnedObject));
    case StackReturn:
        returnedObject = ipop (es);
        return (leaveAndAnswer (es, returnedObject));
    case Duplicate:
        /* avoid possible subtle bug */
        returnedObject = stackTop (es);
        ipush (es, returnedObject);
        return (true);
    case PopTop:
        returnedObject = ipop (es);
        if (isIndex (returnedObject))
            isVolatilePut (returnedObject.ptr, false);
        return (true);
    case Branch:
        /* avoid a subtle bug here */
        i = nextByte (es);
        es->byteOffset = i;
        return (true);
    case BranchIfTrue:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)trueObj))
        {
            /* leave nil on stack */
            es->pst++;
            es->byteOffset = i;
        }
        return (true);
    case BranchIfFalse:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)falseObj))
        {
            /* leave nil on stack */
            es->pst++;
            es->byteOffset = i;
        }
        return (true);
    case AndBranch:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)falseObj))
        {
            ipush (es, returnedObject);
            es->byteOffset = i;
        }
        return (true);
    case OrBranch:
        returnedObject = ipop (es);
        i = nextByte (es);
        if (ptrEq (returnedObject, (objRef)trueObj))
        {
            ipush (es, returnedObject);
            es->byteOffset = i;
        }
        return (true);
    case SendToSuper:
        i = nextByte (es);
        messageToSend = literalAt (es, i).ptr;
        (void)firstLookupClass (es); /* fix? */
        methodClass = orefOf (method, methodClassInMethod).ptr;
        /* if there is a superclass, use it
           otherwise for class Object (the only
           class that doesn't have a superclass) use
           the class again */
        returnedObject = orefOf (methodClass, superClassInClass);
        if (ptrNe (returnedObject, (objRef)nilObj))
            methodClass = returnedObject.ptr;
        return (lookupAndEnter (es, methodClass));
    default:
        sysError ("invalid doSpecial", "");
        return (false);
    }
}

typedef bool bytecodeMethod (execState * es, int low);

#define byteVectLob 0
#define byteVectHib 15
#define byteVectDom ((byteVectHib + 1) - byteVectLob)

bytecodeMethod * bytecodeVector[byteVectDom] = {
    /*00*/ &unsupportedByte,
    /*01*/ &bytePushInstance,
    /*02*/ &bytePushArgument,
    /*03*/ &bytePushTemporary,
    /*04*/ &bytePushLiteral,
    /*05*/ &bytePushConstant,
    /*06*/ &byteAssignInstance,
    /*07*/ &byteAssignTemporary,
    /*08*/ &byteMarkArguments,
    /*09*/ &byteSendMessage,
    /*10*/ &byteSendUnary,
    /*11*/ &byteSendBinary,
    /*12*/ &unsupportedByte,
    /*13*/ &byteDoPrimitive,
    /*14*/ &unsupportedByte,
    /*15*/ &byteDoSpecial};

encPtr processStack = {true, 0};

int linkPointer = 0;

void fetchProcessState (execState * es)
{
    int j;
    processStack = orefOf (es->processObject, stackInProcess).ptr;
    es->psb = (objRef *)vonNeumannSpaceOf (processStack);
    j = intValueOf (orefOf (es->processObject, stackTopInProcess).val);
    es->pst = es->psb + (j - 1);
    linkPointer = intValueOf (orefOf (es->processObject, linkPtrInProcess).val);
}

void storeProcessState (execState * es)
{
    orefOfPut (es->processObject, stackInProcess, (objRef)processStack);
    orefOfPut (es->processObject,
               stackTopInProcess,
               (objRef)encValueOf (stackInUse (es)));
    orefOfPut (
        es->processObject, linkPtrInProcess, (objRef)encValueOf (linkPointer));
}

word traceVect[traceSize] = {};

bool execute (encPtr aProcess, int maxsteps)
{
    execState es = {};

    es.processObject = aProcess;
    es.timeSliceCounter = maxsteps;
    counterAddress = &es.timeSliceCounter;

    fetchProcessState (&es);
    fetchLinkageState (&es);
    fetchReceiverState (&es);
    fetchMethodState (&es);

    while (--es.timeSliceCounter > 0)
    {
        int low;
        int high;
        low = (high = nextByte (&es)) & 0x0F;
        high >>= 4;
        if (high == 0)
        {
            high = low;
            low = nextByte (&es);
        }
        if (execTrace)
            fprintf (stderr, "%d: %d %d\n", execTrace--, high, low);
        if (high >= byteVectLob && high <= byteVectHib)
        {
            bytecodeMethod * byteMethPtr = bytecodeVector[high];
            if (byteMethPtr)
            {
                if (!(*byteMethPtr) (&es, low))
                    return (false);
                continue;
            }
        }
        if (!unsupportedByte (&es, low))
            return (false);
    }

    orefOfPut (
        processStack, linkPointer + 4, (objRef)encValueOf (es.byteOffset));
    storeProcessState (&es);

    return (true);
}

void makeInitialImage (void)
{
    encPtr hashTable;
    encPtr symbolObj;
    encPtr symbolClass; /*shadows global for a reason*/
    encPtr metaclassClass;
    encPtr linkClass;

    nilObj = allocOrefObj (0);
    assert (oteIndexOf (nilObj) == 1);

    trueObj = allocOrefObj (0);
    assert (oteIndexOf (trueObj) == 2);
    falseObj = allocOrefObj (0);
    assert (oteIndexOf (falseObj) == 3);

    /* create the symbol table */

    hashTable = allocOrefObj (3 * 53);
    assert (oteIndexOf (hashTable) == 4);
    symbols = allocOrefObj (1);
    assert (oteIndexOf (symbols) == 5);
    orefOfPut (symbols, 1, (objRef)hashTable);

    /* create #Symbol, Symbol[Meta] and Metaclass[Meta] */

    symbolObj = newSymbol ("Symbol");
#if 0
  assert(ptrEq(classOf(symbolObj),nilObj));
  assert(ptrEq(globalValue("Symbol"),nilObj));
#endif
    symbolClass = newClass ("Symbol");
#if 0
  assert(ptrNe(classOf(symbolClass),nilObj));
  assert(ptrEq(classOf(classOf(symbolClass)),nilObj));
  assert(ptrEq(globalValue("Symbol"),symbolClass));
#endif
    classOfPut (symbolObj, symbolClass);
    classOfPut (newSymbol ("SymbolMeta"), symbolClass);
    metaclassClass = newClass ("Metaclass");
#if 0
  assert(ptrNe(classOf(metaclassClass),nilObj));
  assert(ptrEq(classOf(classOf(metaclassClass)),nilObj));
  assert(ptrEq(globalValue("Metaclass"),metaclassClass));
#endif
    classOfPut (classOf (symbolClass), metaclassClass);
    classOfPut (classOf (metaclassClass), metaclassClass);

    /* patch the class fields of nil, true and false */
    /* set their global values */

    classOfPut (nilObj, newClass ("UndefinedObject"));
    nameTableInsert (symbols, strHash ("nil"), newSymbol ("nil"), nilObj);
    classOfPut (trueObj, newClass ("True"));
    nameTableInsert (symbols, strHash ("true"), newSymbol ("true"), trueObj);
    classOfPut (falseObj, newClass ("False"));
    nameTableInsert (symbols, strHash ("false"), newSymbol ("false"), falseObj);

    /* patch the class fields of the symbol table links */
    /* make the symbol table refer to itself */ /*fix?*/

    linkClass = newClass ("Link");
    {
        word ord = 0;
        word hib = countOf (hashTable);
        for (; ord != hib; ord += 3)
        {
            encPtr link = orefOf (hashTable, ord + 3).ptr;
            while (ptrNe ((objRef)link, (objRef)nilObj))
            {
                if (ptrEq ((objRef)classOf (link), (objRef)nilObj))
                    classOfPut (link, linkClass);
                else
                    assert (ptrEq ((objRef)classOf (link), (objRef)linkClass));
                link = orefOf (link, 3).ptr;
            }
        }
    }
    classOfPut (hashTable, newClass ("Array"));
    classOfPut (symbols, newClass ("SymbolTable"));
    nameTableInsert (
        symbols, strHash ("symbols"), newSymbol ("symbols"), symbols);

    /* graft a skeleton metaclass tree to a skeleton class tree */
    {
        encPtr objectInst = newClass ("Object");
        encPtr classInst = newClass ("Class");
        orefOfPut (classOf (objectInst), superClassInClass, (objRef)classInst);
    }

    /* create other skeleton classes */

    /*(void) newClass("Array");*/
    (void)newClass ("Block");
    (void)newClass ("ByteArray");
    (void)newClass ("Char");
    (void)newClass ("Context");
    (void)newClass ("Dictionary");
    (void)newClass ("Float");
    /*(void) newClass("Link");*/
    /*(void) newClass("Metaclass");*/
    (void)newClass ("Method");
    (void)newClass ("String");
    /*(void) newClass("Symbol");*/
}

void goDoIt (char * text)
{
    encPtr method;
    encPtr process;
    encPtr stack;

    method = newMethod ();
    setInstanceVariables (nilObj);
    (void)parse (method, text, false);

    process = allocOrefObj (processSize);
    stack = allocOrefObj (50);

    /* make a process */
    orefOfPut (process, stackInProcess, (objRef)stack);
    orefOfPut (process, stackTopInProcess, (objRef)encValueOf (10));
    orefOfPut (process, linkPtrInProcess, (objRef)encValueOf (2));

    /* put argument on stack */
    orefOfPut (stack, 1, (objRef)nilObj); /* argument */
    /* now make a linkage area in stack */
    orefOfPut (stack, 2, (objRef)encValueOf (0)); /* previous link */
    orefOfPut (stack, 3, (objRef)nilObj); /* context object (nil => stack) */
    orefOfPut (stack, 4, (objRef)encValueOf (1)); /* return point */
    orefOfPut (stack, 5, (objRef)method);         /* method */
    orefOfPut (stack, 6, (objRef)encValueOf (1)); /* byte offset */

    /* now go execute it */
    printf ("BEGIN ACTUAL EXECUTION\n");
    while (execute (process, 1 << 14))
        fprintf (stderr, ".");

    /* get rid of unwanted process */
    isVolatilePut (process, false);
}

int main_1 (int argc, char * argv[])
{
    char methbuf[4096];
    int i;

    sysWarn ("\nPublic Domain Smalltalk", "");

    coldObjectTable ();

    makeInitialImage ();

    initCommonSymbols ();

    for (i = 1; i < argc; i++)
    {
        fputs ("Processing ", stderr);
        fputs (argv[i], stderr);
        fputs ("...\n", stderr);
        GenerationContext ctx;
        // fprintf(stderr, "%s:\n", argv[i]);
        //(void) sprintf(methbuf,
        //	   "goDoIt <120 1 '%s' 'r'>. <123 1>. <121 1>",
        //	   argv[i]);
        ProgramNode * aNode = MVST_Parser::parseFile (argv[i]);
        if (aNode)
            aNode->generateInContext (&ctx);
        printf ("\n\nDONE MINE\n\n");
        /*methbuf[0] = 0;
        strcat (methbuf, "goDoIt <120 1 '");
        strcat (methbuf, argv[i]);
        strcat (methbuf, "' 'r'>. <123 1>. <121 1>");
        goDoIt (methbuf);*/
    }

    /* when we are all done looking at the arguments, do initialization */
    fprintf (stderr, "initialization\n");
#if 0
  execTrace = 16;
  primTrace = 16;
  mselTrace = 16;
#endif
    goDoIt ("goDoIt nil initialize\n");
    fprintf (stderr, "finished\n");

    return 0;
}

int main_2 (int argc, char * argv[])
{
    FILE * fp;
    encPtr firstProcess;
    char *p, buffer[4096];

    sysWarn ("\nPublic Domain Smalltalk", "");

    warmObjectTableOne ();

    strcpy (buffer, "systemImage");
    p = buffer;
    if (argc != 1)
        p = argv[1];

    fp = fopen (p, "rb");
    if (fp == NULL)
    {
        sysError ("cannot open image", p);
        return (1);
    }

    if (ptrNe ((objRef)imageRead (fp), (objRef)trueObj))
    {
        sysError ("cannot read image", p);
        return (1);
    }

    (void)fclose (fp);

    warmObjectTableTwo ();

    initCommonSymbols ();

    firstProcess = globalValue ("systemProcess");
    if (ptrEq ((objRef)firstProcess, (objRef)nilObj))
    {
        sysError ("no initial process", "in image");
        return (1);
    }

    /* execute the main system process loop repeatedly */

    while (execute (firstProcess, 1 << 14))
        ;

    return 0;
}

void compilError (const char * selector, const char * str1, const char * str2)
{
    (void)fprintf (
        stderr, "compiler error: Method %s : %s %s\n", selector, str1, str2);
    parseOk = false;
}

void compilWarn (const char * selector, const char * str1, const char * str2)
{
    (void)fprintf (
        stderr, "compiler warning: Method %s : %s %s\n", selector, str1, str2);
}

void sysError (const char * s1, const char * s2)
{
    (void)fprintf (stderr, "%s\n%s\n", s1, s2);
    (void)abort ();
}

void sysWarn (const char * s1, const char * s2)
{
    (void)fprintf (stderr, "%s\n%s\n", s1, s2);
}

#ifdef L2_SMALLTALK_EMBEDDED
int L2_SMALLTALK_MAIN (int argc, char * argv[])
#else
int main (int argc, char * argv[])
#endif
{
    // printf("sizeof(otbEnt) = %d, sizeof(ot2Ent) = %d\n", sizeof(otbEnt),
    // sizeof(ot2Ent));
    int ans = 1;
    logTag = fopen ("transcript", "ab");
    if (argc > 1 && streq (argv[1], "-c"))
    {
        argv[1] = argv[0];
        argc--;
        argv++;
        ans = main_1 (argc, argv);
    }
    if (argc > 1 && streq (argv[1], "-w"))
    {
        argv[1] = argv[0];
        argc--;
        argv++;
        ans = main_2 (argc, argv);
    }
#if 0
  fprintf(stderr,"%s?\n",
    (char*)vonNeumannSpaceOf(orefOf(encIndexOf(100),nameInClass).ptr));
#endif
    if (ans == 0)
    {
        FILE * tag;
        tag = fopen ("snapshot", "wb");
        if (tag != NULL)
        {
            reclaim (false);
            if (ptrNe ((objRef)imageWrite (tag), (objRef)trueObj))
                ans = 2;
            (void)fclose (tag);
        }
        else
            ans = 2;
    }
    if (logTag != NULL)
        (void)fclose (logTag);
    return (ans);
}
