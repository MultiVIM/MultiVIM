#pragma once

#include "AST.hxx"

struct VarNode : Node
{
    bool promoted;
    int promotedIndex;

    enum Kind
    {
        kHeapVar,
        kParentsHeapVar,
        kSpecial,
        kLocal,
        kArgument,
        kInstance,
        kGlobalConstant,
        kGlobal,
    } kind;

    std::string name;

    virtual int getIndex ()
    {
        throw;
    }

    VarNode (Kind kind, std::string name)
        : kind (kind), name (name), promoted (false)
    {
    }

    void generateOn (CodeGen & gen);
    void generateAssignOn (CodeGen & gen, ExprNode * expr);

    /*
     * Generates code to move up this variable into myHeapVars.
     */
    void generatePromoteOn (CodeGen & gen);
};

struct SpecialVarNode : VarNode
{
    SpecialVarNode (std::string name) : VarNode (kSpecial, name)
    {
    }
};

struct IndexedVarNode : VarNode
{
    int index;

    virtual int getIndex ()
    {
        return index;
    }

    IndexedVarNode (int index, Kind kind, std::string name)
        : index (index), VarNode (kind, name)
    {
    }
};

struct LocalVarNode : IndexedVarNode
{
    LocalVarNode (int index, std::string name)
        : IndexedVarNode (index, kLocal, name)
    {
    }
};

struct ArgumentVarNode : IndexedVarNode
{
    ArgumentVarNode (int index, std::string name)
        : IndexedVarNode (index, kArgument, name)
    {
    }
};

struct InstanceVarNode : IndexedVarNode
{
    InstanceVarNode (int idx, std::string name)
        : IndexedVarNode (idx, kInstance, name)
    {
    }
};

struct HeapVarNode : IndexedVarNode
{
    HeapVarNode (int idx, std::string name)
        : IndexedVarNode (idx, kHeapVar, name)
    {
    }
};

struct ParentsHeapVarNode : IndexedVarNode
{
    ParentsHeapVarNode (int idx, std::string name)
        : IndexedVarNode (idx, kParentsHeapVar, name)
    {
    }
};

struct GlobalVarNode : VarNode
{
    GlobalVarNode (std::string name) : VarNode (kGlobal, name)
    {
    }
};

struct AbstractScope
{
    virtual AbstractScope * parent () = 0;

    GlobalVarNode * addClass (ClassNode * aClass);
    virtual VarNode * lookup (std::string aName);
};

struct ClassScope : AbstractScope
{
    /* Index + 1 = Smalltalk index */
    std::vector<InstanceVarNode *> iVars;

    AbstractScope * parent ()
    {
        return NULL;
    }

    void addIvar (std::string name);
    virtual VarNode * lookup (std::string aName);
};

struct BlockNode;

struct AbstractCodeScope : AbstractScope
{

  protected:
    ParentsHeapVarNode * promote (VarNode * aNode);

  public:
    /* Our heapvars that we have promoted herein.
     * We always search here first.
     * Pair of MyHeapVarNode to original VarNode. */
    // if SECOND is a ParentsHeapVarNode, then that's fine
    std::vector<std::pair<HeapVarNode *, VarNode *>> myHeapVars;

    std::vector<ArgumentVarNode *> args;
    std::vector<LocalVarNode *> locals;

    void addArg (std::string name);
    void addLocal (std::string name);
    virtual VarNode * lookupFromBlock (std::string aName) = 0;
};

struct MethodScope : public AbstractCodeScope
{
    ClassScope * _parent;

    MethodScope (ClassScope * parent) : _parent (parent)
    {
    }

    ClassScope * parent ()
    {
        return _parent;
    }

    virtual VarNode * lookup (std::string aName);
    VarNode * lookupFromBlock (std::string aName);
};

struct BlockScope : public AbstractCodeScope
{
    AbstractCodeScope * _parent;
    //   std::vector<ParentsHeapVarNode *> parentsHeapVars;
    /**
     * Imagine nested blocks within a method like so:
     * | x |
     * [ [ x + 1 ] ]
     * The outer block does NOT touch the outer method's temporary `x`;
     * therefore it would not get it promoted. But we need it to be!
     * Therefore if our lookupFromBlock() doesn't find a variable locally,
     * we add the parent's heapvar to our list of heapvars that we need to
     * promote ourselves.
     */

    AbstractCodeScope * parent ()
    {
        return _parent;
    }

    BlockScope (AbstractCodeScope * parent) : _parent (parent)
    {
    }

    virtual VarNode * lookup (std::string aName);
    VarNode * lookupFromBlock (std::string aName);
};