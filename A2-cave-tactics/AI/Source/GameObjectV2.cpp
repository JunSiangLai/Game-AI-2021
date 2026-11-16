#include "GameObjectV2.h"
#include "SceneHex.h"
#include <limits>

bool operator<(const std::pair<MazePt, double>& first, const std::pair<MazePt, double>& second)
{
	return first.second > second.second;
}

GameObjectV2::GameObjectV2(GAMEOBJECT_TYPE typeValue)
	: type(typeValue),
	countDown(0),
	scale(5, 5, 5),
	target(0, 0, 0),
	step(0, 0, 0),
	closestObjectPos(0, 0, 0),
	active(false),
	controlled(false),
	wave(false),
	nearDetonator(false),
	escaped(false),
	EngineerDead(false),
	mass(1.f),
	sm(NULL),
	m_currState(NULL),
	m_nextState(NULL),
	Engineer(NULL),
	currOre(NULL),
	closestObject(NULL)
{
	GenerateID();
	m_PostOffice = PostOfficeV2::GetInstance();
	m_PostOffice->Register(ID, this);
	m_visited.resize(SceneHex::m_noGrid * SceneHex::m_noGrid);
	m_cellParent.resize(SceneHex::m_noGrid * SceneHex::m_noGrid);
	m_cellValues.resize(SceneHex::m_noGrid * SceneHex::m_noGrid);
	m_previous.resize(SceneHex::m_noGrid * SceneHex::m_noGrid);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_moved = false;
	m_moveCost = 0;

}

std::set<std::string> GameObjectV2::IDs;
bool GameObjectV2::UpdateVisibility;

GameObjectV2::~GameObjectV2()
{
	if (sm != NULL)
	{
		delete sm;
		sm = NULL;
	}
}

void GameObjectV2::GenerateID()
{
	int i = IDs.size();
	while (IDs.size() == i)
	{
		std::string newID = "";
		for (int j = 0; j < 8; ++j)
		{
			int temp = Math::RandIntMinMax(97,122);
			newID += (char)temp;
		}
		GameObjectV2::IDs.insert(newID);
		ID = newID;
	}
}

GameObjectV2::STATE CurrState(std::string state)
{
	// Grunt
	if (state == "Chase")
		return GameObjectV2::STATE_CHASE;
	else if (state == "Attack")
		return GameObjectV2::STATE_ATTACK;
	else if (state == "Flee")
		return GameObjectV2::STATE_FLEE;
	// Detonator
	else if (state == "Pound")
		return GameObjectV2::STATE_POUND;
	else if (state == "Fireball")
		return GameObjectV2::STATE_FIREBALL;
	else if (state == "SuicideBomb")
		return GameObjectV2::STATE_SUICIDEBOMB;
	// Player
	else if (state == "Search")
		return GameObjectV2::STATE_SEARCH;
	else if (state == "Mine")
		return GameObjectV2::STATE_MINE;
	else if (state == "Run")
		return GameObjectV2::STATE_RUN;
	// Driller
	else if (state == "DriAttack")
		return GameObjectV2::STATE_D_ATTACK;
	else if (state == "DriDefend")
		return GameObjectV2::STATE_D_DEFEND;
	// Engineer
	else if (state == "EngDefend")
		return GameObjectV2::STATE_E_DEFEND;
	else if (state == "Death")
		return GameObjectV2::STATE_DEATH;
	else
		return GameObjectV2::STATE_NONE;
}

void GameObjectV2::SetNextState(const std::string& nextStateID)
{
	State* it = sm->getState(nextStateID);
	if (it != nullptr)
	{
		m_nextState = it;
	}
}

const std::string GameObjectV2::GetCurrentState()
{
	if (m_currState)
		return m_currState->GetStateID();
	return "<No states>";
}

bool GameObjectV2::Handle(MessageV2* Message)
{
	MessageV2* messageWRU = dynamic_cast<MessageV2*>(Message);
	if (messageWRU)
	{
		m_inbox.push(Message);
		return true;
	}
	return false;
}

void GameObjectV2::Read()
{
	while (m_inbox.size() > 0)
	{
		MessageV2* message = m_inbox.front();

		switch (message->type)
		{
		case MessageV2::MSG_NEARESTPLAYER:
		{
			closestObject = message->go;
			closestObjectDist = message->threshold;
			break;
		}
		case MessageV2::MSG_NEARDETONATOR:
		{
			nearDetonator = message->isTrue;
			break;
		}
		case MessageV2::MSG_NEARESTENEMY:
		{
			closestObject = message->go;
			closestObjectDist = message->threshold;
			break;
		}
		case MessageV2::MSG_DRILLERLOCATION:
		{
			driller = message->pos;
			break;
		}
		case MessageV2::MSG_ENGINEERLOCATION:
		{
			closestObjectPos = message->pos;
			break;
		}
		case MessageV2::MSG_NEARESTORE:
		{
			closestObject = message->go;
			break;
		}
		case MessageV2::MSG_WAVE:
		{
			wave = message->isTrue;
			break;
		}
		case MessageV2::MSG_DMG:
		{
			currHealth -= message->threshold;
			break;
		}
		case MessageV2::MSG_DROPPOD:
		{
			closestObjectPos = message->pos;
			break;
		}
		}
		delete message;
		m_inbox.pop();
	}
}

