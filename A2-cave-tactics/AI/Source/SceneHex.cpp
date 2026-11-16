#include "SceneHex.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <algorithm>
#include "StatesGrunts.h"
#include "StatesDetonator.h"
#include "StatesPlayer.h"
#include "StatesCommon.h"
#include "PostOfficeV2.h"
#include "GameObjectV2.h"

int SceneHex::m_noGrid;
float SceneHex::m_gridSize;
float SceneHex::m_gridOffset;
bool SceneHex::m_turnJustEnded;
std::vector<Maze::TILE_CONTENT> SceneHex::m_myGrid;
Maze SceneHex::m_maze;

GameObjectV2* SceneHex::FetchGO(GameObjectV2::GAMEOBJECT_TYPE type)
{
	switch (type)
	{
	case GameObjectV2::GO_GRUNT:
		for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
		{
			GameObjectV2* grunt = (GameObjectV2*)*it;
			if (!grunt->active)
			{
				grunt->active = true;
				grunt->moveSpeed = grunt->baseSpeed;
				grunt->atkCD = 0;
				grunt->currHealth = grunt->maxHealth;
				grunt->m_currState = NULL;
				grunt->target.SetZero();
				grunt->SetNextState("Chase");
				return grunt;
			}
		}
		for (unsigned i = 0; i < 5; ++i)
		{
			GameObjectV2* go = new GameObjectV2(type);
			go->active = false;
			go->baseSpeed = 12;
			go->atkSpd = 1;
			go->maxHealth = 40 * (m_playerList.size() * 0.5);
			go->sm = GruntState;
			m_gruntList.push_back(go);
		}
		return FetchGO(type);
		break;
	case GameObjectV2::GO_DETONATOR:
		for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
		{
			GameObjectV2* detonator = (GameObjectV2*)*it;
			if (!detonator->active)
			{
				detonator->active = true;
				detonator->moveSpeed = detonator->baseSpeed;
				detonator->atkCD = 0;
				detonator->currHealth = detonator->maxHealth;
				detonator->m_currState = NULL;
				detonator->target.SetZero();
				detonator->SetNextState("Fireball");
				return detonator;
			}
		}
		for (unsigned i = 0; i < 5; ++i)
		{
			GameObjectV2* go = new GameObjectV2(type);
			go->active = false;
			go->baseSpeed = 6;
			go->atkSpd = 2;
			go->maxHealth = 300 * (m_playerList.size() * 0.5);
			go->sm = DetonatorState;
			m_detonatorList.push_back(go);
		}
		return FetchGO(type);
		break;
	case GameObjectV2::GO_TURRET:
	{
		for (auto it = m_turretList.begin(); it != m_turretList.end(); ++it)
		{
			GameObjectV2* turret = (GameObjectV2*)*it;
			if (!turret->active)
			{
				turret->atkCD = 0;
				return turret;
			}
		}
	}
	default:
		break;
	}
}

void SceneHex::UpdateVisibility()
{
	std::fill(m_render.begin(), m_render.end(), false);
	LOStriangles.clear();
	for (auto player : m_playerList)
	{
		CALCVISIBILITY(MazePt(player->pos.x, player->pos.y), 1000);
	}
	GameObjectV2::UpdateVisibility = false;
}

SceneHex::SceneHex()
{
}

SceneHex::~SceneHex()
{
}

