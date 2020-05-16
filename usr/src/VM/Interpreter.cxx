#include "Interpreter.hxx"
#include "Bytecode.hxx"
#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "Lowlevel/MVPrinting.hxx"
#include "Oops/Oops.hxx"

static const char * bytecodeNames[] = {
    "None",

    "PushSelf",
    "PushNil",
    "PushTrue",
    "PushFalse",
    "PushContext",

    "PushInstanceVar",
    "PushArgument",
    "PushLocal",
    "PushLiteral",
    "PushParentHeapVar",
    "PushMyHeapVar",
    "PushBlockCopy",

    "StoreInstanceVar",
    "StoreLocal",
    "StoreParentHeapVar",
    "StoreMyHeapVar",
    "StoreGlobal",

    "Send",
    "SendSuper",

    "Duplicate",
    "Pop",

    "Return",
    "BlockReturn",

    "MinTwoArgs",

    "MoveParentHeapVarToMyHeapVars",
    "MoveArgumentToMyHeapVars",
    "MoveLocalToMyHeapVars",

    "Primitive",

    "Max",
};

void printBytecode (uint8_t code, uint8_t arg, uint8_t arg2)
{
    if (code >= kMinTwoArgs && code <= kMax)
    {
        printf ("%-16s(%d,%d)\n", bytecodeNames[code], arg, arg2);
    }
    else
    {
        printf ("%-16s(%d)\n", bytecodeNames[code], arg);
    }
}

void printAllBytecode (ByteArrayOop arr, int indent)
{
    for (int i = 1; i < arr.size ();)
    {
        uint8_t code = arr.basicAt (i++);
        uint8_t arg = arr.basicAt (i++);

        if (code >= kMinTwoArgs && code <= kMax)
        {
            printBytecode (code, arg, arr.basicAt (i++));
        }
        else
        {
            printBytecode (code, arg, 0);
        }
    }
}

static MethodOop lookupMethodInClass (ClassOop cls, SymbolOop selector,
                                      bool super)
{
    ClassOop lookupClass = super ? cls.superClass () : cls;
    MethodOop meth;

    printf (" -> Begin search in class %s\n",
            lookupClass.name ().asString ().c_str ());

    if (!lookupClass.methods ().isNil ())
        meth = lookupClass.methods ().symbolLookup (selector).asMethodOop ();
    else
        printf (" -> Class %s has blank methods table\n ",
                lookupClass.name ().asString ().c_str ());

    if (meth.isNil ())
    {
        ClassOop super;
        if (((super = lookupClass.superClass ()) == memMgr.objNil ()) ||
            (super == lookupClass))
        {
            printf (" -> Failed to find method %s in class %s\n",
                    selector.asString ().c_str (),
                    cls.name ().asString ().c_str ());
            abort ();
        }
        else
        {
            printf (" -> DID NOT find method %s in class %s, searching super\n",
                    selector.asString ().c_str (),
                    cls.name ().asString ().c_str ());
            return lookupMethodInClass (super, selector, false);
        }
    }

    return meth;
}

void opSend (ExecState & es, int numArgs, bool toSuper)
{
    SymbolOop selector = es.proc.context ().pop ().asSymbolOop ();
    ArrayOop arguments = ArrayOop::newWithSize (numArgs);
    Oop receiver;
    MethodOop candidate;

    for (int i = numArgs; i > 0; i--)
    {
        // printf ("-> Argument %d:\n", i);
        arguments.basicatPut (i, es.proc.context ().pop ());
        // arguments.basicAt (i).print (10);
    }

    receiver = es.proc.context ().pop ();

    printf ("- Send %s to (isa %d)%s\n",
            selector.asString ().c_str (),
            receiver.isa ().index (),
            receiver.isa ().name ().asString ().c_str ());
    // receiver.print (2);

    candidate = lookupMethodInClass (receiver.isa (), selector, toSuper);

    if (!candidate.isNil ())
    {
        ContextOop ctx = ContextOop::newWithMethod (receiver, candidate);
        ctx.setArguments (arguments);
        ctx.setPreviousContext (es.proc.context ());
        es.proc.setContext (ctx);
        printf ("------------%s>>%s----------------\n",
                candidate.selector ().asString ().c_str (),
                receiver.isa ().name ().asString ().c_str ());
    }
    else
        es.proc.context ().push (memMgr.objNil ());
}
extern "C" int usleep (int);

