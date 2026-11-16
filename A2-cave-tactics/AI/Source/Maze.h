#ifndef MAZE_H
#define MAZE_H

#include <vector>

struct MazePt
{
	int x, y;
	MazePt(int _x = 0, int _y = 0) : x(_x), y(_y) { }
	void Set(int _x = 0, int _y = 0) { x = _x; y = _y; }

};

class Maze
{
public:
	enum TILE_CONTENT
	{
		TILE_WALL = -1,
		TILE_ROCK = -2,
		TILE_DIRT = -3,
		TILE_HEALTHORE = -5,
		TILE_FOG = 0,
		TILE_ORE = 0,
		TILE_EMPTY = 1,
		TILE_WATER,
		TILE_MUD,
	};
	enum DIRECTION
	{
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		DIR_LEFTUP, DIR_LEFTDOWN, DIR_RIGHTUP, DIR_RIGHTDOWN,
	};
	Maze();
	~Maze();
	void Generate(unsigned key, unsigned size, MazePt start, float wallLoad);
	bool GenerateNierAutomata(unsigned size, MazePt start, unsigned key,
		int birthReq, int deathReq, int simulationRounds);
	bool GenerateNierAutomata(unsigned size, MazePt start, float initialChance,
		int birthReq, int deathReq, int simulationRounds);
	void AttackOnTitan(unsigned size);
	int OccupiedSurrounding(int tileX, int tileY, int size);
	bool Move(DIRECTION direction);
	unsigned GetKey();
	unsigned GetSize();
	MazePt GetCurr();
	int GetNumMove();
	void SetCurr(MazePt newCurr);
	void SetNumMove(int num);

	void TransferGrid(std::vector<Maze::TILE_CONTENT>& m_myGrid, std::vector<std::pair<int, int>> &m_myGridIndex);
private:
	//Do not make these public
	unsigned m_key;
	unsigned m_size;
	std::vector<bool> m_gridTemp;
	std::vector<TILE_CONTENT> m_grid;
	std::vector<std::pair<int, int>> m_gridIndex;
	MazePt m_curr;
	int m_numMove;
};

#endif