void SceneHex::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_PostOffice = PostOfficeV2::GetInstance();
	m_PostOffice->Register("SceneMining", this);
	
	m_noGrid = 20;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	// Debug
	debugRay = false;

	// Turn
	m_currTurn = PLAYER;
	m_turnOver = false;
	m_turnNo = 1;
	playerControlled = 0;

	// Hex
	m_start.Set(1, 1);
	m_maze.SetCurr(m_start);
	m_mazeKey = 0;
	m_wallLoad = 0.3f;
	//m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
	m_maze.GenerateNierAutomata(m_noGrid, m_start, 0.6f, 2, 6, 1); //Generate new maze
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_myGridIndex.resize(m_noGrid * m_noGrid);
	m_visited.resize(SceneHex::m_noGrid * SceneHex::m_noGrid);
	m_render.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;

	m_myMazePt.resize(m_noGrid * m_noGrid);
	m_maze.TransferGrid(m_myGrid, m_myGridIndex);
	DFSGenerateEdgeAndVertices(m_start);
	CALCVISIBILITY(m_start, 1000);

	// A1
	// Grunts
	GruntState = new StateMachineV2();
	GruntState->AddState(new StateChase("Chase"));
	GruntState->AddState(new StateAttack("Attack"));
	GruntState->AddState(new StateFlee("Flee"));
	GruntState->AddState(new StateDeath("Death"));

	// Detonators
	DetonatorState = new StateMachineV2();
	DetonatorState->AddState(new StatePound("Pound"));
	DetonatorState->AddState(new StateFireball("Fireball"));
	DetonatorState->AddState(new StateSuicideBomb("SuicideBomb"));
	DetonatorState->AddState(new StateDeath("Death"));

	// Engineer
	EngineerState = new StateMachineV2();
	EngineerState->AddState(new StatePlayerSearch("Search"));
	EngineerState->AddState(new StatePlayerMine("Mine"));
	EngineerState->AddState(new StateEngineerDefend("EngDefend"));
	EngineerState->AddState(new StatePlayerRun("Run"));
	EngineerState->AddState(new StatePlayerExtract("Extract"));
	EngineerState->AddState(new StateDeath("Death"));

	// Driller
	DrillerState = new StateMachineV2();
	DrillerState->AddState(new StatePlayerSearch("Search"));
	DrillerState->AddState(new StatePlayerMine("Mine"));
	DrillerState->AddState(new StateDrillerAttack("DriAttack"));
	DrillerState->AddState(new StateDrillerDefend("DriDefend"));
	DrillerState->AddState(new StatePlayerRun("Run"));
	DrillerState->AddState(new StatePlayerExtract("Extract"));
	DrillerState->AddState(new StateDeath("Death"));

	//DropPod
	m_DropPod = new GameObjectV2(GameObjectV2::GO_DROPPOD);
	m_DropPod->moveSpeed = 0;
	m_DropPod->active = false;
	//m_DropPod->pos = Vector3(sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), 0);

	// Engineer
	GameObjectV2* m_Engineer = new GameObjectV2();
	m_Engineer->type = GameObjectV2::GO_ENGINEER;
	m_Engineer->active = true;
	m_Engineer->baseSpeed = m_Engineer->moveSpeed = 10;
	m_Engineer->maxHealth = m_Engineer->currHealth = 200;
	m_Engineer->atkSpd = 0.6;
	m_Engineer->target.SetZero();
	m_Engineer->pos = m_Engineer->step = Vector3(1, 1, 0);
	m_Engineer->sm = EngineerState;
	m_Engineer->SetNextState("Search");
	msg = new MessageV2(m_Engineer, MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
	m_PostOffice->Send("SceneMining", msg);
	m_playerList.push_back(m_Engineer);
	// Turret
	for (int i = 0; i < 3; ++i)
	{
		GameObjectV2* turret = new GameObjectV2(GameObjectV2::GO_TURRET);
		turret->baseSpeed = turret->moveSpeed = 0.f;
		turret->active = false;
		turret->atkCD = 0;
		turret->atkSpd = 1.5;
		m_turretList.push_back(turret);
	}

	// Driller
	GameObjectV2* m_Driller = new GameObjectV2();
	m_Driller->type = GameObjectV2::GO_DRILLER;
	m_Driller->active = true;
	m_Driller->baseSpeed = m_Driller->moveSpeed = 10;
	m_Driller->maxHealth = m_Driller->currHealth = 200;
	m_Driller->atkSpd = 1;
	m_Driller->target.SetZero();
	m_Driller->pos = m_Driller->step = Vector3(2, 1, 0);
	m_Driller->sm = DrillerState;
	m_Driller->SetNextState("Search");
	msg = new MessageV2(m_Driller, MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
	m_PostOffice->Send("SceneMining", msg);
	m_playerList.push_back(m_Driller);

	for (auto player : m_playerList)
	{
		m_myGrid[player->pos.y * m_noGrid + player->pos.x] = Maze::TILE_EMPTY;
	}

	// Resources
	for (int i = 0; i < 10; ++i)
	{
		GameObjectV2* ore = new GameObjectV2(GameObjectV2::GO_ORE);
		ore->active = true;
		int x = Math::RandIntMinMax(0, m_noGrid - 1);
		int y = Math::RandIntMinMax(0, m_noGrid - 1);
		while (m_myGrid[y * m_noGrid + x] < 0)
		{
			x = Math::RandIntMinMax(0, m_noGrid - 1);
			y = Math::RandIntMinMax(0, m_noGrid - 1);
		}
		ore->pos.Set(x, y, 0);
		m_resourceList.push_back(ore);
	}

	// Wave Timer
	m_waveTimer = 15;

	// Crystal Count
	crystals = 0;
	nextFrame = setUp = end = win = false;

	m_cameraZoom = 1.f;

}

//GameObject* SceneHex::FetchGO()
//{
//	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
//	{
//		GameObject *go = (GameObject *)*it;
//		if (!go->active)
//		{
//			go->active = true;
//			++m_objectCount;
//			return go;
//		}
//	}
//	for (unsigned i = 0; i < 10; ++i)
//	{
//		GameObject *go = new GameObject(GameObject::GO_NONE);
//		m_goList.push_back(go);
//	}
//	return FetchGO();
//}

void SceneHex::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_UP) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_DOWN);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_DOWN) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_UP);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_LEFT) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_RIGHT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_RIGHT) == true)
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
				DFS(next);
				m_maze.Move(Maze::DIR_LEFT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	
	if (curr.x % 2 == 0)
	{
		//left down
		if (curr.x > 0 && curr.y > 0) //leftdown
		{
			MazePt next(curr.x - 1, curr.y - 1);
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				if (m_maze.Move(Maze::DIR_LEFTDOWN) == true)
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
					DFS(next);
					m_maze.Move(Maze::DIR_RIGHTUP);
				}
				else
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
				}
			}
		}

		//right down
		if (curr.x < m_noGrid - 1 && curr.y > 0)
		{
			MazePt next(curr.x + 1, curr.y - 1);
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				if (m_maze.Move(Maze::DIR_RIGHTDOWN) == true)
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
					DFS(next);
					m_maze.Move(Maze::DIR_LEFTUP);
				}
				else
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
				}
			}
		}
	}
	else if (curr.x % 2 == 1)
	{
		//left up
		if (curr.x > 0 && curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x - 1, curr.y + 1);

			if (!m_visited[next.y * m_noGrid + next.x])
			{
				if (m_maze.Move(Maze::DIR_LEFTUP))
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
					DFS(next);
					m_maze.Move(Maze::DIR_RIGHTDOWN);
				}
				else
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
				}
			}
		}

		//right up
		if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y + 1);

			if (!m_visited[next.y * m_noGrid + next.x])
			{
				if (m_maze.Move(Maze::DIR_RIGHTUP))
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
					DFS(next);
					m_maze.Move(Maze::DIR_LEFTDOWN);
				}
				else
				{
					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
				}
			}
		}
	}

}

