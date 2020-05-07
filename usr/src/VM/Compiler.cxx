#include <cstdio>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "Bytecode.hxx"
#include "VM.hxx"

#define streq(a, b) (strcmp (a, b) == 0)

typedef enum tokensyms
{
    nothing,
    /* identifier */
    nameconst,
    /* keyword */
    namecolon,
    intconst,
    floatconst,
    charconst,
    symconst,
    arraybegin,
    strconst,
    binary,
    closing,
    inputend
} tokentype;

tokentype token = nothing;
char tokenString[4096] = {}; /* text of current token */
int tokenInteger = 0;        /* or character */
double tokenFloat = 0.0;

char * cp = 0;
char cc = 0;

int pushindex = 0;
char pushBuffer[16] = {};

long longresult = 0; /*fix*/

void pushBack (char c)
{
    pushBuffer[pushindex++] = c;
}

char nextChar (void)
{
    if (pushindex > 0)
        cc = pushBuffer[--pushindex];
    else if (*cp)
        cc = *cp++;
    else
        cc = '\0';
    return (cc);
}

char peek (void)
{
    pushBack (nextChar ());
    return (cc);
}

bool isClosing (char c)
{
    switch (c)
    {
    case '.':
    case ']':
    case ')':
    case ';':
    case '\"':
    case '\'':
        return (true);
    }
    return (false);
}

bool isSymbolChar (char c)
{
    if (isdigit (c) || isalpha (c))
        return (true);
    if (isspace (c) || isClosing (c))
        return (false);
    return (true);
}

bool singleBinary (char c)
{
    switch (c)
    {
    case '[':
    case '(':
    case ')':
    case ']':
        return (true);
    }
    return (false);
}

bool binarySecond (char c)
{
    if (isalpha (c) || isdigit (c) || isspace (c) || isClosing (c) ||
        singleBinary (c))
        return (false);
    return (true);
}

tokentype nextToken (void)
{
    char * tp;
    bool sign;

    /* skip over blanks and comments */
    while (nextChar () && (isspace (cc) || (cc == '"')))
        if (cc == '"')
        {
            /* read comment */
            while (nextChar () && (cc != '"'))
                ;
            if (!cc)
                break; /* break if we run into eof */
        }
    tp = tokenString;
    *tp++ = cc;

    if (!cc) /* end of input */
        token = inputend;

    else if (isalpha (cc))
    { /* identifier */
        while (nextChar () && isalnum (cc))
            *tp++ = cc;
        if (cc == ':')
        {
            *tp++ = cc;
            token = namecolon;
        }
        else
        {
            pushBack (cc);
            token = nameconst;
        }
    }
    else if (isdigit (cc))
    { /* number */
        longresult = cc - '0';
        while (nextChar () && isdigit (cc))
        {
            *tp++ = cc;
            longresult = (longresult * 10) + (cc - '0');
        }
        if (canEmbed (longresult))
        {
            tokenInteger = longresult;
            token = intconst;
        }
        else
        {
            token = floatconst;
            tokenFloat = (double)longresult;
        }
        if (cc == '.')
        { /* possible float */
            if (nextChar () && isdigit (cc))
            {
                *tp++ = '.';
                do
                    *tp++ = cc;
                while (nextChar () && isdigit (cc));
                if (cc)
                    pushBack (cc);
                token = floatconst;
                *tp = '\0';
                tokenFloat = atof (tokenString);
            }
            else
            {
                /* nope, just an ordinary period */
                if (cc)
                    pushBack (cc);
                pushBack ('.');
            }
        }
        else
            pushBack (cc);

        if (nextChar () && cc == 'e')
        { /* possible float */
            if (nextChar () && cc == '-')
            {
                sign = true;
                (void)nextChar ();
            }
            else
                sign = false;
            if (cc && isdigit (cc))
            { /* yep, its a float */
                *tp++ = 'e';
                if (sign)
                    *tp++ = '-';
                while (cc && isdigit (cc))
                {
                    *tp++ = cc;
                    (void)nextChar ();
                }
                if (cc)
                    pushBack (cc);
                *tp = '\0';
                token = floatconst;
                tokenFloat = atof (tokenString);
            }
            else
            { /* nope, wrong again */
                if (cc)
                    pushBack (cc);
                if (sign)
                    pushBack ('-');
                pushBack ('e');
            }
        }
        else if (cc)
            pushBack (cc);
    }
    else if (cc == '$')
    { /* character constant */
        tokenInteger = (int)nextChar ();
        token = charconst;
    }
    else if (cc == '#')
    {         /* symbol */
        tp--; /* erase pound sign */
        if (nextChar () == '(')
            token = arraybegin;
        else
        {
            pushBack (cc);
            while (nextChar () && isSymbolChar (cc))
                *tp++ = cc;
            pushBack (cc);
            token = symconst;
        }
    }
    else if (cc == '\'')
    {         /* string constant */
        tp--; /* erase pound sign */
    strloop:
        while (nextChar () && (cc != '\''))
            *tp++ = cc;
        /* check for nested quote marks */
        if (cc && nextChar () && (cc == '\''))
        {
            *tp++ = cc;
            goto strloop;
        }
        pushBack (cc);
        token = strconst;
    }
    else if (isClosing (cc)) /* closing expressions */
        token = closing;

    else if (singleBinary (cc))
    { /* single binary expressions */
        token = binary;
    }
    else
    { /* anything else is binary */
        if (nextChar () && binarySecond (cc))
            *tp++ = cc;
        else
            pushBack (cc);
        token = binary;
    }

    *tp = '\0';
    return (token);
}

