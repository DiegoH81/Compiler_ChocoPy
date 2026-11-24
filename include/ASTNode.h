#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <string>
#include <vector>

#include "Position.h"

struct ASTNode;

using ASTPtr = std::shared_ptr<ASTNode>;

struct ASTNode
{
	std::string label;
	Position position;
	std::string type;
	std::vector<ASTPtr> children;

	ASTNode(const std::string& inLabel, const Position& inPos) :
		label(inLabel), position(inPos)
	{}

	bool isLeaf() const
	{
		return children.empty();
	}

	void addChild(const ASTPtr& child)
	{
		if (child)
			children.push_back(child);
	}
};

inline ASTPtr makeNode(const std::string& label, const Position& pos)
{
	return std::make_shared<ASTNode>(label, pos);
}

#endif