void SceneHex::DFSGenerateEdgeAndVertices(MazePt curr)
{
	bool leftup = false;
	bool rightup = false;
	bool leftdown = false;
	bool rightdown = false;
	bool up = false;
	bool down = false;
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (m_myGrid[next.y * m_noGrid + next.x] < 0)
		{
			up = true;
		}
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			DFSGenerateEdgeAndVertices(next);
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (m_myGrid[next.y * m_noGrid + next.x] < 0)
		{
			down = true;
		}
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			DFSGenerateEdgeAndVertices(next);
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (m_myGrid[next.y * m_noGrid + next.x] < 0)
		{
			if (curr.x % 2 == 0)
			{
				leftup = true;
			}
			else
			{
				leftdown = true;
			}
		}
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			DFSGenerateEdgeAndVertices(next);
			
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (m_myGrid[next.y * m_noGrid + next.x] < 0)
		{
			if (curr.x % 2 == 0)
			{
				rightup = true;
			}
			else
			{
				rightdown = true;
			}
		}
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			DFSGenerateEdgeAndVertices(next);
		}
	}

	if (curr.x % 2 == 0)
	{
		//left down
		if (curr.x > 0 && curr.y > 0) //leftdown
		{
			MazePt next(curr.x - 1, curr.y - 1);
			if (m_myGrid[next.y * m_noGrid + next.x] < 0)
			{
				leftdown = true;
			}
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				DFSGenerateEdgeAndVertices(next);
			}
		}

		//right down
		if (curr.x < m_noGrid - 1 && curr.y > 0)
		{
			MazePt next(curr.x + 1, curr.y - 1);
			if (m_myGrid[next.y * m_noGrid + next.x] < 0)
			{
				rightdown = true;
			}
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				DFSGenerateEdgeAndVertices(next);
			}
		}
	}
	else if (curr.x % 2 == 1)
	{
		//left up
		if (curr.x > 0 && curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x - 1, curr.y + 1);

			if (m_myGrid[next.y * m_noGrid + next.x] < 0)
			{
				leftup = true;
			}
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				DFSGenerateEdgeAndVertices(next);
			}
		}

		//right up
		if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y + 1);

			if (m_myGrid[next.y * m_noGrid + next.x] < 0)
			{
				rightup = true;
			}
			if (!m_visited[next.y * m_noGrid + next.x])
			{
				DFSGenerateEdgeAndVertices(next);
			}
		}
	}
	std::vector < std::pair<float, float> > temp;
	std::vector < std::pair<float, float> > makeEdge;
	makeEdge.resize(7);
	std::fill(makeEdge.begin(), makeEdge.end(), std::make_pair(INFINITY, INFINITY));
	float size = 0.6 * m_gridSize;
	for (int slice = 0; slice < 6; ++slice)
	{
		float theta = slice * 360.f / 6;
		switch (slice)
		{
		case 1:
		{
			if (!(up && rightup && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[1] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		case 0:
		{
			if (!(rightdown && rightup && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[0] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
				makeEdge[6] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		case 5:
		{
			if (!(down && rightdown && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[5] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		case 4:
		{
			if (!(down && leftdown && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[4] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		case 3:
		{
			if (!(leftup && leftdown && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[3] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		case 2:
		{
			if (!(up && leftup && m_myGrid[curr.y * m_noGrid + curr.x] < 0))
			{
				makeEdge[2] = (std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			}
			temp.push_back(std::make_pair(size * cos(Math::DegreeToRadian(theta)) + ((m_gridOffset + m_gridSize * curr.x) * 0.9), size * sin(Math::DegreeToRadian(theta)) + (m_gridOffset + m_gridSize * curr.y)));
			break;
		}
		}
	}
	if (m_myGrid[curr.y * m_noGrid + curr.x] < 0)
	{
		for (int i = 0; i < 7; i++)
		{
			if (curr.x % 2 == 0)
				makeEdge[i].second -= m_gridOffset;
		}
		for (int i = 0; i < 6; i++)
		{
			switch (i)
			{
			case 0:
			{
				if (rightup)
				{
					continue;
				}
				break;
			}
			case 1:
			{
				if (up)
				{
					continue;
				}
				break;
			}
			case 2:
			{
				if (leftup)
				{
					continue;
				}
				break;
			}
			case 3:
			{
				if (leftdown)
				{
					continue;
				}
				break;
			}
			case 4:
			{
				if (down)
				{
					continue;
				}
				break;
			}
			case 5:
			{
				if (rightdown)
				{
					continue;
				}
				break;
			}
			}
			if (makeEdge[i].first == INFINITY || makeEdge[i + 1].first == INFINITY)
			{
				continue;
			}
			else
			{
				m_myEdges.push_back(Edge(makeEdge[i].first, makeEdge[i].second, makeEdge[i + 1].first, makeEdge[i + 1].second));
			}
		}
	}
	m_myMazePt[curr.y * m_noGrid + curr.x] = temp;
}

void SceneHex::VISIBLEBFS(MazePt start, int range)
{
	std::fill(m_render.begin(), m_render.end(), false);
	while (!m_queue.empty())
		m_queue.pop();
	m_queue.push(start);
	std::queue<MazePt> temp; //queue for BFS

	while (!m_queue.empty())
	{
		MazePt curr = m_queue.front();
		m_queue.pop();

		//UP
		if (curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_render[next.y * m_noGrid + next.x])
			{

				temp.push(next);
				m_render[next.y * m_noGrid + next.x] = true;
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_render[next.y * m_noGrid + next.x])
			{
				temp.push(next);
				m_render[next.y * m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_render[next.y * m_noGrid + next.x])
			{
				temp.push(next);
				m_render[next.y * m_noGrid + next.x] = true;
			}
		}
		//RIGHT
		if (curr.x < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_render[next.y * m_noGrid + next.x])
			{
				temp.push(next);
				m_render[next.y * m_noGrid + next.x] = true;
			}
		}
		if (curr.x % 2 == 0)
		{
			if (curr.x > 0 && curr.y > 0)
			{
				MazePt next(curr.x - 1, curr.y - 1);
				if (!m_render[next.y * m_noGrid + next.x])
				{
					temp.push(next);
					m_render[next.y * m_noGrid + next.x] = true;
				}
			}
		}
		else
		{
			if (curr.x > 0 && curr.y < m_noGrid - 1)
			{
				MazePt next(curr.x - 1, curr.y + 1);
				if (!m_render[next.y * m_noGrid + next.x])
				{
					temp.push(next);
					m_render[next.y * m_noGrid + next.x] = true;
				}
			}
		}
		if (curr.x % 2 == 0)
		{
			if (curr.x < m_noGrid - 1 && curr.y > 0)
			{
				MazePt next(curr.x + 1, curr.y - 1);
				if (!m_render[next.y * m_noGrid + next.x])
				{
					temp.push(next);
					m_render[next.y * m_noGrid + next.x] = true;
				}
			}
		}
		else
		{
			if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
			{
				MazePt next(curr.x + 1, curr.y + 1);
				if (!m_render[next.y * m_noGrid + next.x])
				{
					temp.push(next);
					m_render[next.y * m_noGrid + next.x] = true;
				}
			}
		}

		if (m_queue.empty())
		{
			--range;
			if (range > 0)
			{
				m_queue = temp;
			}
		}
	}
}

void SceneHex::VISIBLEDFS(MazePt start, Maze::DIRECTION dir)
{
	if (start.x % 2 == 0)
	{
		if (dir == Maze::DIR_LEFTDOWN)
		{
			dir = Maze::DIR_LEFT;
		}
		else if (dir == Maze::DIR_RIGHTDOWN)
		{
			dir = Maze::DIR_RIGHT;
		}
	}
	else
	{
		if (dir == Maze::DIR_LEFT)
		{
			dir = Maze::DIR_LEFTDOWN;
		}
		else if (dir == Maze::DIR_RIGHT)
		{
			dir = Maze::DIR_RIGHTDOWN;
		}
	}
	MazePt curr = start;
	m_render[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1)
	{
		if (dir == Maze::DIR_UP)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
			{
				VISIBLEDFS(next, dir);
			}
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		if (dir == Maze::DIR_DOWN)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
			{
				VISIBLEDFS(next, dir);
			}
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		if (dir == Maze::DIR_LEFT)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
			{
				VISIBLEDFS(next, dir);
			}
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		if (dir == Maze::DIR_RIGHT)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
			{
				VISIBLEDFS(next, dir);
			}
		}
	}

	if (curr.x % 2 == 0)
	{
		//left down
		if (curr.x > 0 && curr.y > 0) //leftdown
		{
			if (dir == Maze::DIR_LEFTDOWN || dir == Maze::DIR_LEFT)
			{
				MazePt next(curr.x - 1, curr.y - 1);
				if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
				{
					VISIBLEDFS(next, dir);
				}
			}
		}

		//right down
		if (curr.x < m_noGrid - 1 && curr.y > 0)
		{
			if (dir == Maze::DIR_RIGHTDOWN || dir == Maze::DIR_RIGHT)
			{
				MazePt next(curr.x + 1, curr.y - 1);
				if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
				{
					VISIBLEDFS(next, dir);
				}
			}
		}
	}
	else if (curr.x % 2 == 1)
	{
		//left up
		if (curr.x > 0 && curr.y < m_noGrid - 1)
		{
			if (dir == Maze::DIR_LEFTUP || dir == Maze::DIR_LEFT)
			{
				MazePt next(curr.x - 1, curr.y + 1);

				if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
				{
					VISIBLEDFS(next, dir);
				}
			}
		}

		//right up
		if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
		{
			if (dir == Maze::DIR_RIGHTUP || dir == Maze::DIR_RIGHT)
			{
				MazePt next(curr.x + 1, curr.y + 1);

				if (!m_render[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] > 0)
				{
					VISIBLEDFS(next, dir);
				}
			}
		}
	}
}

// L-O-S
// Reference
// https://www.youtube.com/watch?v=fc3nnG2CG8U&ab_channel=javidx9
void SceneHex::CALCVISIBILITY(MazePt start, float range)
{
	origin.SetZero();
	origin.x = start.x;
	origin.y = start.y;
	origin.x = (m_gridOffset + m_gridSize * start.x) * 0.9;
	origin.y = m_gridOffset + m_gridSize * start.y;
	if (start.x % 2 == 0)
		origin.y -= m_gridOffset;

	for (auto edge : m_myEdges)
	{
		for (int i = 0; i < 2; i++)
		{

			Vector3 ray; // Ray
			ray.SetZero();
			switch (i)
			{
			case 0: //End
				ray.x = edge.eX - origin.x;
				ray.y = edge.eY - origin.y;
				break;
			case 1: //Start
				ray.x = edge.sX - origin.x;
				ray.y = edge.sY - origin.y;
				break;
			}

			ray.Normalize();
			float Rayangle = atan2f(ray.y, ray.x);
			ray.x = range * cos(Rayangle);
			ray.y = range * sin(Rayangle);

			float closestDistance = INFINITY;
			Vector3 closestIntersect;
			closestIntersect.SetZero();
			float angle = 0.f;
			bool intersect = false;
			for (auto edge2 : m_myEdges)
			{
				Vector3 wall = Vector3(edge2.eX - edge2.sX, edge2.eY - edge2.sY, 0);

				if (fabs(wall.x - ray.x) > 0.0f && fabs(wall.y - ray.y)) //Check if they dont overlap
				{
					float t = (Vector3(edge2.sX, edge2.sY, 0) - origin).Cross(wall).z / ray.Cross(wall).z;
					float u = (Vector3(edge2.sX, edge2.sY, 0) - origin).Cross(ray).z / ray.Cross(wall).z;
					if (ray.x * ray.y != 0 && t >= 0 && t <= 1 && u >= 0 && u <= 1)
					{
						//Intersect Point
						float intersectX = origin.x + (t * ray.x);
						float intersectY = origin.y + (t * ray.y);
						float distance = Vector3(intersectX - origin.x, intersectY - origin.y, 0).LengthSquared();

						if (fabs(distance) < closestDistance)
						{
							closestDistance = distance;
							closestIntersect.x = intersectX;
							closestIntersect.y = intersectY;
							float angle = atan2f(closestIntersect.y - origin.y, closestIntersect.x - origin.x);
							intersect = true;
						}
					}
				}
			}
			if (intersect)
				LOStriangles.push_back(std::make_tuple(angle, closestIntersect.x, closestIntersect.y));

		}
	}

	std::sort(
		LOStriangles.begin(),
		LOStriangles.end(),
		[&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2)
	{
		return std::get<0>(t1) < std::get<0>(t2);
	});

	std::fill(m_visited.begin(), m_visited.end(), false);
	std::fill(m_render.begin(), m_render.end(), false);
	CALCVISIBLEHEX();

}

// Render Tiles using L-O-S
// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
void SceneHex::CALCVISIBLEHEX()
{
	if (LOStriangles.size() == 0)
		return;
	LOStriangles.push_back(LOStriangles.front());
	for (int j = 0; j < m_myMazePt.size(); j++)
	{
		int count = 0;
		for (auto mazePt : m_myMazePt[j])
		{
			for (int i = 0; i < LOStriangles.size() - 1; i++)
			{
				double s1 = std::get<2>(LOStriangles[i]) - origin.y;
				double s2 = std::get<1>(LOStriangles[i]) - origin.x;
				double s3 = std::get<2>(LOStriangles[i + 1]) - origin.y;
				double s4 = mazePt.second - origin.y;

				double w1 = (origin.x * s1 + s4 * s2 - mazePt.first * s1) / (s3 * s2 - (std::get<1>(LOStriangles[i + 1]) - origin.x) * s1);
				double w2 = (s4 - w1 * s3) / s1;
				if (w1 >= 0 && w2 >= 0 && (w1 + w2) <= 1)
				{
					++count;
				}
			}
		}
		if (count >= 4)
		{
			m_render[j] = true;
			if (m_myGrid[j] >= 0)
			{
				MazePt curr(m_myGridIndex[j].first, m_myGridIndex[j].second);
				//UP
				if (curr.y < m_noGrid - 1)
				{
					MazePt next(curr.x, curr.y + 1);
					if (m_myGrid[next.y * m_noGrid + next.x] < 0)
					{
						m_render[next.y * m_noGrid + next.x] = true;
					}
				}
				//DOWN
				if (curr.y > 0)
				{
					MazePt next(curr.x, curr.y - 1);
					if (m_myGrid[next.y * m_noGrid + next.x] < 0)
					{
						m_render[next.y * m_noGrid + next.x] = true;
					}
				}
				//LEFT
				if (curr.x > 0)
				{
					MazePt next(curr.x - 1, curr.y);
					if (m_myGrid[next.y * m_noGrid + next.x] < 0)
					{
						m_render[next.y * m_noGrid + next.x] = true;
					}
				}
				//RIGHT
				if (curr.x < m_noGrid - 1)
				{
					MazePt next(curr.x + 1, curr.y);
					if (m_myGrid[next.y * m_noGrid + next.x] < 0)
					{
						m_render[next.y * m_noGrid + next.x] = true;
					}
				}

				if (curr.x % 2 == 0)
				{
					//left down
					if (curr.x > 0 && curr.y > 0) //leftdown
					{
						MazePt next(curr.x - 1, curr.y - 1);
						if (m_myGrid[next.y * m_noGrid + next.x] < 0)
						{
							m_render[next.y * m_noGrid + next.x] = true;
						}
					}

					//right down
					if (curr.x < m_noGrid - 1 && curr.y > 0)
					{
						MazePt next(curr.x + 1, curr.y - 1);
						if (m_myGrid[next.y * m_noGrid + next.x] < 0)
						{
							m_render[next.y * m_noGrid + next.x] = true;
						}
					}
				}
				else if (curr.x % 2 == 1)
				{
					//left up
					if (curr.x > 0 && curr.y < m_noGrid - 1)
					{
						MazePt next(curr.x - 1, curr.y + 1);

						if (m_myGrid[next.y * m_noGrid + next.x] < 0)
						{
							m_render[next.y * m_noGrid + next.x] = true;
						}
					}

					//right up
					if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
					{
						MazePt next(curr.x + 1, curr.y + 1);

						if (m_myGrid[next.y * m_noGrid + next.x] < 0)
						{
							m_render[next.y * m_noGrid + next.x] = true;
						}
					}
				}
			}
		}
	}

	for (auto player : m_playerList)
	{
		m_render[player->pos.y * m_noGrid + player->pos.x] = true;
	}

}

void SceneHex::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	static bool bOneState = false;
	if (!bOneState && Application::IsKeyPressed('1'))
	{
		bOneState = true;
		debugRay = !debugRay;
	}
	else if (bOneState && !Application::IsKeyPressed('1'))
	{
		bOneState = false;
	}
	static bool bTwoState = false;
	if (!bTwoState && Application::IsKeyPressed('2'))
	{
		bTwoState = true;
		for (auto player : m_playerList)
		{
			player->controlled = false;
		}
		playerControlled = 0;
	}
	else if (bTwoState && !Application::IsKeyPressed('2'))
	{
		bTwoState = false;
	}
	static bool bThreeState = false;
	if (!bThreeState && Application::IsKeyPressed('3'))
	{
		bThreeState = true;
		for (auto player : m_playerList)
		{
			if (player->type == GameObjectV2::GO_ENGINEER)
				player->controlled = true;
			else
				player->controlled = false;
		}
		playerControlled = 2;
	}
	else if (bThreeState && !Application::IsKeyPressed('3'))
	{
		bThreeState = false;
	}
	static bool bFourState = false;
	if (!bFourState && Application::IsKeyPressed('4'))
	{
		bFourState = true;
		for (auto player : m_playerList)
		{
			if (player->type == GameObjectV2::GO_DRILLER)
				player->controlled = true;
			else
				player->controlled = false;
		}
		playerControlled = 1;
	}
	else if (bFourState && !Application::IsKeyPressed('4'))
	{
		bFourState = false;
	}
	if (Application::IsKeyPressed('N'))
	{
	}
	if (Application::IsKeyPressed('M'))
	{
	}
	if (Application::IsKeyPressed(VK_RETURN))
	{
	}
	if (Application::IsKeyPressed('W'))
	{
		camera.position.y += dt * 40;
		camera.target.y += dt * 40;
	}
	if (Application::IsKeyPressed('A'))
	{
		camera.position.x -= dt * 40;
		camera.target.x -= dt * 40;
	}
	if (Application::IsKeyPressed('S'))
	{
		camera.position.y -= dt * 40;
		camera.target.y -= dt * 40;
	}
	if (Application::IsKeyPressed('D'))
	{
		camera.position.x += dt * 40;
		camera.target.x += dt * 40;
	}
	if (Application::m_ismousescrollymoved)
	{
		m_cameraZoom -= Application::m_mousescrolly * dt * 5;
		if (m_cameraZoom > 2.f)
		{
			m_cameraZoom = 2.f;
		}
		if (m_cameraZoom < 0.5f)
		{
			m_cameraZoom = 0.5f;
		}
	}
	if (Application::IsKeyPressed('R'))
	{
		//Exercise: Implement Reset button
		camera.position = Vector3(0, 0, 1);
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		if (playerControlled != 0)
		{
			double x, y;
			Application::GetCursorPos(&x, &y);
			int w = Application::GetWindowWidth();
			int h = Application::GetWindowHeight();
			float posX = (static_cast<float>(x)) / w * m_worldWidth * m_cameraZoom;
			float posY = ((h - static_cast<float>(y))) / h * m_worldHeight * m_cameraZoom;
			if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
			{
				int x = (int)(posX / (m_gridSize * 0.9));
				if (x % 2 == 0)
					posY += m_gridOffset;
				MazePt target((int)(posX / (m_gridSize * 0.9)), (int)(posY / m_gridSize));
				if (target.x != 0 && target.x != m_noGrid - 1 && target.y != 0 && target.y != m_noGrid - 1)
				{
					for (auto player : m_playerList)
					{
						if (player->type == playerControlled)
						{
							if (!player->AStar(MazePt(player->pos.x, player->pos.y), target))
							{
								if (playerControlled == 1)
								{
									player->NoWallAStar(MazePt(player->pos.x, player->pos.y), target);
								}
							}
							/*std::cout << "Path:";
							for (auto tile : m_shortestPath)
							{
								std::cout << "(" << tile.x << "," << tile.y << ")";
							}
							std::cout << std::endl;*/
						}
					}
				}
			}
		}
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	/*static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
		{
			int x = (int)(posX / (m_gridSize * 0.9));
			if (x % 2 == 0)
				posY += m_gridOffset;
			MazePt mud((int)(posX / (m_gridSize * 0.9)), (int)(posY / m_gridSize));
			m_myGrid[mud.y * m_noGrid + mud.x] = Maze::TILE_MUD;
		}
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}*/
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
		if (m_currTurn == PLAYER)
		{
			m_turnOver = true;
		}
	}

	// A1
	// WAVE ~~~~~~~~~~~~~~~~~~~~
	if (m_waveTimer <= 0 && wave == false)
	{
		wave = true;
		int grunts = Math::RandIntMinMax(8 * (m_playerList.size() * 0.5), 15 * (m_playerList.size() * 0.5));
		int detonator = Math::RandIntMinMax(1 * (m_playerList.size() * 0.5), 2 * (m_playerList.size() * 0.5));
		for (int i = 0; i < 5; ++i)
		{
			GameObjectV2* enemy = FetchGO(GameObjectV2::GO_GRUNT);
			int x = Math::RandIntMinMax(0, m_noGrid - 1);
			int y = Math::RandIntMinMax(0, m_noGrid - 1);
			while (m_myGrid[y * m_noGrid + x] < 0)
			{
				x = Math::RandIntMinMax(0, m_noGrid - 1);
				y = Math::RandIntMinMax(0, m_noGrid - 1);
			}
			enemy->pos.Set(x, y, 0);
			enemy->step = enemy->pos;
			msg = new MessageV2(enemy, MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", msg);
			msg = new MessageV2(enemy, MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", msg);
		}
		for (int i = 0; i < 1; ++i)
		{
			GameObjectV2* enemy = FetchGO(GameObjectV2::GO_DETONATOR);
			int x = Math::RandIntMinMax(0, m_noGrid - 1);
			int y = Math::RandIntMinMax(0, m_noGrid - 1);
			while (m_myGrid[y * m_noGrid + x] < 0)
			{
				x = Math::RandIntMinMax(0, m_noGrid - 1);
				y = Math::RandIntMinMax(0, m_noGrid - 1);
			}
			enemy->pos.Set(x, y, 0);
			enemy->step = enemy->pos;
			MessageV2* msg = new MessageV2(enemy, MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", msg);

		}
	}
	if (wave)
	{
		enemies = 0;
		// Grunt ~~~~~~~~~~~~~~
		for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
		{
			GameObjectV2* grunt = (GameObjectV2*)*it;
			if (grunt->active)
			{
				++enemies;
			}
		}

		// Detonator ~~~~~~~~~~
		for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
		{
			GameObjectV2* detonator = (GameObjectV2*)*it;
			if (detonator->active)
			{
				++enemies;
			}
		}

		if (enemies == 0)
		{
			// Send
			wave = false;
			m_waveTimer = Math::RandIntMinMax(20, 30);
		}
	}

	// Process Messages
	Read();

	if (m_currTurn == PLAYER)
	{
		// Turret
		for (auto it = m_turretList.begin(); it != m_turretList.end(); ++it)
		{
			GameObjectV2* turret = (GameObjectV2*)*it;
			if (turret->active)
			{
				turret->Update(dt * m_speed);
			}
		}
		
		// Player
		end = true;
		win = false;
		for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
		{
			GameObjectV2* player = (GameObjectV2*)*it;
			if (player->active)
			{
				player->Update(dt * m_speed);
			}
			if (!player->escaped && player->active)
			{
				end = false;
			}
			if (player->escaped)
			{
				win = true;
			}
		}
	}
	else
	{
		bool turnEnd = true;
		// Grunt
		for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
		{
			GameObjectV2* grunt = (GameObjectV2*)*it;
			if (grunt->active)
			{
				grunt->Update(dt * m_speed);
				if (!grunt->m_moved)
				{
					turnEnd = false;
				}
			}
		}
		// Detonator
		for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
		{
			GameObjectV2* detonator = (GameObjectV2*)*it;
			if (detonator->active)
			{
				detonator->Update(dt * m_speed);
				if (!detonator->m_moved)
				{
					turnEnd = false;
				}
			}
		}

		if (turnEnd)
		{
			m_turnOver = true;
		}
	}

	if (GameObjectV2::UpdateVisibility)
	{
		UpdateVisibility();
	}

	if (m_turnJustEnded)
		m_turnJustEnded = !m_turnJustEnded;
	if (m_turnOver)
	{
		++m_turnNo;
		switch (m_currTurn)
		{
		case SceneHex::PLAYER:
			m_currTurn = COMPUTER;
			--m_waveTimer;
			// Grunt
			for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
			{
				GameObjectV2* grunt = (GameObjectV2*)*it;
				if (grunt->active)
				{
					grunt->m_moved = false;
				}
			}
			// Detonator
			for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
			{
				GameObjectV2* detonator = (GameObjectV2*)*it;
				if (detonator->active)
				{
					detonator->m_moved = false;
				}
			}
			break;
		case SceneHex::COMPUTER:
			m_currTurn = PLAYER;
			--m_waveTimer;
			for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
			{
				GameObjectV2* player = (GameObjectV2*)*it;
				if (player->active)
				{
					player->m_moved = false;
				}
			}
			break;
		default:
			break;
		}

		// Move Cost
		for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
		{
			GameObjectV2* player = (GameObjectV2*)*it;
			if (player->active)
			{
				if (!player->m_moved)
				{
					--player->m_moveCost;
				}
			}
		}
		// Grunt
		for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
		{
			GameObjectV2* grunt = (GameObjectV2*)*it;
			if (grunt->active)
			{
				--grunt->m_moveCost;
			}
		}

		// Detonator
		for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
		{
			GameObjectV2* detonator = (GameObjectV2*)*it;
			if (detonator->active)
			{
				--detonator->m_moveCost;
			}
		}
		m_turnOver = false;
		m_turnJustEnded = true;
	}

	if (crystals >= 3)
	{
		m_DropPod->active = true;
		for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
		{
			GameObjectV2* player = (GameObjectV2*)*it;
			if (player->active)
			{
				player->SetNextState("Extract");
			}
		}
	}

	// Reset mouseWheel
	Application::m_ismousescrollymoved = false;
	Application::m_mousescrollx = 0.f;
	Application::m_mousescrolly = 0.f;
}


void SceneHex::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_NONE:
		break;
	}
}

void SceneHex::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth * m_cameraZoom, 0, m_worldHeight * m_cameraZoom, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	// Render tiles 
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			if (m_render[row * m_noGrid + col] || row == 0 || row == m_noGrid -1 || col == 0 || col == m_noGrid - 1)
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_gridOffset + m_gridSize * col) * 0.9, m_gridOffset + m_gridSize * row, 0);
				if (col % 2 == 0)
					modelStack.Translate(0, -m_gridOffset, 0);
				modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
				modelStack.Rotate(90, 1, 0, 0);
				switch (m_myGrid[row * m_noGrid + col])
				{
				case Maze::TILE_WALL:
					RenderMesh(meshList[GEO_GREYHEX], false);
					break;

				case Maze::TILE_FOG:
					RenderMesh(meshList[GEO_BLACKHEX], false);
					break;
				case Maze::TILE_MUD:
					RenderMesh(meshList[GEO_BROWNHEX], false);
					break;
				default:
					RenderMesh(meshList[GEO_WHITEHEX], false);
					break;
				}
				modelStack.PopMatrix();
			}
			else
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_gridOffset + m_gridSize * col) * 0.9, m_gridOffset + m_gridSize * row, 0);
				if (col % 2 == 0)
					modelStack.Translate(0, -m_gridOffset, 0);
				modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
				modelStack.Rotate(90, 1, 0, 0);
				RenderMesh(meshList[GEO_BLACKHEX], false);
				modelStack.PopMatrix();
			}
				/*RenderMesh(meshList[GEO_QUEEN], false);
				modelStack.PopMatrix();
			}*/
		}
	}
	//Render curr point
	/*MazePt curr = m_maze.GetCurr();
	modelStack.PushMatrix();
	modelStack.Translate((m_gridOffset + m_gridSize * curr.x) * 0.9, m_gridOffset + m_gridSize * curr.y, 1);
	if (curr.x % 2 == 0)
		modelStack.Translate(0, -m_gridOffset, 0);
	modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
	RenderMesh(meshList[GEO_QUEEN], false);
	modelStack.PopMatrix();*/
	//Render shortest path
	/*for (auto tile : m_shortestPath)
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * tile.x) * 0.9, m_gridOffset + m_gridSize * tile.y, 1);
		if (tile.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
		RenderMesh(meshList[GEO_QUEEN], false);
		modelStack.PopMatrix();
	}*/

	if (debugRay)
	{
		for (int i = 0; i < m_myEdges.size(); i++)
		{
			RenderLine(Vector3(m_myEdges[i].sX, m_myEdges[i].sY, 0), Vector3(m_myEdges[i].eX, m_myEdges[i].eY, 0));
		}
		for (int i = 0; i < LOStriangles.size(); i++)
		{
			RenderLine(Vector3(origin.x, origin.y, 0), Vector3(std::get<1>(LOStriangles[i]), std::get<2>(LOStriangles[i]), 0));
		}
	}

	// A1
	zOffset = 0.002f;
	for (auto it = m_resourceList.begin(); it != m_resourceList.end(); ++it)
	{
		GameObjectV2* ore = (GameObjectV2*)*it;
		if (ore->active)
		{
			RenderGO(ore, zOffset);
			zOffset += 0.001f;

			if (!ore->target.IsZero())
				RenderLine(ore->pos, ore->target);
		}
	}
	for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
	{
		GameObjectV2* grunt = (GameObjectV2*)*it;
		if (grunt->active)
		{
			RenderGO(grunt, zOffset);
			zOffset += 0.001f;

			if (!grunt->target.IsZero())
				RenderLine(grunt->pos, grunt->target);
		}
	}
	for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
	{
		GameObjectV2* detonator = (GameObjectV2*)*it;
		if (detonator->active)
		{
			RenderGO(detonator, zOffset);
			zOffset += 0.001f;

			if (!detonator->target.IsZero())
				RenderLine(detonator->pos, detonator->target);
		}
	}
	for (auto it = m_turretList.begin(); it != m_turretList.end(); ++it)
	{
		GameObjectV2* turret = (GameObjectV2*)*it;
		if (turret->active)
		{
			RenderGO(turret, zOffset);
			zOffset += 0.001f;

			if (!turret->target.IsZero())
				RenderLine(turret->pos, turret->target);
		}
	}

	for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
	{
		GameObjectV2* player = (GameObjectV2*)*it;
		if (player->active)
		{
			RenderGO(player, zOffset);
			if (!player->target.IsZero())
				RenderLine(player->pos, player->target);
			zOffset += 0.001f;
		}
	}

	if (m_DropPod->active)
	{
		RenderGO(m_DropPod, zOffset);
	}

	//On screen text
	std::ostringstream ss;
	ss.precision(3);

	switch (m_currTurn)
	{
	case SceneHex::PLAYER:
		ss << "Turn: PLAYER";
		break;
	case SceneHex::COMPUTER:
		ss << "Turn: COMPUTER";
		break;
	}
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);

	ss.str("");
	ss << "Turn No.:" << m_turnNo;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Turn Maze " << m_mazeKey;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);

	// A1
	
	ss.str("");
	ss.precision(3);
	ss << "Enemies Left:" << enemies;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(),  Color(0, 1, 0), 3, 50, 15);

	int playernum = 0;
	int y = 15;
	for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
	{
		GameObjectV2* player = (GameObjectV2*)*it;
		++playernum;
		y += 5;
		std::string playerClass;
		switch (player->type)
		{
		case GameObjectV2::GO_DRILLER:
			playerClass = "Driller";
			break;
		case GameObjectV2::GO_ENGINEER:
			playerClass = "Engineer";
			break;
		default:
			break;
		}

		ss.str("");
		ss.precision(3);
		ss << "Player " << playernum << ": (" << playerClass
			<< ") Pos:" << player->pos;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(),  Color(0, 1, 0), 3, 50, y - 2);

		ss.str("");
		ss.precision(3);
		ss << "Player " << playernum << ": (" << playerClass
			<< ") Health:" << player->currHealth;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(),  Color(0, 1, 0), 3, 50, y);

	}

	if (end)
	{
		switch (win)
		{
		case true:
			ss.str("");
			ss << "WIN";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(),  Color(0, 1, 0), 10, 40, 40);
			break;
		case false:
			ss.str("");
			ss << "LOSE";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(),  Color(0, 1, 0), 10, 40, 40);
			break;
		default:
			break;
		}
	}
}