void lexinit (char * str)
{
    pushindex = 0;
    cp = str;
    /* get first token */
    (void)nextToken ();
}

bool parseOk = false;

int codeTop = 0;
byte codeArray[codeLimit] = {};

int literalTop = 0;
objRef literalArray[literalLimit] = {};

int temporaryTop = 0;
char * temporaryName[temporaryLimit] = {};

int argumentTop = 0;
char * argumentName[argumentLimit] = {};

int instanceTop = 0;
char * instanceName[instanceLimit] = {};

int maxTemporary = 0;
char selector[4096] = {};

enum blockstatus
{
    NotInBlock,
    InBlock,
    OptimizedBlock
} blockstat = NotInBlock;

void setInstanceVariables (encPtr aClass)
{
    int i, limit;
    encPtr vars;

    if (ptrEq ((objRef)aClass, (objRef)nilObj))
        instanceTop = 0;
    else
    {
        setInstanceVariables (orefOf (aClass, superClassInClass).ptr);
        vars = orefOf (aClass, variablesInClass).ptr;
        if (ptrNe ((objRef)vars, (objRef)nilObj))
        {
            limit = countOf (vars);
            for (i = 1; i <= limit; i++)
                instanceName[++instanceTop] =
                    (char *)vonNeumannSpaceOf (orefOf (vars, i).ptr);
        }
    }
}

void genMessage (bool toSuper, int argumentCount, encPtr messagesym);
void expression (void);
void parsePrimitive (void);
void block (void);
void body (void);
void assignment (char * name);

void genCode (int value)
{
    if (codeTop >= codeLimit)
        compilError (selector, "too many bytecode instructions in method", "");
    else
        codeArray[codeTop++] = value;
}

void genInstruction (int high, int low)
{
    printf ("GENINSTRUCTION: %d %d\n", high, low);
    if (low >= 16)
    {
        genInstruction (Extended, high);
        genCode (low);
    }
    else
        genCode (high * 16 + low);
}

int genLiteral (objRef aLiteral)
{
    if (literalTop >= literalLimit)
        compilError (selector, "too many literals in method", "");
    else
    {
        literalArray[++literalTop] = aLiteral;
    }
    return (literalTop - 1);
}

