#include "Maze.h"
#include <iostream>
#include "Application.h"
#include "MyMath.h"

Maze::Maze()
{
}

Maze::~Maze()
{
}

void Maze::Generate(unsigned key, unsigned size, MazePt start, float wallLoad)
{
	m_grid.clear();
	if (size == 0)
		return;
	start.x = Math::Clamp(start.x, 0, (int)size - 1);
	start.y = Math::Clamp(start.y, 0, (int)size - 1);
	wallLoad = Math::Clamp(wallLoad, 0.f, 0.8f);
	unsigned total = size * size;
	m_grid.resize(total);
	m_gridIndex.resize(total);
	std::fill(m_grid.begin(), m_grid.end(), TILE_EMPTY);
	unsigned startId = start.y * size + start.x;
	srand(key);
	for (int i = 0; i < (int)total * wallLoad;)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId)
			continue;
		if (m_grid[chosen] == TILE_EMPTY)
		{
			m_grid[chosen] = TILE_WALL;
			++i;
		}
	}
	std::cout << "Maze " << key << std::endl;
	for (int row = (int)size - 1; row >= 0; --row)
	{
		for (int col = 0; col < (int)size; ++col)
		{
			m_gridIndex[row * size + col] = std::make_pair(col, row);
			if (m_grid[row * size + col] == TILE_WALL)
				std::cout << "1 ";
			else
				std::cout << "0 ";
		}
		std::cout << std::endl;
	}
	m_key = key;
	m_size = size;
	m_numMove = 0;

	AttackOnTitan(size);
}

bool Maze::GenerateNierAutomata(unsigned size, MazePt start, unsigned key, 
	int birthReq, int deathReq, int simulationRounds)
{
	m_grid.clear();
	if (size == 0)
		return false;
	start.x = Math::Clamp(start.x, 0, (int)size - 1);
	start.y = Math::Clamp(start.y, 0, (int)size - 1);
	unsigned total = size * size;
	m_grid.resize(total);
	m_gridIndex.resize(total);
	std::fill(m_grid.begin(), m_grid.end(), TILE_EMPTY);
	unsigned startId = start.y * size + start.x;
	srand(key);
	for (int i = 0; i < (int)total; i++)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId)
			continue;
		if (m_grid[chosen] == TILE_EMPTY)
		{
			m_grid[chosen] = TILE_WALL;
			++i;
		}
	}
	std::cout << "Maze " << key << std::endl;
	for (int row = (int)size - 1; row >= 0; --row)
	{
		for (int col = 0; col < (int)size; ++col)
		{
			m_gridIndex[row * size + col] = std::make_pair(col, row);
			if (m_grid[row * size + col] == TILE_WALL)
				std::cout << "1 ";
			else
				std::cout << "0 ";
		}
		std::cout << std::endl;
	}

	for (int simulation = 0; simulation < simulationRounds; ++simulation)
	{
		for (int row = 0; row < size; ++row)
		{
			for (int col = 0; col < size; ++col)
			{
				if (col * size + row == startId)
					continue;
				int rule = OccupiedSurrounding(row, col, size);
				if (m_gridTemp[col * size + row] == true)
				{
					if (rule < deathReq)
					{
						m_grid[col * size + row] = TILE_EMPTY;
					}
					else
					{
						m_grid[col * size + row] = TILE_WALL;
					}
				}
				else
				{
					rule = 8 - rule;
					if (rule > birthReq)
					{
						m_grid[col * size + row] = TILE_WALL;
					}
					else
					{
						m_grid[col * size + row] = TILE_EMPTY;
					}
				}
			}
		}
	}

	AttackOnTitan(size);
	std::cout << "Maze " << std::endl;
	for (int row = (int)size - 1; row >= 0; --row)
	{
		for (int col = 0; col < (int)size; ++col)
		{
			if (m_grid[row * size + col] == TILE_WALL)
				std::cout << "1 ";
			else
				std::cout << "0 ";
		}
		std::cout << std::endl;
	}
	m_size = size;
	m_key = 0;
	m_numMove = 0;

	return true;
}

bool Maze::GenerateNierAutomata(unsigned size, MazePt start, float initialChance, 
	int birthReq, int deathReq, int simulationRounds)
{
	m_grid.clear();
	m_gridTemp.clear();
	if (size == 0)
		return false;
	if (initialChance <= 0 || initialChance >= 1)
		return false;
	start.x = Math::Clamp(start.x, 0, (int)size - 1);
	start.y = Math::Clamp(start.y, 0, (int)size - 1);
	unsigned total = size * size;
	m_grid.resize(total);
	m_gridIndex.resize(total);
	std::fill(m_grid.begin(), m_grid.end(), TILE_EMPTY);
	m_gridTemp.resize(total);
	std::fill(m_gridTemp.begin(), m_gridTemp.end(), false);
	unsigned startId = start.y * size + start.x;
	srand(time(NULL));
	for (int row = 0; row < size; ++row)
	{
		for (int col = 0; col < size; ++col)
		{
			m_gridIndex[row * size + col] = std::make_pair(col, row);
			if (col * size + row == startId)
				continue;
			float chance = Math::RandFloatMinMax(0,1);
			if (chance < initialChance)
			{
				m_gridTemp[col * size + row] = true;
			}
		}
	}

	for (int simulation = 0; simulation < simulationRounds; ++simulation)
	{
		for (int row = 0; row < size; ++row)
		{
			for (int col = 0; col < size; ++col)
			{
				if (col * size + row == startId)
					continue;
				int rule = OccupiedSurrounding(row, col, size);
				if (m_gridTemp[col * size + row] == true)
				{
					if (rule < deathReq)
					{
						m_grid[col * size + row] = TILE_EMPTY;
					}
					else
					{
						m_grid[col * size + row] = TILE_WALL;
					}
				}
				else
				{
					rule = 8 - rule;
					if (rule > birthReq)
					{
						m_grid[col * size + row] = TILE_WALL;
					}
					else
					{
						m_grid[col * size + row] = TILE_EMPTY;
					}
				}
			}
		}
	}

	AttackOnTitan(size);
	std::cout << "Maze " << std::endl;
	for (int row = (int)size - 1; row >= 0; --row)
	{
		for (int col = 0; col < (int)size; ++col)
		{
			if (m_grid[row * size + col] == TILE_WALL)
				std::cout << "1 ";
			else
				std::cout << "0 ";
		}
		std::cout << std::endl;
	}
	m_size = size;
	m_key = 0;
	m_numMove = 0;

	return true;
}