void SceneHex::RenderGO(GameObjectV2* go, float zOffset)
{
	std::ostringstream ss;
	switch (go->type)
	{
	case GameObjectV2::GO_DRILLER:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DRILLER], false);

		ss << "Driller";
		modelStack.Translate(-0.2, -0.6, 0);
		modelStack.Scale(0.5, 0.5, 0.5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));

		ss << go->m_currState->GetStateID();
		modelStack.Translate(0, -0.4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_ENGINEER:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ENGINEER], false);

		ss << "Engineer";
		modelStack.Translate(-0.5, -0.6, 0);
		modelStack.Scale(0.5, 0.5, 0.5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));

		ss << go->m_currState->GetStateID();
		modelStack.Translate(0, -0.4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_GRUNT:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_GRUNT], false);

		ss << "Grunt";
		modelStack.Translate(-0.1, -0.6, 0);
		modelStack.Scale(0.5, 0.5, 0.5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));

		ss << go->m_currState->GetStateID();
		modelStack.Translate(0, -0.4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_DETONATOR:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DETONATOR], false);

		ss << "Detonator";
		modelStack.Translate(-0.6, -0.6, 0);
		modelStack.Scale(0.5, 0.5, 0.5);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));

		ss << go->m_currState->GetStateID();
		modelStack.Translate(0, -0.4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_ORE:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ORE], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_TURRET:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(1, 1, 1);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	}
	case GameObjectV2::GO_DROPPOD:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_gridOffset + m_gridSize * go->pos.x) * 0.9, m_gridOffset + m_gridSize * go->pos.y, zOffset);
		if ((int)go->pos.x % 2 == 0)
			modelStack.Translate(0, -m_gridOffset, 0);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DROPPOD], false);
		modelStack.PopMatrix();
		break;
	}
	}

}