void genInteger (int val)
{
    printf ("GEN INTEGER\n");
    if (val == -1)
        genInstruction (PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction (PushConstant, val);
    else
        genInstruction (PushLiteral, genLiteral ((objRef)encValueOf (val)));
}

const char * glbsyms[] = {"currentInterpreter", "nil", "true", "false", 0};

bool nameTerm (char * name)
{
    int i;
    bool done = false;
    bool isSuper = false;

    /* it might be self or super */
    if (streq (name, "self") || streq (name, "super"))
    {
        genInstruction (PushArgument, 0);
        done = true;
        if (streq (name, "super"))
            isSuper = true;
    }
    /* or it might be a temporary (reverse this to get most recent first) */
    if (!done)
        for (i = temporaryTop; (!done) && (i >= 1); i--)
            if (streq (name, temporaryName[i]))
            {
                genInstruction (PushTemporary, i - 1);
                done = true;
            }
    /* or it might be an argument */
    if (!done)
        for (i = 1; (!done) && (i <= argumentTop); i++)
            if (streq (name, argumentName[i]))
            {
                genInstruction (PushArgument, i);
                done = true;
            }
    /* or it might be an instance variable */
    if (!done)
        for (i = 1; (!done) && (i <= instanceTop); i++)
        {
            if (streq (name, instanceName[i]))
            {
                genInstruction (PushInstance, i - 1);
                done = true;
            }
        }
    /* or it might be a global constant */
    if (!done)
        for (i = 0; (!done) && glbsyms[i]; i++)
            if (streq (name, glbsyms[i]))
            {
                printf ("generateglobal constant because %s\n", name);
                genInstruction (PushConstant, i + 4);
                done = true;
            }
    /* not anything else, it must be a global */
    /* must look it up at run time */
    if (!done)
    {
        genInstruction (PushLiteral, genLiteral ((objRef)newSymbol (name)));
        genMessage (false, 0, newSymbol ("value"));
    }
    return (isSuper);
}

int parseArray (void)
{
    int i, size, base;
    encPtr newLit;
    objRef obj;

    base = literalTop;
    (void)nextToken ();
    while (parseOk && (token != closing))
    {
        switch (token)
        {
        case arraybegin:
            (void)parseArray ();
            break;

        case intconst:
            (void)genLiteral ((objRef)encValueOf (tokenInteger));
            (void)nextToken ();
            break;

        case floatconst:
            (void)genLiteral ((objRef)newFloat (tokenFloat));
            (void)nextToken ();
            break;

        case nameconst:
        case namecolon:
        case symconst:
            (void)genLiteral ((objRef)newSymbol (tokenString));
            (void)nextToken ();
            break;

        case binary:
            if (streq (tokenString, "("))
            {
                (void)parseArray ();
                break;
            }
            if (streq (tokenString, "-") && isdigit (peek ()))
            {
                (void)nextToken ();
                if (token == intconst)
                    (void)genLiteral ((objRef)encValueOf (-tokenInteger));
                else if (token == floatconst)
                {
                    (void)genLiteral ((objRef)newFloat (-tokenFloat));
                }
                else
                    compilError (
                        selector, "negation not followed", "by number");
                (void)nextToken ();
                break;
            }
            (void)genLiteral ((objRef)newSymbol (tokenString));
            (void)nextToken ();
            break;

        case charconst:
            (void)genLiteral ((objRef)newChar (tokenInteger));
            (void)nextToken ();
            break;

        case strconst:
            (void)genLiteral ((objRef)newString (tokenString));
            (void)nextToken ();
            break;

        default:
            compilError (
                selector, "illegal text in literal array", tokenString);
            (void)nextToken ();
            break;
        }
    }

    if (parseOk)
    { /* FIXME: dangling else */
        if (!streq (tokenString, ")"))
            compilError (selector,
                         "array not terminated by right parenthesis",
                         tokenString);
        else
            (void)nextToken ();
    }

    size = literalTop - base;
    newLit = newArray (size);
    for (i = size; i >= 1; i--)
    {
        obj = literalArray[literalTop];
        orefOfPut (newLit, i, obj);
        literalArray[literalTop] = (objRef)nilObj;
        literalTop = literalTop - 1;
    }
    return (genLiteral ((objRef)newLit));
}

bool term (void)
{
    bool superTerm = false; /* true if term is pseudo var super */

    if (token == nameconst)
    {
        superTerm = nameTerm (tokenString);
        (void)nextToken ();
    }
    else if (token == intconst)
    {
        genInteger (tokenInteger);
        (void)nextToken ();
    }
    else if (token == floatconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newFloat (tokenFloat)));
        (void)nextToken ();
    }
    else if ((token == binary) && streq (tokenString, "-"))
    {
        (void)nextToken ();
        if (token == intconst)
            genInteger (-tokenInteger);
        else if (token == floatconst)
        {
            genInstruction (PushLiteral,
                            genLiteral ((objRef)newFloat (-tokenFloat)));
        }
        else
            compilError (selector, "negation not followed", "by number");
        (void)nextToken ();
    }
    else if (token == charconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newChar (tokenInteger)));
        (void)nextToken ();
    }
    else if (token == symconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newSymbol (tokenString)));
        (void)nextToken ();
    }
    else if (token == strconst)
    {
        genInstruction (PushLiteral,
                        genLiteral ((objRef)newString (tokenString)));
        (void)nextToken ();
    }
    else if (token == arraybegin)
    {
        genInstruction (PushLiteral, parseArray ());
    }
    else if ((token == binary) && streq (tokenString, "("))
    {
        (void)nextToken ();
        expression ();
        if (parseOk)
        { /* FIXME: dangling else */
            if ((token != closing) || !streq (tokenString, ")"))
                compilError (selector, "Missing Right Parenthesis", "");
            else
                (void)nextToken ();
        }
    }
    else if ((token == binary) && streq (tokenString, "<"))
        parsePrimitive ();
    else if ((token == binary) && streq (tokenString, "["))
        block ();
    else
        compilError (selector, "invalid expression start", tokenString);

    return (superTerm);
}

