#include "tcc/libtcc.h"

#include "Bytecode.hxx"
#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "Interpreter.hxx"
#include "Lowlevel/MVPrinting.hxx"
#include "Oops/Oops.hxx"

//#define dprintf(...) printf (__VA_ARGS__)
#define dprintf printf

#define __DEBUG 1
//#define dprintf(...)

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
    "PushGlobal",
    "PushBlockCopy",

    "StoreInstanceVar",
    "StoreLocal",
    "StoreParentHeapVar",
    "StoreMyHeapVar",
    "StoreGlobal",

    "IfTrueIfFalse",

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

ProcessorOop ProcessorOopDesc::mainProcessor;
SymbolOop ProcessorOopDesc::binSels[28];

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

inline MethodOop ProcessorOopDesc::lookupMethodInClass (
    ProcessOop proc, Oop receiver, ClassOop cls, SymbolOop selector, bool super)
{
    ClassOop lookupClass = super ? cls->superClass () : cls;
    MethodOop meth;

    dprintf (" -> Begin search in class %s\n", lookupClass->name ()->asCStr ());

    if (!lookupClass->methods ().isNil ())
    {
        meth = lookupClass->methods ()->symbolLookup (selector)->asMethodOop ();
    }
    else
        dprintf (" -> Class %s has blank methods table\n ",
                 lookupClass->name ()->asCStr ());

    if (meth.isNil ())
    {
        ClassOop super;
        if (((super = lookupClass->superClass ()) == Oop::nilObj ()) ||
            (super == lookupClass))
        {
            ContextOop ctx = proc->context ();
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
            while ((ctx = ctx->previousContext ()) != Oop::nilObj ())
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
            return lookupMethodInClass (proc, receiver, super, selector, false);
        }
    }

    return meth;
}

inline MethodOop ProcessorOopDesc::lookupMethodInCache (int hash,
                                                        SymbolOop selector,
                                                        ClassOop receiverClass)
{
    Oop * candArea = &cache ()->vonNeumannSpace ()[hash * 3];

    if (candArea[0] == selector && candArea[1] == receiverClass)
    {
        /*printf ("%d Cache HIT %s>>%s !!\n",
                hash,
                receiverClass->name ()->asCStr (),
                selector->asCStr ());*/
        return candArea[2]->asMethodOop ();
    }
    else
    {
        /*printf ("%d Cache miss %s>>%s !!\n",
                hash,
                receiverClass->name ()->asCStr (),
                selector->asCStr ());*/
        return Oop::nilObj ()->asMethodOop ();
    }
}

inline void ProcessorOopDesc::setMethodInCache (int hash, SymbolOop selector,
                                                ClassOop receiverClass,
                                                MethodOop meth)
{
    Oop * candArea = &cache ()->vonNeumannSpace ()[hash * 3];

    candArea[0] = selector;
    candArea[1] = receiverClass;
    candArea[2] = meth;
}

inline void ProcessorOopDesc::doSend (ProcessOop proc, Oop receiver,
                                      SymbolOop selector, ArrayOop arguments,
                                      bool toSuper)
{
    MethodOop candidate;
    ClassOop receiverClass =
        toSuper ? receiver.isa ()->superClass () : receiver.isa ();
    int hash =
        (((uintptr_t)selector.addr ()) + (uintptr_t) (receiverClass.addr ())) %
        cacheSize;

    dprintf ("- Send %s to (isa %d)%s\n",
             selector->asCStr (),
             receiver.isa ().index (),
             receiver.isa ()->name ()->asCStr ());
    candidate = lookupMethodInCache (hash, selector, receiverClass);

    if (candidate.isNil ())
    {
        candidate = lookupMethodInClass (
            proc, receiver, receiver.isa (), selector, toSuper);
        setMethodInCache (hash, selector, receiverClass, candidate);
    }

    if (!candidate.isNil ())
    {
        ContextOop ctx = ContextOopDesc::newWithMethod (receiver, candidate);
        ctx->setArguments (arguments);
        ctx->setPreviousContext (proc->context ());
        proc->setContext (ctx);
        dprintf ("------------%s>>%s----------------\n",

                 receiver.isa ()->name ()->asCStr (),
                 candidate->selector ()->asCStr ());
    }
    else
        proc->context ()->push (Oop::nilObj ());
}