int Maze::OccupiedSurrounding(int tileX, int tileY, int size)
{
	int count = 0;
	int nextTileX = 0, nextTileY = 0;
	//UP
	if (tileY < size - 1)
	{
		nextTileX = tileX; 
		nextTileY = tileY + 1;
		if (m_gridTemp[nextTileY * size + nextTileX] == true)
		{
			++count;
		}
	}
	//DOWN
	if (tileY > 0)
	{
		nextTileX = tileX; 
		nextTileY = tileY - 1;
		if (m_gridTemp[nextTileY * size + nextTileX] == true)
		{
			++count;
		}
	}
	//LEFT
	if (tileX > 0)
	{
		nextTileX = tileX - 1; 
		nextTileY = tileY;
		if (m_gridTemp[nextTileY * size + nextTileX] == true)
		{
			++count;
		}
	}
	//RIGHT
	if (tileX < size - 1)
	{
		nextTileX = tileX + 1; 
		nextTileY = tileY;
		if (m_gridTemp[nextTileY * size + nextTileX] == true)
		{
			++count;
		}
	}
	//LEFT
	if (tileX % 2 == 0)
	{
		if (tileX > 0 && tileY > 0)
		{
			nextTileX = tileX - 1; 
			nextTileY = tileY - 1;
			if (m_gridTemp[nextTileY * size + nextTileX] == true)
			{
				++count;
			}
		}
	}
	else if (tileX % 2 == 1)
	{
		if (tileX > 0 && tileY < size - 1)
		{
			nextTileX = tileX - 1; 
			nextTileY = tileY + 1;
			if (m_gridTemp[nextTileY * size + nextTileX] == true)
			{
				++count;
			}
		}
	}
	//RIGHT
	if (tileX % 2 == 0)
	{
		if (tileX < size - 1 && tileY > 0)
		{
			nextTileX = tileX + 1; 
			nextTileY = tileY - 1;
			if (m_gridTemp[nextTileY * size + nextTileX] == true)
			{
				++count;
			}
		}
	}
	else if (tileX % 2 == 1)
	{
		if (tileX < size - 1 && tileY < size - 1)
		{
			nextTileX = tileX + 1; 
			nextTileY = tileY + 1;
			if (m_gridTemp[nextTileY * size + nextTileX] == true)
			{
				++count;
			}
		}
	}
	return count;
}

void Maze::TransferGrid(std::vector<Maze::TILE_CONTENT> &m_myGrid, std::vector<std::pair<int, int>>& m_myGridIndex)
{
	m_myGrid = m_grid;
	m_myGridIndex = m_gridIndex;
}

void Maze::AttackOnTitan(unsigned size)
{
	for (int i = 0; i < size; i++)
	{
		m_grid[i] = TILE_WALL;

		m_grid[(size - 1) * size + i] = TILE_WALL;

		m_grid[i * size] = TILE_WALL;

		m_grid[i * size + (size - 1)] = TILE_WALL;
	}
}

unsigned Maze::GetKey()
{
	return m_key;
}

unsigned Maze::GetSize()
{
	return m_size;
}

MazePt Maze::GetCurr()
{
	return m_curr;
}

int Maze::GetNumMove()
{
	return m_numMove;
}

void Maze::SetCurr(MazePt newCurr)
{
	++m_numMove;
	m_curr = newCurr;
}

void Maze::SetNumMove(int num)
{
	m_numMove = num;
}

bool Maze::Move(DIRECTION direction)
{
	//Application::GetInstance().Iterate();
	++m_numMove;
	MazePt temp = m_curr;
	switch (direction)
	{
	case DIR_LEFT:
		if (temp.x == 0)
			return false;
		temp.x -= 1;
		break;
	case DIR_RIGHT:
		if (temp.x == (int)m_size - 1)
			return false;
		temp.x += 1;
		break;
	case DIR_UP:
		if (temp.y == (int)m_size - 1)
			return false;
		temp.y += 1;
		break;
	case DIR_DOWN:
		if (temp.y == 0)
			return false;
		temp.y -= 1;
		break;

	case DIR_LEFTUP:
		if (temp.y == (int)m_size - 1 || temp.x == 0)
			return false;
		temp.x -= 1;
		temp.y += 1;
		break;
	case DIR_LEFTDOWN:
		if (temp.x == 0 || temp.y == 0)
			return false;
		temp.y -= 1;
		temp.x -= 1;

		break;
	case DIR_RIGHTUP:
		if (temp.x == (int)m_size - 1 || temp.y == (int)m_size - 1)
			return false;
		temp.y += 1;
		temp.x += 1;
		break;
	case DIR_RIGHTDOWN:
		if (temp.y == 0 || temp.x == (int)m_size - 1)
			return false;
		temp.x += 1;
		temp.y -= 1;
		break;
	}
	int tempId = temp.y * m_size + temp.x;
	if (m_grid[tempId] == TILE_WALL)
		return false;
	m_curr = temp;
	return true;
}
