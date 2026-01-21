#pragma once
#include <JParse/Token.h>

class Grammar;

class LCRSNode
{
public:
    LCRSNode(Grammar* grammar);
    ~LCRSNode();
    int GetChildCount() const;
    LCRSNode* GetChild(size_t index) const;
    LCRSNode* NextSibling() const;
    LCRSNode* GetParent() const;
    void AddChild(LCRSNode* newChild);
    void InsertFirst(LCRSNode* newChild);
    void InsertAfter(LCRSNode* child, LCRSNode* newChild);
    LCRSNode* DeleteChild(LCRSNode* child, bool recursive);
    LCRSNode* DetachChild(LCRSNode* child, bool recursive);
    bool HasChildren();
    ASTData* GetData();
    void BorrowData(TokenData* data);
    LCRSNode* FindPrevChild(LCRSNode* key);
private:
    ASTData _data;
    LCRSNode* _right;
    LCRSNode* _down;
    LCRSNode* _parent;
    int _nChild;
};

class LCRSTree
{
public:
	LCRSTree(LCRSNode* root);
    ~LCRSTree();

    LCRSNode* GetRoot() const;
    LCRSNode* FindNode(ASTData* key);
    void PruneTree();
    bool TraversePreOrder(callback_type callback);
    bool TraversePostOrder(callback_type callback);
    void Print(LCRSNode* node, int depth);

    static LCRSNode* CreateNode(Grammar* grammar);
    static bool DeleteNode(LCRSNode* node);
private:
	LCRSNode* _root;
};