void parsePrimitive (void)
{
    int primitiveNumber, argumentCount;

    if (nextToken () != intconst)
        compilError (selector, "primitive number missing", "");
    primitiveNumber = tokenInteger;
    (void)nextToken ();
    argumentCount = 0;
    while (parseOk && !((token == binary) && streq (tokenString, ">")))
    {
        (void)term ();
        argumentCount++;
    }
    genInstruction (DoPrimitive, argumentCount);
    genCode (primitiveNumber);
    (void)nextToken ();
}

void genMessage (bool toSuper, int argumentCount, encPtr messagesym)
{
    bool sent = false;
    int i;

    if ((!toSuper) && (argumentCount == 0))
        for (i = 0; (!sent) && ptrNe ((objRef)unSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)unSyms[i]))
            {
                genInstruction (SendUnary, i);
                sent = true;
            }
    if ((!toSuper) && (argumentCount == 1))
        for (i = 0; (!sent) && ptrNe ((objRef)binSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)binSyms[i]))
            {
                genInstruction (SendBinary, i);
                sent = true;
            }
    if (!sent)
    {
        genInstruction (MarkArguments, 1 + argumentCount);
        if (toSuper)
        {
            genInstruction (DoSpecial, SendToSuper);
            genCode (genLiteral ((objRef)messagesym));
        }
        else
            genInstruction (SendMessage, genLiteral ((objRef)messagesym));
    }
}

bool unaryContinuation (bool superReceiver)
{
    int i;
    bool sent;

    while (parseOk && (token == nameconst))
    {
        /* first check to see if it could be a temp by mistake */
        for (i = 1; i < temporaryTop; i++)
            if (streq (tokenString, temporaryName[i]))
                compilWarn (
                    selector, "message same as temporary:", tokenString);
        for (i = 1; i < argumentTop; i++)
            if (streq (tokenString, argumentName[i]))
                compilWarn (selector, "message same as argument:", tokenString);
        /* the next generates too many spurious messages */
        /* for (i=1; i < instanceTop; i++)
           if (streq(tokenString, instanceName[i]))
           compilWarn(selector,"message same as instance",
           tokenString); */

        sent = false;

        if (!sent)
        {
            genMessage (superReceiver, 0, newSymbol (tokenString));
        }
        /* once a message is sent to super, reciever is not super */
        superReceiver = false;
        (void)nextToken ();
    }
    return (superReceiver);
}

