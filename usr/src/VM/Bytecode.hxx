#define Extended 0
#define PushInstance 1
#define PushArgument 2
#define PushTemporary 3
#define PushLiteral 4
#define PushConstant 5
#define AssignInstance 6
#define AssignTemporary 7
#define MarkArguments 8
#define SendMessage 9
#define SendUnary 10
#define SendBinary 11
#define DoPrimitive 13
#define DoSpecial 15

#define minusOne 3     /* the value -1 */
#define contextConst 4 /* the current context */
#define nilConst 5     /* the constant nil */
#define trueConst 6    /* the constant true */
#define falseConst 7   /* the constant false */

/* DoSpecial codes */
#define SelfReturn 1
#define StackReturn 2
#define Duplicate 4
#define PopTop 5
#define Branch 6
#define BranchIfTrue 7
#define BranchIfFalse 8
#define AndBranch 9
#define OrBranch 10
#define SendToSuper 11

#define codeLimit 256
#define literalLimit 256
#define temporaryLimit 256
#define argumentLimit 256
#define instanceLimit 256

#define MethodTableSize 39