inline void ProcessorOopDesc::opSend (ProcessOop proc, int numArgs,
                                      bool toSuper)
{
    SymbolOop selector = proc->context ()->pop ()->asSymbolOop ();
    ArrayOop arguments = ArrayOopDesc::newWithSize (numArgs);
    Oop receiver;
    MethodOop candidate;

    for (int i = numArgs; i > 0; i--)
    {
        // printf ("-> Argument %d:\n", i);
        arguments->basicatPut (i, proc->context ()->pop ());
        // arguments->basicAt (i)->print (10);
    }

    receiver = proc->context ()->pop ();

    doSend (proc, receiver, selector, arguments, toSuper);
}

void ProcessorOopDesc::interpret ()
{
    uint8_t opcode;
    bool shouldRun;
    ProcessOop proc = process ();

    setCache (ArrayOopDesc::newWithSize (cacheSize * 4));
    proc->context ()->init ();
    printf ("Beginning execution. Method disassembly:\n");
    proc->context ()->methodOrBlock ()->print (5);

    while (1)
    {
        opcode = proc->context ()->fetchByte ();

        if (!opcode)
            abort ();

        uint8_t arg = proc->context ()->fetchByte ();
        uint8_t arg2;

        if (opcode > kMinTwoArgs && opcode < kMax)
            arg2 = proc->context ()->fetchByte ();

        dprintf ("[%d@%p]",
                 proc->context ()->programCounter ().intValue (),
                 proc->context ().index ());
        printBytecode (opcode, arg, arg2);

        switch (opcode)
        {

        case kPushSelf:
            proc->context ()->push (proc->context ()->receiver ());
            break;

        case kPushNil:
            proc->context ()->push (Oop::nilObj ());
            break;

        case kPushTrue:
            proc->context ()->push (MemoryManager::objTrue);
            break;

        case kPushFalse:
            proc->context ()->push (MemoryManager::objFalse);
            break;

        case kPushSmalltalk:
            proc->context ()->push (MemoryManager::objGlobals);
            break;

        case kPushContext:
            proc->context ()->push (proc->context ());
            break;

        case kPushInstanceVar:
        {
            Oop var = proc->context ()->receiver ()->asOopOop ()->basicAt (arg);
            proc->context ()->push (var);
            break;
        }

        case kPushArgument:
        {
            Oop var = proc->context ()->arguments ()->basicAt (arg);
            proc->context ()->push (var);
            break;
        }

        case kPushLocal:
        {
            Oop var = proc->context ()->temporaries ()->basicAt (arg);
            proc->context ()->push (var);
            break;
        }

        case kPushLiteral:
        {
            Oop var = proc->context ()
                          ->methodOrBlock ()
                          ->asMethodOop ()
                          ->literals ()
                          ->basicAt (arg);
            proc->context ()->push (var);
            break;
        }

        case kPushParentHeapVar:
        {
            Oop val = proc->context ()->parentHeapVars ()->basicAt (arg);
            proc->context ()->push (val);
            break;
        }

        case kPushMyHeapVar:
        {
            Oop var = proc->context ()->heapVars ()->basicAt (arg);
            proc->context ()->push (var);
            break;
        }

        case kPushGlobal:
        {
            SymbolOop sym = proc->context ()
                                ->methodOrBlock ()
                                ->asMethodOop ()
                                ->literals ()
                                ->basicAt (arg)
                                ->asSymbolOop ();
            Oop var = proc->context ()
                          ->receiver ()
                          .isa ()
                          ->dictionary ()
                          ->symbolLookupNamespaced (sym->asString ());
            assert (!var.isNil ());
            var->print (5);
            proc->context ()->push (var);
            break;
        }

        case kPushBlockCopy:
        {
            BlockOop var = memMgr
                               .copyObj (proc->context ()
                                             ->methodOrBlock ()
                                             ->asMethodOop ()
                                             ->literals ()
                                             ->basicAt (arg))
                               ->asBlockOop ();
            var->setParentHeapVars (proc->context ()->heapVars ());
            var->setReceiver (proc->context ()->receiver ());
            var->setHomeMethodContext (proc->context ());
            proc->context ()->push (var);
            break;
        }

        case kStoreInstanceVar:
        {
            Oop val = proc->context ()->top ();
            proc->context ()->receiver ()->asOopOop ()->basicatPut (arg, val);
            break;
        }

        case kStoreLocal:
        {
            Oop val = proc->context ()->top ();
            proc->context ()->temporaries ()->basicatPut (arg, val);
            break;
        }

        case kStoreParentHeapVar:
        {
            Oop val = proc->context ()->top ();
            proc->context ()->parentHeapVars ()->basicatPut (arg, val);
            break;
        }

        case kStoreMyHeapVar:
        {
            Oop val = proc->context ()->top ();
            proc->context ()->heapVars ()->basicatPut (arg, val);
            break;
        }

        case kStoreGlobal:
        {
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
            break;
        }

        case kIfTrueIfFalse:
        {
            if (arg == 2)
            {
                Oop falseBlock = proc->context ()->pop ();
                Oop trueBlock = proc->context ()->pop ();
                Oop boole = proc->context ()->pop ();

                if (boole == memMgr.objTrue)
                {
                    doSend (proc,
                            trueBlock,
                            symValue,
                            Oop::nilObj ()->asArrayOop (),
                            false);
                }
                else if (boole == memMgr.objFalse)
                {
                    doSend (proc,
                            falseBlock,
                            symValue,
                            Oop::nilObj ()->asArrayOop (),
                            false);
                }
                else
                {
                    ArrayOop args = ArrayOopDesc::newWithSize (2);
                    args->basicatPut (1, trueBlock);
                    args->basicatPut (2, falseBlock);
                    doSend (proc, boole, symIfTrueIfFalse, args, false);
                }
            }
            break;
        }

        case kSend:
            /*printf ("\n\n\n");
            for (int i = proc->context()->stackPointer ().intValue (); i > 0;
                 i--)
            {
                printf ("STACK AT %d\n", i);
                proc->context()->stack ()->basicAt (i)->print (15);
            }
            printf ("\n\n\n");*/
            opSend (proc, arg, false);
            break;

        case kSendSuper:
            opSend (proc, arg, true);
            break;

        case kBinOp:
        {
            /* FIXME: Stupid */
            if (arg >= 0 && arg <= 12)
            {
                Oop returned;
                ArrayOop args = ArrayOopDesc::newWithSize (2);

                args->basicatPut (2, proc->context ()->pop ());
                args->basicatPut (1, proc->context ()->pop ());

                returned = primVec[60 + arg](proc, args);
                if (!returned.isNil ())
                {
                    proc->context ()->push (returned);
                    break;
                }
                else
                {
                    ArrayOop sendArgs = ArrayOopDesc::newWithSize (1);
                    Oop receiver = args->basicAt (1);
                    printf ("=====> For selector %s>>%s, argument %s, prim %d "
                            "returned NIL; sending real message.\n",
                            receiver.isa ()->name ()->asCStr (),
                            binSels[arg]->asCStr (),

                            args->basicAt (2).isa ()->name ()->asCStr (),
                            arg);

                    sendArgs->basicatPut (1, args->basicAt (2));
                    doSend (proc, receiver, binSels[arg], sendArgs, false);
                }

                break;
            }
            else
            {
                ArrayOop sendArgs = ArrayOopDesc::newWithSize (1);
                Oop receiver;

                sendArgs->basicatPut (1, proc->context ()->pop ());
                receiver = proc->context ()->pop ();
                doSend (proc, receiver, binSels[arg], sendArgs, false);
                break;
            }
        }

        case kDuplicate:
            proc->context ()->dup ();
            break;

        case kPop:
            proc->context ()->pop ();
            break;

        case kReturn:
        {
            Oop result = proc->context ()->pop ();
            ContextOop prevContext = proc->context ()->previousContext ();
            if (!prevContext.isNil ())
            {
                // printf ("RESULT!\n");
                // result->print (5);
                prevContext->push (result);
                proc->setContext (prevContext);
                dprintf ("Returning:\n");
                if (__DEBUG)
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
                printf ("FINAL RESULT:\n");
                result->print (0);
                for (int i = proc->context ()->stackPointer ().intValue ();
                     i > 0;
                     i--)
                {
                    dprintf ("STACK AT %d\n", i);
                    // proc->context()->stack ()->basicAt (i)->print (15);
                }
                printf ("VALUTRON PROCESS ID 1 EXITED.");
                goto end;
            }

            break;
        }

        case kBlockReturn:
        {
            Oop result = proc->context ()->pop ();
            ContextOop prevContext = proc->context ();
            ContextOop homeContext = proc->context ()
                                         ->methodOrBlock ()
                                         ->asBlockOop ()
                                         ->homeMethodContext ();
            int i = 1;
            while ((prevContext = prevContext->previousContext ()) !=
                       homeContext &&
                   !prevContext.isNil ())
                i++;
            dprintf ("Home context: %p\n, %p\n", homeContext, prevContext);

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
                proc->setContext (prevContext);
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
            Oop val = proc->context ()->parentHeapVars ()->basicAt (arg);
            proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveArgumentToMyHeapVars:
        {
            Oop val = proc->context ()->arguments ()->basicAt (arg);
            proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveLocalToMyHeapVars:
        {
            Oop val = proc->context ()->temporaries ()->basicAt (arg);
            proc->context ()->heapVars ()->basicatPut (arg2, val);
            break;
        }

        case kMoveMyHeapVarToParentHeapVars:
        {
            Oop val = proc->context ()->heapVars ()->basicAt (arg);
            proc->context ()->parentHeapVars ()->basicatPut (arg2, val);
            break;
        }

        case kPrimitive:
        {
            ArrayOop arguments = ArrayOopDesc::newWithSize (arg2);

            for (int i = arg2; i > 0; i--)
            {
                arguments->basicatPut (i, proc->context ()->pop ());
            }

            proc->context ()->push (primVec[arg](proc, arguments));
            break;
        }

        default:
            printf ("UNIMPLEMENTED %s\n", bytecodeNames[opcode]);
        }
    }
end:
    return;
}

Oop ProcessorOopDesc::runUntilCompletion (std::string code)
{
}

ProcessorOop ProcessorOopDesc::allocate ()
{
    ProcessorOop psr = memMgr.allocateOopObj (clsNstLength)->asProcessorOop ();
    psr.setIsa (memMgr.clsProcessor);
    return psr;
}

extern "C" void tcc_print_stats (TCCState * s, unsigned total_time);

void ProcessorOopDesc::coldBootMainProcessor ()
{
    MethodOop start = MVST_Parser::parseText ("^ 20 fib")
                          ->synthInClassScope (nullptr)
                          ->generate ();
    ProcessOop firstProcess = ProcessOopDesc::allocate ();
    firstProcess->setContext (
        ContextOopDesc::newWithMethod (Oop::nilObj (), start));

    mainProcessor = allocate ();
    mainProcessor->setProcess (firstProcess);
    mainProcessor->interpret ();
}

int ProcessorOopDesc::optimisedBinopSym (SymbolOop name)
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