bool GameObjectV2::AStar(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_priorQueue.empty())
		m_priorQueue.pop();
	m_shortestPath.clear();
	m_priorQueue.push(std::make_pair(start, 0));
	// Reset Nodes
	std::fill(m_cellValues.begin(), m_cellValues.end(), std::pair<double, double>(FLT_MAX, FLT_MAX));
	std::fill(m_cellParent.begin(), m_cellParent.end(), std::pair<int, int>(-1, -1));

	// Reset Starting Node
	m_cellValues[start.y * SceneHex::m_noGrid + start.x].first = 0.f;
	m_cellValues[start.y * SceneHex::m_noGrid + start.x].second = 0.f;
	m_cellParent[start.y * SceneHex::m_noGrid + start.x].first = start.x;
	m_cellParent[start.y * SceneHex::m_noGrid + start.x].second = start.y;
	while (!m_priorQueue.empty())
	{
		/*for (int i = 0; i < m_priorQueue.size(); i++)
		{
			for (int j = 0; j < m_priorQueue.size(); j++)
			{
				if (m_priorQueue[j] < )
			}
		}*/
		MazePt curr = m_priorQueue.top().first;
		//SceneHex::m_maze.SetCurr(curr);
		m_priorQueue.pop();
		m_visited[curr.y * SceneHex::m_noGrid + curr.x] = true;

		// Destination
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = MazePt(m_cellParent[curr.y * SceneHex::m_noGrid + curr.x].first,
					m_cellParent[curr.y * SceneHex::m_noGrid + curr.x].second);
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//RIGHT
		if (curr.x < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//LEFT
		if (curr.x % 2 == 0)
		{
			if (curr.x > 0 && curr.y > 0)
			{
				MazePt next(curr.x - 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x > 0 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x - 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}


		//RIGHT
		if (curr.x % 2 == 0)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y > 0)
			{
				MazePt next(curr.x + 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x + 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] > 0)
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
	}

	return false;
}

bool GameObjectV2::NoWallAStar(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_priorQueue.empty())
		m_priorQueue.pop();
	m_shortestPath.clear();
	m_priorQueue.push(std::make_pair(start, 0));
	// Reset Nodes
	std::fill(m_cellValues.begin(), m_cellValues.end(), std::pair<double, double>(FLT_MAX, FLT_MAX));
	std::fill(m_cellParent.begin(), m_cellParent.end(), std::pair<int, int>(-1, -1));

	// Reset Starting Node
	m_cellValues[start.y * SceneHex::m_noGrid + start.x].first = 0.f;
	m_cellValues[start.y * SceneHex::m_noGrid + start.x].second = 0.f;
	m_cellParent[start.y * SceneHex::m_noGrid + start.x].first = start.x;
	m_cellParent[start.y * SceneHex::m_noGrid + start.x].second = start.y;
	while (!m_priorQueue.empty())
	{
		/*for (int i = 0; i < m_priorQueue.size(); i++)
		{
			for (int j = 0; j < m_priorQueue.size(); j++)
			{
				if (m_priorQueue[j] < )
			}
		}*/
		MazePt curr = m_priorQueue.top().first;
		//SceneHex::m_maze.SetCurr(curr);
		m_priorQueue.pop();
		m_visited[curr.y * SceneHex::m_noGrid + curr.x] = true;

		// Destination
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = MazePt(m_cellParent[curr.y * SceneHex::m_noGrid + curr.x].first,
					m_cellParent[curr.y * SceneHex::m_noGrid + curr.x].second);
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//RIGHT
		if (curr.x < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
			{
				double value, cost, fcost;
				cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
				fcost = Math::Max(end.x - next.x, end.y - next.y);
				value = cost + fcost;
				if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
				{
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
					m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
					m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
					m_priorQueue.push(std::make_pair(next, value));
				}
			}
		}
		//LEFT
		if (curr.x % 2 == 0)
		{
			if (curr.x > 0 && curr.y > 0)
			{
				MazePt next(curr.x - 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x > 0 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x - 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}


		//RIGHT
		if (curr.x % 2 == 0)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y > 0)
			{
				MazePt next(curr.x + 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x + 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x])
				{
					double value, cost, fcost;
					cost = m_cellValues[curr.y * SceneHex::m_noGrid + curr.x].second + 1 + SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] * SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x];
					fcost = Math::Max(end.x - next.x, end.y - next.y);
					value = cost + fcost;
					if (m_cellValues[next.y * SceneHex::m_noGrid + next.x].first == FLT_MAX ||
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first > value)
					{
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].first = value;
						m_cellValues[next.y * SceneHex::m_noGrid + next.x].second = cost;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].first = curr.x;
						m_cellParent[next.y * SceneHex::m_noGrid + next.x].second = curr.y;
						m_priorQueue.push(std::make_pair(next, value));
					}
				}
			}
		}
	}

	return false;
}


