#include "Interpreter.hxx"
#include "Bytecode.hxx"
#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "Lowlevel/MVPrinting.hxx"
#include "Oops/Oops.hxx"

//#define dprintf(...) printf (__VA_ARGS__)
#define dprintf printf

static const char * bytecodeNames[] = {
    "None",

    "PushSelf",
    "PushNil",
    "PushTrue",
    "PushFalse",
    "PushContext",
    "PushSmalltalk",

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
    "BinOp",

    "Duplicate",
    "Pop",

    "Return",
    "BlockReturn",

    "MinTwoArgs",

    "MoveParentHeapVarToMyHeapVars",
    "MoveArgumentToMyHeapVars",
    "MoveLocalToMyHeapVars",
    "MoveMyHeapVarToParentHeapVars",

    "Primitive",

    "Max",
};

const char * binStr[] = {"+",
                         "-",
                         "<",
                         ">",
                         "<=",
                         ">=",
                         "=",
                         "~=",
                         "*",
                         "quo:",
                         "rem:",
                         "bitAnd:",
                         "bitXor:",
                         "==",
                         ",",
                         "at:",
                         "basicAt:",
                         "do:",
                         "coerce:",
                         "error:",
                         "includesKey:",
                         "isMemberOf:",
                         "new:",
                         "to:",
                         "value:",
                         "whileTrue:",
                         "addFirst:",
                         "addLast:",
                         0};

SymbolOop Processor::binSels[28];

void printBytecode (uint8_t code, uint8_t arg, uint8_t arg2)
{
    if (code >= kMinTwoArgs && code <= kMax)
    {
        dprintf ("%-16s(%d,%d)\n", bytecodeNames[code], arg, arg2);
    }
    else
    {
        dprintf ("%-16s(%d)\n", bytecodeNames[code], arg);
    }
}

void printAllBytecode (ByteArrayOop arr, int indent)
{
    for (int i = 1; i < arr->size ();)
    {
        uint8_t code = arr->basicAt (i++);
        uint8_t arg = arr->basicAt (i++);

        if (code >= kMinTwoArgs && code <= kMax)
        {
            printBytecode (code, arg, arr->basicAt (i++));
        }
        else
        {
            printBytecode (code, arg, 0);
        }
    }
}

static MethodOop lookupMethodInClass (ExecState & es, ClassOop cls,
                                      SymbolOop selector, bool super)
{
    ClassOop lookupClass = super ? cls->superClass () : cls;
    MethodOop meth;

    dprintf (" -> Begin search in class %s\n", lookupClass->name ()->asCStr ());

    if (!lookupClass->methods ().isNil ())
        meth = lookupClass->methods ()->symbolLookup (selector)->asMethodOop ();
    else
        dprintf (" -> Class %s has blank methods table\n ",
                 lookupClass->name ()->asCStr ());

    if (meth.isNil ())
    {
        ClassOop super;
        if (((super = lookupClass->superClass ()) == Oop::nil ()) ||
            (super == lookupClass))
        {
            ContextOop ctx = es.proc->context ();
            dprintf (" -> Failed to find method %s in class %s\n",
                     selector->asCStr (),
                     cls->name ()->asCStr ());
            dprintf ("          --> %s>>%s\n",
                     ctx->receiver ().isa ()->name ()->asCStr (),
                     ctx->isBlockContext () ? "<block>"
                                            : ctx->methodOrBlock ()
                                                  ->asMethodOop ()
                                                  ->selector ()
                                                  ->asCStr ());
            while ((ctx = ctx->previousContext ()) != Oop::nil ())
                dprintf ("          --> %s>>%s\n",
                         ctx->receiver ().isa ()->name ()->asCStr (),
                         ctx->isBlockContext () ? "<block>"
                                                : ctx->methodOrBlock ()
                                                      ->asMethodOop ()
                                                      ->selector ()
                                                      ->asCStr ());
            abort ();
        }
        else
        {
            dprintf (
                " -> DID NOT find method %s in class %s, searching super\n",
                selector->asCStr (),
                cls->name ()->asCStr ());
            return lookupMethodInClass (es, super, selector, false);
        }
    }

    return meth;
}