bool binaryContinuation (bool superReceiver)
{
    bool superTerm;
    encPtr messagesym;

    superReceiver = unaryContinuation (superReceiver);
    while (parseOk && (token == binary))
    {
        messagesym = newSymbol (tokenString);
        (void)nextToken ();
        superTerm = term ();
        (void)unaryContinuation (superTerm);
        genMessage (superReceiver, 1, messagesym);
        superReceiver = false;
    }
    return (superReceiver);
}

int optimizeBlock (int instruction, bool dopop)
{
    int location;
    enum blockstatus savebstat;

    savebstat = blockstat;
    genInstruction (DoSpecial, instruction);
    location = codeTop;
    genCode (0);
    if (dopop)
        genInstruction (DoSpecial, PopTop);
    (void)nextToken ();
    if (streq (tokenString, "["))
    {
        (void)nextToken ();
        if (blockstat == NotInBlock)
            blockstat = OptimizedBlock;
        body ();
        if (!streq (tokenString, "]"))
            compilError (selector, "missing close", "after block");
        (void)nextToken ();
    }
    else
    {
        (void)binaryContinuation (term ());
        genMessage (false, 0, newSymbol ("value"));
    }
    codeArray[location] = codeTop + 1;
    blockstat = savebstat;
    return (location);
}

bool keyContinuation (bool superReceiver)
{
    int i, j, argumentCount;
    bool sent, superTerm;
    encPtr messagesym;
    char pattern[4096];

    superReceiver = binaryContinuation (superReceiver);
    if (token == namecolon)
    {
        if (streq (tokenString, "ifTrue:"))
        {
            i = optimizeBlock (BranchIfFalse, false);
            if (streq (tokenString, "ifFalse:"))
            {
                codeArray[i] = codeTop + 3;
                (void)optimizeBlock (Branch, true);
            }
        }
        else if (streq (tokenString, "ifFalse:"))
        {
            i = optimizeBlock (BranchIfTrue, false);
            if (streq (tokenString, "ifTrue:"))
            {
                codeArray[i] = codeTop + 3;
                (void)optimizeBlock (Branch, true);
            }
        }
        else if (streq (tokenString, "whileTrue:"))
        {
            j = codeTop;
            genInstruction (DoSpecial, Duplicate);
            genMessage (false, 0, newSymbol ("value"));
            i = optimizeBlock (BranchIfFalse, false);
            genInstruction (DoSpecial, PopTop);
            genInstruction (DoSpecial, Branch);
            genCode (j + 1);
            codeArray[i] = codeTop + 1;
            genInstruction (DoSpecial, PopTop);
        }
        else if (streq (tokenString, "and:"))
            (void)optimizeBlock (AndBranch, false);
        else if (streq (tokenString, "or:"))
            (void)optimizeBlock (OrBranch, false);
        else
        {
            pattern[0] = '\0';
            argumentCount = 0;
            while (parseOk && (token == namecolon))
            {
                (void)strcat (pattern, tokenString);
                argumentCount++;
                (void)nextToken ();
                superTerm = term ();
                (void)binaryContinuation (superTerm);
            }
            sent = false;

            /* check for predefined messages */
            messagesym = newSymbol (pattern);

            if (!sent)
            {
                genMessage (superReceiver, argumentCount, messagesym);
            }
        }
        superReceiver = false;
    }
    return (superReceiver);
}

void continuation (bool superReceiver)
{
    superReceiver = keyContinuation (superReceiver);

    while (parseOk && (token == closing) && streq (tokenString, ";"))
    {
        genInstruction (DoSpecial, Duplicate);
        (void)nextToken ();
        (void)keyContinuation (superReceiver);
        genInstruction (DoSpecial, PopTop);
    }
}