void SceneHex::RenderLine(Vector3 origin, Vector3 destination)
{
	Vector3 dist = destination - origin;

	float l = dist.Length();
	modelStack.PushMatrix();
	modelStack.Translate(origin.x, origin.y, 2);
	modelStack.Rotate(Math::RadianToDegree(atan2f(dist.y, dist.x)), 0, 0, 1);
	modelStack.Scale(l, l, l);
	RenderMesh(meshList[GEO_LINE], false);
	modelStack.PopMatrix();
}

void SceneHex::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}

bool SceneHex::Handle(MessageV2* Message)
{
	MessageV2* messageWRU = dynamic_cast<MessageV2*>(Message);
	if (messageWRU)
	{
		m_inbox.push(Message);
		return true;
	}
	return false;
}

void SceneHex::Read()
{
	while (m_inbox.size() > 0)
	{
		MessageV2* message = m_inbox.front();
		MessageV2* reply;
		// Tracking
		float distance = FLT_MAX;
		GameObjectV2* closest = NULL;

		switch (message->type)
		{
		case MessageV2::MSG_NEARESTPLAYER:
		{
			distance = FLT_MAX;
			for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
			{
				GameObjectV2* player = (GameObjectV2*)*it;
				if (player->active)
				{
					if ((player->pos - message->go->pos).Length() < distance)
					{
						distance = (player->pos - message->go->pos).Length();
						closest = player;
					}
				}
			}
			// Send
			reply = new MessageV2(closest, MessageV2::MSG_NEARESTPLAYER, distance, Vector3(0, 0, 0), false);
			m_PostOffice->Send(message->go->ID, reply);
			break;
		}
		case MessageV2::MSG_NEARDETONATOR:
		{
			bool isNear = false;
			for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
			{
				GameObjectV2* detonator = (GameObjectV2*)*it;
				if (detonator->active)
				{
					if ((detonator->pos - message->go->pos).Length() < 5)
					{
						// Send
						isNear = true;
						break;
					}
				}
			}
			reply = new MessageV2(NULL, MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), isNear);
			m_PostOffice->Send(message->go->ID, reply);
			break;
		}
		case MessageV2::MSG_NEARESTENEMY:
		{
			// Grunt ~~~~~~~~~~~~~~
			distance = FLT_MAX;
			for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
			{
				GameObjectV2* grunt = (GameObjectV2*)*it;
				if (grunt->active)
				{
					if ((grunt->pos - message->go->pos).Length() < distance)
					{
						distance = (grunt->pos - message->go->pos).Length();
						closest = grunt;
					}
				}
			}

			// Detonator ~~~~~~~~~~
			for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
			{
				GameObjectV2* detonator = (GameObjectV2*)*it;
				if (detonator->active)
				{
					if ((detonator->pos - message->go->pos).Length() < distance)
					{
						distance = (detonator->pos - message->go->pos).Length();
						closest = detonator;
					}
				}
			}
			// Send
			reply = new MessageV2(closest, MessageV2::MSG_NEARESTENEMY, distance, Vector3(0, 0, 0), false);
			m_PostOffice->Send(message->go->ID, reply);
			break;
		}
		case MessageV2::MSG_DRILLERLOCATION:
		{
			for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
			{
				GameObjectV2* player = (GameObjectV2*)*it;
				if (player->active && player->type == GameObjectV2::GO_DRILLER)
				{
					if ((player->pos - message->go->pos).Length() < distance)
					{
						distance = (player->pos - message->go->pos).Length();
						closest = player;
					}
				}
			}
			// Send
			if (!closest)
			{
				reply = new MessageV2(NULL, MessageV2::MSG_DRILLERLOCATION, 0.f, Vector3(0, 0, 0), false);
				m_PostOffice->Send(message->go->ID, reply);
			}
			else
			{
				reply = new MessageV2(NULL, MessageV2::MSG_DRILLERLOCATION, 0.f, closest->pos, false);
				m_PostOffice->Send(message->go->ID, reply);
			}
			break;
		}
		case MessageV2::MSG_ENGINEERLOCATION:
		{
			for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
			{
				GameObjectV2* player = (GameObjectV2*)*it;
				if (player->active && player->type == GameObjectV2::GO_ENGINEER)
				{
					if ((player->pos - message->go->pos).Length() < distance)
					{
						distance = (player->pos - message->go->pos).Length();
						closest = player;
					}
				}
			}
			// Send
			if (!closest)
			{
				reply = new MessageV2(NULL, MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
				m_PostOffice->Send(message->go->ID, reply);
			}
			else
			{
				reply = new MessageV2(NULL, MessageV2::MSG_ENGINEERLOCATION, 0.f, closest->pos, false);
				m_PostOffice->Send(message->go->ID, reply);
			}
			break;
		}
		case MessageV2::MSG_NEARESTORE:
		{
			distance = FLT_MAX;
			int count = 0;
			for (auto it = m_resourceList.begin(); it != m_resourceList.end(); ++it)
			{
				GameObjectV2* ore = (GameObjectV2*)*it;
				if (ore->active)
				{
					++count;
					// Engineer ~~~~~~~~~~~~~~
					if ((ore->pos - message->go->pos).Length() < distance)
					{
						distance = (ore->pos - message->go->pos).Length();
						closest = ore;
					}
				}
			}
			// Send
			reply = new MessageV2(closest, MessageV2::MSG_NEARESTORE, distance, Vector3(0, 0, 0), false);
			m_PostOffice->Send(message->go->ID, reply);
			reply = new MessageV2(NULL, MessageV2::MSG_CRYSTALMINED, count, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", reply);
			break;
		}
		case MessageV2::MSG_TURRET:
		{
			if (message->isTrue)
			{
				for (int i = 0; i < 3; ++i)
				{
					GameObjectV2* turret = FetchGO(GameObjectV2::GO_TURRET);
					turret->active = true;
					turret->Engineer = message->go;
					turret->pos = message->go->pos + Vector3(Math::RandFloatMinMax(-1, 1), Math::RandFloatMinMax(-1, 1), 0);
					MessageV2* msg = new MessageV2(turret, MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
					m_PostOffice->Send("SceneMining", msg);
				}
			}
			else
			{
				// Turret
				for (auto it = m_turretList.begin(); it != m_turretList.end(); ++it)
				{
					GameObjectV2* turret = (GameObjectV2*)*it;
					if (turret->active && turret->Engineer == message->go)
					{
						turret->active = false;
					}
				}
			}
			break;
		}
		case MessageV2::MSG_WAVE:
		{
			// Send
			reply = new MessageV2(message->go, MessageV2::MSG_WAVE, distance, Vector3(0, 0, 0), wave);
			m_PostOffice->Send(message->go->ID, reply);
			break;
		}
		case MessageV2::MSG_EXPLOSION:
		{
			for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
			{
				GameObjectV2* grunt = (GameObjectV2*)*it;
				if (grunt->active)
				{
					if ((grunt->pos - message->pos).Length() <= 2)
					{
						grunt->SetNextState("Death");
					}
				}
			}
			for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
			{
				GameObjectV2* player = (GameObjectV2*)*it;
				if (player->active && player->type == GameObjectV2::GO_ENGINEER)
				{
					if ((player->pos - message->pos).Length() <= 1)
					{
						player->SetNextState("Death");
					}
					else if ((player->pos - message->pos).Length() <= 2)
					{
						MessageV2* message = new MessageV2(NULL, MessageV2::MSG_DMG, 80.f, Vector3(0, 0, 0), false);
						m_PostOffice->Send(player->ID, message);
					}
				}
			}
			break;
		}
		case MessageV2::MSG_DROPPOD:
		{
			// Send
			reply = new MessageV2(NULL, MessageV2::MSG_DROPPOD, 0.f, m_DropPod->pos, false);
			m_PostOffice->Send(message->go->ID, reply);
			break;
		}
		case MessageV2::MSG_CRYSTALMINED:
		{
			crystals = 3 - message->threshold;
			break;
		}
		}
		delete message;
		m_inbox.pop();
	}
}