static void doSend (ExecState & es, Oop receiver, SymbolOop selector,
                    ArrayOop arguments, bool toSuper)
{
    MethodOop candidate;

    dprintf ("- Send %s to (isa %d)%s\n",
             selector->asCStr (),
             receiver.isa ().index (),
             receiver.isa ()->name ()->asCStr ());

    // receiver->print (2);

    candidate = lookupMethodInClass (es, receiver.isa (), selector, toSuper);

    if (!candidate.isNil ())
    {
        ContextOop ctx = ContextOopDesc::newWithMethod (receiver, candidate);
        ctx->setArguments (arguments);
        ctx->setPreviousContext (es.proc->context ());
        es.proc->setContext (ctx);
        dprintf ("------------%s>>%s----------------\n",

                 receiver.isa ()->name ()->asCStr (),
                 candidate->selector ()->asCStr ());
    }
    else
        es.proc->context ()->push (Oop::nil ());
}

void opSend (ExecState & es, int numArgs, bool toSuper)
{
    SymbolOop selector = es.proc->context ()->pop ()->asSymbolOop ();
    ArrayOop arguments = ArrayOopDesc::newWithSize (numArgs);
    Oop receiver;
    MethodOop candidate;

    for (int i = numArgs; i > 0; i--)
    {
        // printf ("-> Argument %d:\n", i);
        arguments->basicatPut (i, es.proc->context ()->pop ());
        // arguments->basicAt (i)->print (10);
    }

    receiver = es.proc->context ()->pop ();

    doSend (es, receiver, selector, arguments, toSuper);
}
extern "C" int usleep (int);

/*static ContextOop firstMethodContext (ExecState & es)
{

}*/