void expression (void)
{
    bool superTerm;
    char assignname[4096];

    if (token == nameconst)
    { /* possible assignment */
        (void)strcpy (assignname, tokenString);
        (void)nextToken ();
        if ((token == binary) && streq (tokenString, "<-"))
        {
            (void)nextToken ();
            assignment (assignname);
        }
        else
        { /* not an assignment after all */
            superTerm = nameTerm (assignname);
            continuation (superTerm);
        }
    }
    else
    {
        superTerm = term ();
        if (parseOk)
            continuation (superTerm);
    }
}

void assignment (char * name)
{
    int i;
    bool done;

    done = false;

    /* it might be a temporary */
    for (i = temporaryTop; (!done) && (i > 0); i--)
        if (streq (name, temporaryName[i]))
        {
            expression ();
            genInstruction (AssignTemporary, i - 1);
            done = true;
        }
    /* or it might be an instance variable */
    for (i = 1; (!done) && (i <= instanceTop); i++)
        if (streq (name, instanceName[i]))
        {
            expression ();
            genInstruction (AssignInstance, i - 1);
            done = true;
        }
    if (!done)
    { /* not known, handle at run time */
        genInstruction (PushArgument, 0);
        genInstruction (PushLiteral, genLiteral ((objRef)newSymbol (name)));
        expression ();
        genMessage (false, 2, newSymbol ("assign:value:"));
    }
}

void statement (void)
{

    if ((token == binary) && streq (tokenString, "^"))
    {
        (void)nextToken ();
        expression ();
        if (blockstat == InBlock)
        {
            /* change return point before returning */
            genInstruction (PushConstant, contextConst);
            genMessage (false, 0, newSymbol ("blockReturn"));
            genInstruction (DoSpecial, PopTop);
        }
        genInstruction (DoSpecial, StackReturn);
    }
    else
    {
        expression ();
    }
}

void body (void)
{
    /* empty blocks are same as nil */
    if ((blockstat == InBlock) || (blockstat == OptimizedBlock))
        if ((token == closing) && streq (tokenString, "]"))
        {
            genInstruction (PushConstant, nilConst);
            return;
        }
    while (parseOk)
    {
        statement ();
        if (token == closing)
            if (streq (tokenString, "."))
            {
                (void)nextToken ();
                if (token == inputend)
                    break;
                else /* pop result, go to next statement */
                    genInstruction (DoSpecial, PopTop);
            }
            else
                break; /* leaving result on stack */
        else if (token == inputend)
            break; /* leaving result on stack */
        else
        {
            compilError (
                selector, "invalid statement ending; token is ", tokenString);
        }
    }
}

void block (void)
{
    int saveTemporary, argumentCount, fixLocation;
    encPtr tempsym, newBlk;
    enum blockstatus savebstat;

    saveTemporary = temporaryTop;
    savebstat = blockstat;
    argumentCount = 0;
    (void)nextToken ();
    if ((token == binary) && streq (tokenString, ":"))
    {
        while (parseOk && (token == binary) && streq (tokenString, ":"))
        {
            if (nextToken () != nameconst)
                compilError (selector,
                             "name must follow colon",
                             "in block argument list");
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            argumentCount++;
            if (temporaryTop > temporaryLimit)
                compilError (selector, "too many temporaries in method", "");
            else
            {
                tempsym = newSymbol (tokenString);
                temporaryName[temporaryTop] =
                    (char *)vonNeumannSpaceOf (tempsym);
            }
            (void)nextToken ();
        }
        if ((token != binary) || !streq (tokenString, "|"))
            compilError (
                selector, "block argument list must be terminated", "by |");
        (void)nextToken ();
    }
    newBlk = newBlock ();
    orefOfPut (
        newBlk, argumentCountInBlock, (objRef)encValueOf (argumentCount));
    orefOfPut (newBlk,
               argumentLocationInBlock,
               (objRef)encValueOf (saveTemporary + 1));
    genInstruction (PushLiteral, genLiteral ((objRef)newBlk));
    genInstruction (PushConstant, contextConst);
    genInstruction (DoPrimitive, 2);
    genCode (29);
    genInstruction (DoSpecial, Branch);
    fixLocation = codeTop;
    genCode (0);
    /*genInstruction(DoSpecial, PopTop); */
    orefOfPut (
        newBlk, bytecountPositionInBlock, (objRef)encValueOf (codeTop + 1));
    blockstat = InBlock;
    body ();
    if ((token == closing) && streq (tokenString, "]"))
        (void)nextToken ();
    else
        compilError (selector, "block not terminated by ]", "");
    genInstruction (DoSpecial, StackReturn);
    codeArray[fixLocation] = codeTop + 1;
    temporaryTop = saveTemporary;
    blockstat = savebstat;
}