bool GameObjectV2::BFS(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_queue.empty())
		m_queue.pop();
	m_shortestPath.clear();
	m_queue.push(start);
	while (!m_queue.empty())
	{
		MazePt curr = m_queue.front();
		SceneHex::m_maze.SetCurr(curr);
		m_queue.pop();
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = m_previous[curr.y * SceneHex::m_noGrid + curr.x];
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
			}
		}
		//RIGHT
		if (curr.x < SceneHex::m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x % 2 == 0)
		{
			if (curr.x > 0 && curr.y > 0)
			{
				MazePt next(curr.x - 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
				{
					m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
					m_queue.push(next);
					m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x > 0 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x - 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
				{
					m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
					m_queue.push(next);
					m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
				}
			}
		}


		//RIGHT
		if (curr.x % 2 == 0)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y > 0)
			{
				MazePt next(curr.x + 1, curr.y - 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
				{
					m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
					m_queue.push(next);
					m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
				}
			}
		}
		else if (curr.x % 2 == 1)
		{
			if (curr.x < SceneHex::m_noGrid - 1 && curr.y < SceneHex::m_noGrid - 1)
			{
				MazePt next(curr.x + 1, curr.y + 1);
				if (!m_visited[next.y * SceneHex::m_noGrid + next.x] && SceneHex::m_myGrid[next.y * SceneHex::m_noGrid + next.x] == Maze::TILE_EMPTY)
				{
					m_previous[next.y * SceneHex::m_noGrid + next.x] = curr;
					m_queue.push(next);
					m_visited[next.y * SceneHex::m_noGrid + next.x] = true;
				}
			}
		}
	}

	return false;
}

void GameObjectV2::Update(double dt)
{
	if (!active)
		return;
	
	if (type == GameObjectV2::GO_DRILLER || GameObjectV2::GO_ENGINEER && currHealth < maxHealth)
	{
		currHealth += 10 * dt;
		if (currHealth > maxHealth)
		{
			currHealth = maxHealth;
		}
	}
	// Process Messages
	Read();

	if (controlled)
	{
		if (SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x] == Maze::TILE_ORE && m_moveCost <= 0)
		{
			SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x] = Maze::TILE_EMPTY;
		}
	}
	else
	{
		// State
		if (sm != NULL)
		{
			sm->Update(this, dt);
		}
	}

	// Turret shooting
	if (type == GameObjectV2::GO_TURRET)
	{
		MessageV2* msg = new MessageV2(this, MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		m_PostOffice->Send("SceneMining", msg);
		atkCD -= dt;
		if (closestObject != NULL)
		{
			if (atkCD <= 0 && closestObjectDist < 20)
			{
				MessageV2* msg = new MessageV2(NULL, MessageV2::MSG_DMG, 15.f, Vector3(0, 0, 0), false);
				m_PostOffice->Send(closestObject->ID, msg);
				atkCD = atkSpd;
			}
		}
	}

	// Movement
	if (m_shortestPath.size() != 0 && !m_moved && m_moveCost <= 0)
	{
		m_moved = true;
		UpdateVisibility = true;
		pos = Vector3(m_shortestPath.front().x, m_shortestPath.front().y, 0);
		std::cout << pos;
		if (type == GO_DRILLER)
		{
			if (SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x] < 0)
			{
				SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x] = Maze::TILE_MUD;
				m_moveCost = Maze::TILE_MUD + 1;
			}
			else
			{
				m_moveCost = SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x];
			}
		}
		else
		{
			m_moveCost = SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x];
		}
		if (SceneHex::m_myGrid[pos.y * SceneHex::m_noGrid + pos.x] == Maze::TILE_ORE && controlled && m_shortestPath.empty())
		{
			m_moveCost += 2;
		}
		m_shortestPath.erase(m_shortestPath.begin());
	}
	if (!m_moved)
	{
		m_moved = true;
	}
}