void Processor::interpret (ProcessOop proc)
{
    ExecState es;
    uint8_t opcode;
    bool shouldRun;

    es.proc = proc;

    proc.context ().methodOrBlock ().print (5);

    while (opcode = proc.context ().fetchByte ())
    {
        usleep (5000);
        uint8_t arg = proc.context ().fetchByte ();
        uint8_t arg2;

        if (opcode > kMinTwoArgs && opcode < kMax)
            arg2 = es.proc.context ().fetchByte ();

        printf ("[%d@%p]",
                proc.context ().programCounter ().intValue (),
                proc.context ().index ());
        printBytecode (opcode, arg, arg2);

        switch (opcode)
        {

        case kPushSelf:
            es.proc.context ().push (es.proc.context ().receiver ());
            break;

        case kPushNil:
            es.proc.context ().push (memMgr.objNil ());
            break;

        case kPushTrue:
            es.proc.context ().push (memMgr.objTrue ());
            break;

        case kPushFalse:
            es.proc.context ().push (memMgr.objFalse ());
            break;

        case kPushContext:
            es.proc.context ().push (es.proc.context ());
            break;

        case kPushInstanceVar:
        {
            Oop var = es.proc.context ().receiver ().asOopOop ().basicAt (arg);
            es.proc.context ().push (var);
            break;
        }

        case kPushArgument:
        {
            Oop var = es.proc.context ().arguments ().basicAt (arg);
            es.proc.context ().push (var);
            break;
        }

        case kPushLocal:
        {
            Oop var = es.proc.context ().temporaries ().basicAt (arg);
            es.proc.context ().push (var);
            break;
        }

        case kPushLiteral:
        {
            Oop var = es.proc.context ()
                          .methodOrBlock ()
                          .asMethodOop ()
                          .literals ()
                          .basicAt (arg);
            es.proc.context ().push (var);
            break;
        }

        case kPushParentHeapVar:
        {
            Oop val = es.proc.context ().parentHeapVars ().basicAt (arg);
            es.proc.context ().push (val);
            break;
        }

        case kPushMyHeapVar:
        {
            Oop var = es.proc.context ().heapVars ().basicAt (arg);
            es.proc.context ().push (var);
            break;
        }

        case kPushBlockCopy:
        {
            BlockOop var = es.proc.context ()
                               .methodOrBlock ()
                               .asMethodOop ()
                               .literals ()
                               .basicAt (arg)
                               .asBlockOop ();
            var.setParentHeapVars (es.proc.context ().heapVars ());
            var.setReceiver (es.proc.context ().receiver ());
            es.proc.context ().push (var);
            break;
        }

        case kStoreInstanceVar:
        {
            Oop val = es.proc.context ().top ();
            es.proc.context ().receiver ().asOopOop ().basicatPut (arg, val);
            break;
        }

        case kStoreLocal:
        {
            Oop val = es.proc.context ().top ();
            es.proc.context ().temporaries ().basicatPut (arg, val);
            break;
        }

        case kStoreParentHeapVar:
        {
            Oop val = es.proc.context ().top ();
            es.proc.context ().parentHeapVars ().basicatPut (arg, val);
            break;
        }

        case kStoreMyHeapVar:
        {
            Oop val = es.proc.context ().top ();
            es.proc.context ().heapVars ().basicatPut (arg, val);
            break;
        }

        case kStoreGlobal:
        {
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
            break;
        }

        case kSend:
            /*printf ("\n\n\n");
            for (int i = es.proc.context ().stackPointer ().intValue (); i > 0;
                 i--)
            {
                printf ("STACK AT %d\n", i);
                es.proc.context ().stack ().basicAt (i).print (15);
            }
            printf ("\n\n\n");*/
            opSend (es, arg, false);
            break;

        case kSendSuper:
            opSend (es, arg, true);
            break;

        case kDuplicate:
            es.proc.context ().dup ();
            break;

        case kPop:
            es.proc.context ().pop ();
            break;

        case kReturn:
        {
            Oop result = es.proc.context ().pop ();
            ContextOop prevContext = es.proc.context ().previousContext ();
            if (!prevContext.isNil ())
            {
                // printf ("RESULT!\n");
                // result.print (5);
                prevContext.push (result);
                es.proc.setContext (prevContext);
                printf ("Returning:\n");
                result.print (5);
                printf ("----------------RETURNED-------------------\n");
            }
            else
            {
                printf ("FINAL RESULT:\n\n");
                result.print (0);
                for (int i = es.proc.context ().stackPointer ().intValue ();
                     i > 0;
                     i--)
                {
                    printf ("STACK AT %d\n", i);
                    es.proc.context ().stack ().basicAt (i).print (15);
                }
                printf ("\n\n\n");
            }

            break;
        }

        case kMoveParentHeapVarToMyHeapVars:
        {
            Oop val = es.proc.context ().parentHeapVars ().basicAt (arg);
            es.proc.context ().heapVars ().basicatPut (arg2, val);
            break;
        }

        case kMoveArgumentToMyHeapVars:
        {
            Oop val = es.proc.context ().arguments ().basicAt (arg);
            es.proc.context ().heapVars ().basicatPut (arg2, val);
            break;
        }

        case kMoveLocalToMyHeapVars:
        {
            Oop val = es.proc.context ().temporaries ().basicAt (arg);
            es.proc.context ().heapVars ().basicatPut (arg2, val);
            break;
        }

        case kPrimitive:
        {
            ArrayOop arguments = ArrayOop::newWithSize (arg2);

            for (int i = arg2; i > 0; i--)
            {
                arguments.basicatPut (i, es.proc.context ().pop ());
            }

            es.proc.context ().push (primVec[arg](es, arguments));
            break;
        }

        default:
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
        }
    }
}

void Processor::coldBootMainProcessor ()
{
    Processor mainProc;
    MethodOop start = MVST_Parser::parseText ("^ 1 + 3")
                          ->synthInClassScope (nullptr)
                          ->generate ();
    ProcessOop firstProcess = ProcessOop::allocate ();
    firstProcess.setContext (
        ContextOop::newWithMethod (memMgr.objNil (), start));

    mainProc.interpret (firstProcess);
}