void temporaries (void)
{
    encPtr tempsym;

    temporaryTop = 0;
    if ((token == binary) && streq (tokenString, "|"))
    {
        (void)nextToken ();
        while (token == nameconst)
        {
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            if (temporaryTop > temporaryLimit)
                compilError (selector, "too many temporaries in method", "");
            else
            {
                tempsym = newSymbol (tokenString);
                temporaryName[temporaryTop] =
                    (char *)vonNeumannSpaceOf (tempsym);
            }
            (void)nextToken ();
        }
        if ((token != binary) || !streq (tokenString, "|"))
            compilError (selector, "temporary list not terminated by bar", "");
        else
            (void)nextToken ();
    }
}

void selectorDeclPattern (void)
{
    encPtr argsym;

    argumentTop = 0;
    (void)strcpy (selector, tokenString);
    if (token == nameconst) /* unary message pattern */
        (void)nextToken ();
    else if (token == binary)
    { /* binary message pattern */
        (void)nextToken ();
        if (token != nameconst)
            compilError (selector,
                         "binary message pattern not followed by name",
                         selector);
        argsym = newSymbol (tokenString);
        argumentName[++argumentTop] = (char *)vonNeumannSpaceOf (argsym);
        (void)nextToken ();
    }
    else if (token == namecolon)
    { /* keyword message pattern */
        selector[0] = '\0';
        while (parseOk && (token == namecolon))
        {
            (void)strcat (selector, tokenString);
            (void)nextToken ();
            if (token != nameconst)
                compilError (selector,
                             "keyword message pattern",
                             "not followed by a name");
            if (++argumentTop > argumentLimit)
                compilError (selector, "too many arguments in method", "");
            argsym = newSymbol (tokenString);
            argumentName[argumentTop] = (char *)vonNeumannSpaceOf (argsym);
            (void)nextToken ();
        }
    }
    else
        compilError (selector, "illegal message selector", tokenString);
}

bool parse (encPtr method, char * text, bool saveText)
{
    int i;
    encPtr bytecodes, theLiterals;
    byte * bp;

    printf ("PARSE %s\n\n");

    lexinit (text);
    parseOk = true;
    blockstat = NotInBlock;
    codeTop = 0;
    literalTop = temporaryTop = argumentTop = 0;
    maxTemporary = 0;

    printf ("BEGIN ACTUAL\n");

    selectorDeclPattern ();
    if (parseOk)
        temporaries ();
    if (parseOk)
        body ();
    if (parseOk)
    {
        genInstruction (DoSpecial, PopTop);
        genInstruction (DoSpecial, SelfReturn);
    }
    if (!parseOk)
    {
        orefOfPut (method, bytecodesInMethod, (objRef)nilObj);
    }
    else
    {
        bytecodes = newByteArray (codeTop);
        bp = (byte *)vonNeumannSpaceOf (bytecodes);
        for (i = 0; i < codeTop; i++)
        {
            bp[i] = codeArray[i];
        }
        orefOfPut (method, messageInMethod, (objRef)newSymbol (selector));
        orefOfPut (method, bytecodesInMethod, (objRef)bytecodes);
        if (literalTop > 0)
        {
            theLiterals = newArray (literalTop);
            for (i = 1; i <= literalTop; i++)
            {
                orefOfPut (theLiterals, i, literalArray[i]);
            }
            orefOfPut (method, literalsInMethod, (objRef)theLiterals);
        }
        else
        {
            orefOfPut (method, literalsInMethod, (objRef)nilObj);
        }
        orefOfPut (method, stackSizeInMethod, (objRef)encValueOf (6));
        orefOfPut (method,
                   temporarySizeInMethod,
                   (objRef)encValueOf (1 + maxTemporary));
        if (saveText)
        {
            orefOfPut (method, textInMethod, (objRef)newString (text));
        }

        printf ("END ACTUAL1.\n\n");
        return (true);
    }
    printf ("END ACTUAL2.\n\n");

    return (false);
}

