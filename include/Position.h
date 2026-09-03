#ifndef POSITION_H
#define POSITION_H

class Position
{
public:
	int row, col;

	Position(int inRow = -1, int inCol = -1) :
		row(inRow), col(inCol)
	{}
};

#endif