void Processor::interpret (ProcessOop proc)
{
    ExecState es;
    uint8_t opcode;
    bool shouldRun;

    es.proc = proc;
    SmiOop * test = (SmiOop *)calloc (1, sizeof (SmiOop));
    printf ("\n\n\n\n\nsizeof: %d, %d, %d\n\n\n\n",
            sizeof (TagPtr),
            test->intValue (),
            test->isInteger ());

    proc->context ()->init ();
    proc->context ()->programCounter ()->print (5);
    proc->context ()->methodOrBlock ()->print (5);

    while (1)
    {
        opcode = proc->context ()->fetchByte ();
        if (!opcode)
            abort ();
        // usleep (5000);
        uint8_t arg = proc->context ()->fetchByte ();
        uint8_t arg2;

        if (opcode > kMinTwoArgs && opcode < kMax)
            arg2 = es.proc->context ()->fetchByte ();

        dprintf ("[%d@%p]",
                 proc->context ()->programCounter ().intValue (),
                 proc->context ().index ());
        printBytecode (opcode, arg, arg2);

        switch (opcode)
        {

        case kPushSelf:
            es.proc->context ()->push (es.proc->context ()->receiver ());
            break;

        case kPushNil:
            es.proc->context ()->push (Oop::nil ());
            break;

        case kPushTrue:
            es.proc->context ()->push (MemoryManager::objTrue);
            break;

        case kPushFalse:
            es.proc->context ()->push (MemoryManager::objFalse);
            break;

        case kPushSmalltalk:
            es.proc->context ()->push (MemoryManager::objGlobals);
            break;

        case kPushContext:
            es.proc->context ()->push (es.proc->context ());
            break;

        case kPushInstanceVar:
        {
            Oop var =
                es.proc->context ()->receiver ()->asOopOop ()->basicAt (arg);
            es.proc->context ()->push (var);
            break;
        }

        case kPushArgument:
        {
            Oop var = es.proc->context ()->arguments ()->basicAt (arg);
            es.proc->context ()->push (var);
            break;
        }

        case kPushLocal:
        {
            Oop var = es.proc->context ()->temporaries ()->basicAt (arg);
            es.proc->context ()->push (var);
            break;
        }

        case kPushLiteral:
        {
            Oop var = es.proc->context ()
                          ->methodOrBlock ()
                          ->asMethodOop ()
                          ->literals ()
                          ->basicAt (arg);
            es.proc->context ()->push (var);
            break;
        }

        case kPushParentHeapVar:
        {
            Oop val = es.proc->context ()->parentHeapVars ()->basicAt (arg);
            es.proc->context ()->push (val);
            break;
        }

        case kPushMyHeapVar:
        {
            Oop var = es.proc->context ()->heapVars ()->basicAt (arg);
            es.proc->context ()->push (var);
            break;
        }

        case kPushBlockCopy:
        {
            BlockOop var = memMgr
                               .copyObj (es.proc->context ()
                                             ->methodOrBlock ()
                                             ->asMethodOop ()
                                             ->literals ()
                                             ->basicAt (arg))
                               ->asBlockOop ();
            /*ContextOop home = es.proc->context ()->isBlockContext ()
                                  ? firstMethodContext (es)
                                  : es.proc->context ();*/
            var->setParentHeapVars (es.proc->context ()->heapVars ());
            var->setReceiver (es.proc->context ()->receiver ());
            printf ("=> Set home method continuation to %p\n",
                    es.proc->context ().index ());
            var->setHomeMethodContext (es.proc->context ());
            es.proc->context ()->push (var);
            break;
        }

        case kStoreInstanceVar:
        {
            Oop val = es.proc->context ()->top ();
            es.proc->context ()->receiver ()->asOopOop ()->basicatPut (arg,
                                                                       val);
            break;
        }

        case kStoreLocal:
        {
            Oop val = es.proc->context ()->top ();
            es.proc->context ()->temporaries ()->basicatPut (arg, val);
            break;
        }

        case kStoreParentHeapVar:
        {
            Oop val = es.proc->context ()->top ();
            es.proc->context ()->parentHeapVars ()->basicatPut (arg, val);
            break;
        }

        case kStoreMyHeapVar:
        {
            Oop val = es.proc->context ()->top ();
            es.proc->context ()->heapVars ()->basicatPut (arg, val);
            break;
        }

        case kStoreGlobal:
        {
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
            break;
        }

        case kSend:
            /*printf ("\n\n\n");
            for (int i = es.proc->context()->stackPointer ().intValue (); i > 0;
                 i--)
            {
                printf ("STACK AT %d\n", i);
                es.proc->context()->stack ()->basicAt (i)->print (15);
            }
            printf ("\n\n\n");*/
            opSend (es, arg, false);
            break;

        case kSendSuper:
            opSend (es, arg, true);
            break;

        case kBinOp:
        {
            /* FIXME: Stupid */
            if (arg >= 0 && arg <= 12)
            {
                Oop returned;
                ArrayOop args = ArrayOopDesc::newWithSize (2);

                args->basicatPut (2, es.proc->context ()->pop ());
                args->basicatPut (1, es.proc->context ()->pop ());

                dprintf ("Prim %s\n", binSels[arg]->asCStr ());

                returned = primVec[60 + arg](es, args);
                if (!returned.isNil ())
                {
                    es.proc->context ()->push (returned);
                    break;
                }
                else
                {
                    ArrayOop sendArgs = ArrayOopDesc::newWithSize (1);
                    Oop receiver = args->basicAt (1);
                    printf ("=====> For selector %s>>%s, argument %s, prim %d "
                            "returned NIL.\n",
                            receiver.isa ()->name ()->asCStr (),
                            binSels[arg]->asCStr (),

                            args->basicAt (2).isa ()->name ()->asCStr (),
                            arg);

                    sendArgs->basicatPut (1, args->basicAt (2));
                    doSend (es, receiver, binSels[arg], sendArgs, false);
                }

                break;
            }
            else
            {
                ArrayOop sendArgs = ArrayOopDesc::newWithSize (1);
                Oop receiver;

                sendArgs->basicatPut (1, es.proc->context ()->pop ());
                receiver = es.proc->context ()->pop ();
                doSend (es, receiver, binSels[arg], sendArgs, false);
                break;
            }
        }

        case kDuplicate:
            es.proc->context ()->dup ();
            break;

        case kPop:
            es.proc->context ()->pop ();
            break;

        case kReturn:
        {
            Oop result = es.proc->context ()->pop ();
            ContextOop prevContext = es.proc->context ()->previousContext ();
            if (!prevContext.isNil ())
            {
                // printf ("RESULT!\n");
                // result->print (5);
                prevContext->push (result);
                es.proc->setContext (prevContext);
                printf ("Returning:\n");
                result->print (5);
                dprintf ("----------------RETURNED-------------------\n");
                dprintf ("-----------INTO %s>>%s-------------\n",
                         prevContext->receiver ().isa ()->name ()->asCStr (),
                         prevContext->isBlockContext ()
                             ? "<block>"
                             : prevContext->methodOrBlock ()
                                   ->asMethodOop ()
                                   ->selector ()
                                   ->asString ()
                                   .c_str ());
            }
            else
            {
                printf ("FINAL RESULT:\n\n");
                result->print (0);
                for (int i = es.proc->context ()->stackPointer ().intValue ();
                     i > 0;
                     i--)
                {
                    dprintf ("STACK AT %d\n", i);
                    // es.proc->context()->stack ()->basicAt (i)->print (15);
                }
                dprintf ("\n\n\n");
                goto end;
            }

            break;
        }

        case kBlockReturn:
        {
            Oop result = es.proc->context ()->pop ();
            ContextOop prevContext = es.proc->context ();
            ContextOop homeContext = es.proc->context ()
                                         ->methodOrBlock ()
                                         ->asBlockOop ()
                                         ->homeMethodContext ();
            int i = 1;
            while ((prevContext = prevContext->previousContext ()) !=
                       homeContext &&
                   !prevContext.isNil ())
                i++;
            printf ("Home context: %p\n, %p\n", homeContext, prevContext);

            /* FIXME: Is this legit? It seems hacky to me. Better that we
             * somehow make all blocks defined in a given method (even if
             * nested) share the correct homecontext somehow. */
            while (prevContext->isBlockContext ())
            {
                homeContext = prevContext->methodOrBlock ()
                                  ->asBlockOop ()
                                  ->homeMethodContext ();
                while ((prevContext = prevContext->previousContext ()) !=
                           homeContext &&
                       !prevContext.isNil ())
                    i++;
            }

            /* Finally, escape the containing method. */
            prevContext = prevContext->previousContext ();

            if (prevContext.isNil ())
            {
                dprintf ("---------NON-LOCAL RETURNED (%d FRAMES - "
                         "TERMINATED - HOME CONTEXT %p)---------\n",
                         i,
                         homeContext.index ());
                printf ("FINAL RESULT:\n\n");
                result->print (2);
            }
            else
            {
                prevContext->push (result);
                es.proc->setContext (prevContext);
                printf ("Returning:\n");
                result->print (5);
                dprintf ("---------NON-LOCAL RETURNED (%d FRAMES)---------\n",
                         i);
                dprintf ("-----------INTO %s>>%s (%p:%p)-------------\n",
                         prevContext->receiver ().isa ()->name ()->asCStr (),
                         prevContext->isBlockContext ()
                             ? "<block>"
                             : prevContext->methodOrBlock ()
                                   ->asMethodOop ()
                                   ->selector ()
                                   ->asString ()
                                   .c_str (),
                         prevContext.index (),
                         homeContext.index ());
            }
            break;
        }

        case kMoveParentHeapVarToMyHeapVars:
        {
            Oop val = es.proc->context ()->parentHeapVars ()->basicAt (arg);
            es.proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveArgumentToMyHeapVars:
        {
            Oop val = es.proc->context ()->arguments ()->basicAt (arg);
            es.proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveLocalToMyHeapVars:
        {
            Oop val = es.proc->context ()->temporaries ()->basicAt (arg);
            es.proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveMyHeapVarToParentHeapVars:
        {
            Oop val = es.proc->context ()->heapVars ()->basicAt (arg);
            es.proc->context ()->parentHeapVars ()->basicatPut (arg2, val);
            break;
        }

        case kPrimitive:
        {
            ArrayOop arguments = ArrayOopDesc::newWithSize (arg2);

            for (int i = arg2; i > 0; i--)
            {
                arguments->basicatPut (i, es.proc->context ()->pop ());
            }

            es.proc->context ()->push (primVec[arg](es, arguments));
            break;
        }

        default:
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
        }
    }
end:
    return;
}

void Processor::coldBootMainProcessor ()
{
    Processor mainProc;
    MethodOop start = MVST_Parser::parseText ("^ 153 radix: 16")
                          ->synthInClassScope (nullptr)
                          ->generate ();
    ProcessOop firstProcess = ProcessOopDesc::allocate ();
    firstProcess->setContext (
        ContextOopDesc::newWithMethod (Oop::nil (), start));

    mainProc.interpret (firstProcess);
}

int Processor::optimisedBinopSym (SymbolOop name)
{
    if (binSels[0].isNil ())
    {
        for (int i = 0; i < 28; i++)
            binSels[i] = SymbolOopDesc::fromString (binStr[i]);
    }

    for (int i = 0; i < 28; i++)
        if (name->strEquals (binStr[i]))
            return i;
    return -1;
}