void coldClassDef (encPtr strRef)
{
    encPtr superStr;
    encPtr classObj;
    int size;
    lexinit ((char *)vonNeumannSpaceOf (strRef));
    superStr = newString (tokenString);
    (void)nextToken ();
    (void)nextToken ();
    classObj = findClass (tokenString);
    if (streq ((char *)vonNeumannSpaceOf (superStr), "nil"))
        size = 0;
    else
    {
        encPtr superObj;
        superObj = findClass ((char *)vonNeumannSpaceOf (superStr));
        size = intValueOf (orefOf (superObj, sizeInClass).val);
        orefOfPut (classObj, superClassInClass, (objRef)superObj);
        {
            encPtr classMeta = classOf (classObj);
            encPtr superMeta = classOf (superObj);
            orefOfPut (classMeta, superClassInClass, (objRef)superMeta);
        }
    }
    (void)nextToken ();
    (void)nextToken ();
    if (*tokenString)
    {
        encPtr instStr;
        int instTop;
        encPtr instVars[256];
        encPtr varVec;
        int i;
        instStr = newString (tokenString);
        lexinit ((char *)vonNeumannSpaceOf (instStr));
        instTop = 0;
        while (*tokenString)
        {
            instVars[instTop++] = newSymbol (tokenString);
            size++;
            (void)nextToken ();
        }
        varVec = newArray (instTop);
        for (i = 0; i < instTop; i++)
            orefOfPut (varVec, i + 1, (objRef)instVars[i]);
        orefOfPut (classObj, variablesInClass, (objRef)varVec);
        isVolatilePut (instStr, false);
    }
    orefOfPut (classObj, sizeInClass, (objRef)encValueOf (size));
    isVolatilePut (superStr, false);
}

void coldMethods (encVal tagRef)
{
    encPtr strRef;
    encPtr classObj;
    encPtr methTable;
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    if (ptrEq ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
               (objRef)nilObj))
        return;
    if (streq ((char *)vonNeumannSpaceOf (strRef), "}"))
        return;
    lexinit ((char *)vonNeumannSpaceOf (strRef));
    classObj = findClass (tokenString);
    setInstanceVariables (classObj);
    /* now find or create a method table */
    methTable = orefOf (classObj, methodsInClass).ptr;
    if (ptrEq ((objRef)methTable, (objRef)nilObj))
    { /* must make */
        methTable = newDictionary (MethodTableSize);
        orefOfPut (classObj, methodsInClass, (objRef)methTable);
    }
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    while (ptrNe ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
                  (objRef)nilObj))
    {
        encPtr theMethod;
        encPtr selector;
        if (streq ((char *)vonNeumannSpaceOf (strRef), "}"))
            return;
        /* now we have a method */
        theMethod = newMethod ();
        if (parse (theMethod, (char *)vonNeumannSpaceOf (strRef), true))
        {
            orefOfPut (theMethod, methodClassInMethod, (objRef)classObj);
            selector = orefOf (theMethod, messageInMethod).ptr;
            nameTableInsert (
                methTable, oteIndexOf (selector), selector, theMethod);
        }
        else
        {
            /* get rid of unwanted method */
            isVolatilePut (theMethod, false);
        }
    }
}

void coldFileIn (encVal tagRef)
{
    encPtr strRef;
    /* Fix from Zak - cast encPtr to objRef to keep compiler happy.
     * XXX is this actually safe?.
     */
    while (ptrNe ((objRef) (strRef = primGetChunk ((objRef *)&tagRef).ptr),
                  (objRef)nilObj))
    {
        if (streq ((char *)vonNeumannSpaceOf (strRef), "{"))
            coldMethods (tagRef);
        else
            coldClassDef (strRef);
    }
}