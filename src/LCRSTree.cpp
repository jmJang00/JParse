#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <functional>
#include <stack>
#include <algorithm>
#include <JParse/LCRSTree.h>

LCRSNode::LCRSNode(Grammar* grammar)
{
	_data.grammar = grammar;
	_right = nullptr;
	_down = nullptr;
	_parent = nullptr;
	_nChild = 0;
}

LCRSNode::~LCRSNode()
{
}

int LCRSNode::GetChildCount() const
{
	return _nChild;
}

LCRSNode* LCRSNode::GetChild(size_t index) const
{
	LCRSNode* node = _down;
	while (node != nullptr && index > 0)
	{
		node = node->_right;
		index--;
	}
	return node;
}

LCRSNode* LCRSNode::NextSibling() const
{
	return _right;
}

LCRSNode* LCRSNode::GetParent() const
{
	return _parent;
}

void LCRSNode::AddChild(LCRSNode* newChild)
{
	if (_down == nullptr)
	{
		_down = newChild;
	}
	else
	{
		LCRSNode* tmpNode = _down;
		while (tmpNode->_right != nullptr)
		{
			tmpNode = tmpNode->_right;
		}
		tmpNode->_right = newChild;
	}

	newChild->_parent = this;
	_nChild++;
}

void LCRSNode::InsertFirst(LCRSNode* newChild)
{
	if (_down == nullptr)
	{
		_down = newChild;
	}
	else
	{
		newChild->_right = _down;
		newChild->_parent = this;
		_down = newChild;
	}

	newChild->_parent = this;
	_nChild++;
}

void LCRSNode::InsertAfter(LCRSNode* child, LCRSNode* newChild)
{
	if (newChild == nullptr || child == nullptr)
		return;

	if (_down == nullptr)
	{
		_down = newChild;
	}
	else
	{
		LCRSNode* currNode = _down;

		while (currNode != nullptr)
		{
			if (currNode == child)
			{
				newChild->_right = currNode->_right;
				currNode->_right = newChild;
				break;
			}

			currNode = currNode->_right;
		}
	}

	newChild->_parent = this;
	_nChild++;
}

LCRSNode* LCRSNode::DetachChild(LCRSNode* child, bool recursive)
{
	if (child == nullptr)
		return nullptr;

	if (!recursive)
	{
		LCRSNode* prevChild = FindPrevChild(child);
		LCRSNode* grandChild = child->_down;
		LCRSNode* nextChild = child->_right;
		LCRSNode* lastGrandChild = grandChild;
		if (grandChild != nullptr)
		{
			while (lastGrandChild->_right != nullptr)
			{
				lastGrandChild->_parent = this;
				lastGrandChild = lastGrandChild->_right;
			}
			lastGrandChild->_parent = this;
		}

		// 이전 노드와 연결 재구성
		if (prevChild == nullptr)
		{
			_down = (grandChild ? grandChild : nextChild);
		}
		else
		{
			prevChild->_right = (grandChild ? grandChild : nextChild);
		}

		// 마지막 손자가 있다면 형제 라인에 추가
		if (lastGrandChild != nullptr)
		{
			lastGrandChild->_right = nextChild;
		}

		_nChild += child->_nChild - 1;
		child->_right = nullptr;
		child->_down = nullptr;
		child->_nChild = 0;
		return nextChild;
	}

	LCRSNode* prevNode = nullptr;
	LCRSNode* currNode = _down;

	while (currNode != nullptr)
	{
		if (currNode == child)
		{
			if (prevNode == nullptr)
			{
				_down = currNode->_right;
			}
			else
			{
				prevNode->_right = currNode->_right;
			}

			LCRSNode* nextNode = currNode->_right;
			currNode->_right = nullptr;
			_nChild--;
			return nextNode;
		}

		prevNode = currNode;
		currNode = currNode->_right;
	}

	return nullptr;
}

LCRSNode* LCRSNode::DeleteChild(LCRSNode* child, bool recursive)
{
	LCRSNode* node = DetachChild(child, recursive);
	LCRSTree tree(child);
	tree.PruneTree();
	return node;
}

bool LCRSNode::HasChildren()
{
	return _nChild > 0;
}

ASTData* LCRSNode::GetData()
{
	return &_data;
}

void LCRSNode::BorrowData(TokenData* data)
{
	_data.token = data;
}

LCRSNode* LCRSNode::FindPrevChild(LCRSNode* node)
{
	LCRSNode* currChild = _down;
	LCRSNode* prevChild = nullptr;
	while (currChild != nullptr)
	{
		if (currChild == node)
		{
			return prevChild;
		}

		prevChild = currChild;
		currChild = currChild->_right;
	}
	
	return nullptr;
}

LCRSNode* LCRSTree::FindNode(ASTData* key)
{
	std::stack<LCRSNode*> stack;

	stack.push(_root);

	while (stack.size())
	{
		LCRSNode* node = stack.top();
		stack.pop();
		if (node->GetData() == key)
		{
			return node;
		}

		for (int i = node->GetChildCount() - 1; i >= 0; --i)
		{
			stack.push(node->GetChild(i));
		}
	}

	return nullptr;
}

bool LCRSTree::TraversePreOrder(callback_type callback)
{
	std::stack<LCRSNode*> stack;

	stack.push(GetRoot());
	while (stack.size())
	{
		LCRSNode* node = stack.top();
		if (!callback(node))
		{
			return false;
		}

		stack.pop();

		for (int i = node->GetChildCount() - 1; i >= 0; --i)
		{
			stack.push(node->GetChild(i));
		}
	}

	return true;
}

bool LCRSTree::TraversePostOrder(callback_type callback)
{
	std::stack<std::pair<LCRSNode*, bool>> stack;

	stack.push({ GetRoot(), false });
	while (stack.size())
	{
		auto& pair = stack.top();
		LCRSNode* node = pair.first;
		bool& visited = pair.second;

		if (visited)
		{
			if (!callback(node))
			{
				return false;
			}
			stack.pop();
		}
		else
		{
			visited = true;
			for (int i = node->GetChildCount() - 1; i >= 0; --i)
			{
				stack.push({ node->GetChild(i), false });
			}
		}

	}

	return true;
}

void LCRSTree::PruneTree()
{
	TraversePostOrder(DeleteNode);
}

void LCRSTree::Print(LCRSNode* node, int depth)
{
	if (node == nullptr)
		return;

	for (int i = 0; i < depth; i++)
	{
		printf("    ");
	}
	printf("| %-8s\n", node->GetData()->symbol->ToString());

	if (node->HasChildren())
	{
		LCRSNode* p = node->GetChild(0);

		while (p != nullptr)
		{
			Print(p, depth + 1);
			p = p->NextSibling();
		}
	}
}

LCRSTree::LCRSTree(LCRSNode* root)
	: _root(root)
{
}

LCRSTree::~LCRSTree()
{
}

LCRSNode* LCRSTree::GetRoot() const
{
	return _root;
}

LCRSNode* LCRSTree::CreateNode(Grammar* grammar)
{
	return new LCRSNode(grammar);
}

bool LCRSTree::DeleteNode(LCRSNode* node)
{
	delete node;
	return true;
}

