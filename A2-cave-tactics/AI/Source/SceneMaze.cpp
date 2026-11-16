#include "SceneMaze.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneMaze::SceneMaze()
{
}

SceneMaze::~SceneMaze()
{
}

void SceneMaze::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();
	
	m_noGrid = 12;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_start.Set(0, 0);
	m_mazeKey = 0;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, 0.3);
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
	DFS(m_start);
}

GameObject* SceneMaze::FetchGO()
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_CROSS);
		m_goList.push_back(go);
	}
	return FetchGO();
}

void SceneMaze::Update(double dt)
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
	if (Application::IsKeyPressed('R'))
	{
		//Exercise: Implement Reset button
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		
		//Exercise: turn mouse click into end point and run BFS
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
		{
			int gridX = posX / m_gridSize;
			int gridY = posY / m_gridSize;

			m_end.Set(gridX, gridY);
			BFS(m_start, m_end);

			std::cout << "Path:";
			for (auto tile : m_shortestPath)
			{
				std::cout << "(" << tile.x << ", " << tile.y << ")";
			}
			std::cout << std::endl;
		}
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
}


void SceneMaze::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_NONE:
		break;
	}
}

void SceneMaze::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
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

	RenderMesh(meshList[GEO_AXES], false);

	//Render tiles
	for (int x = 0; x < m_noGrid; ++x)
	{
		for (int y = 0; y < m_noGrid; ++y)
		{
			modelStack.PushMatrix();
			if (x % 2 == 0)
				modelStack.Translate(m_gridOffset + m_gridSize * y + m_gridOffset, m_gridOffset + m_gridSize * x, 0);
			else
				modelStack.Translate(m_gridOffset + m_gridSize * y, m_gridOffset + m_gridSize * x, 0);
			if (y % 2 == 0)
				modelStack.Translate(0, -m_gridOffset, 0);
			modelStack.Rotate(90, 1, 0, 0);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			switch (m_myGrid[x * m_noGrid + y])
			{
			case Maze::TILE_WALL:
				RenderMesh(meshList[GEO_GREYHEX], false);
				break;

			case Maze::TILE_FOG:
				RenderMesh(meshList[GEO_BLACKHEX], false);
				break;
			default:
				RenderMesh(meshList[GEO_WHITEHEX], false);
				break;
			}
			modelStack.PopMatrix();

		}
	}

	//Render curr point
	modelStack.PushMatrix();
	modelStack.Translate(m_gridOffset + m_gridSize * m_maze.GetCurr().x, m_gridOffset + m_gridSize * m_maze.GetCurr().y, 0);
	modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
	RenderMesh(meshList[GEO_QUEEN], false);
	modelStack.PopMatrix();

	//Render shortest path
	for (auto tile : m_shortestPath)
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + m_gridSize * tile.x, m_gridOffset + m_gridSize * tile.y, 0);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
		RenderMesh(meshList[GEO_QUEEN], false);
		modelStack.PopMatrix();
	}

	//On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Num Move:" << 0;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Maze " << 0;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);
}

void SceneMaze::DFS(MazePt curr)
{
	//m_visited[curr.y * m_noGrid + curr.x] = true;

	////up
	//if (curr.y < m_noGrid - 1)
	//{
	//	MazePt next(curr.x, curr.y + 1);
	//	if (!m_visited[next.y * m_noGrid + next.x])
	//	{
	//		if (m_maze.Move(Maze::DIR_UP) == true)
	//		{
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
	//			DFS(next);
	//			m_maze.Move(Maze::DIR_DOWN);
	//		}

	//		else
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
	//	}

	//}

	////down
	//if (curr.y > 0)
	//{
	//	MazePt next(curr.x, curr.y - 1);
	//	if (!m_visited[next.y * m_noGrid + next.x])
	//	{
	//		if (m_maze.Move(Maze::DIR_DOWN) == true)
	//		{
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
	//			DFS(next);
	//			m_maze.Move(Maze::DIR_UP);
	//		}

	//		else
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
	//	}
	//}

	////right
	//if (curr.x < m_noGrid - 1)
	//{
	//	MazePt next(curr.x + 1, curr.y);
	//	if (!m_visited[next.y * m_noGrid + next.x])
	//	{
	//		if (m_maze.Move(Maze::DIR_RIGHT) == true)
	//		{
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
	//			DFS(next);
	//			m_maze.Move(Maze::DIR_LEFT);
	//		}

	//		else
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
	//	}
	//}

	////left
	//if (curr.x > 0)
	//{
	//	MazePt next(curr.x - 1, curr.y);
	//	if (!m_visited[next.y * m_noGrid + next.x])
	//	{
	//		if (m_maze.Move(Maze::DIR_LEFT) == true)
	//		{
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
	//			DFS(next);
	//			m_maze.Move(Maze::DIR_RIGHT);
	//		}

	//		else
	//			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
	//	}
	//}
}

bool SceneMaze::BFS(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_queue.empty())
	{
		m_queue.pop();
	}
	
	m_shortestPath.clear();
	m_previous.clear();
	m_queue.push(start);
	m_maze.SetNumMove(0);

	MazePt curr;
	while (!m_queue.empty())
	{
		curr = m_queue.front();

		m_maze.SetCurr(curr);
		m_queue.pop();

		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = m_previous[curr.y * m_noGrid + curr.x];
			}

			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}

		//up
		if (curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);

			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.x * m_noGrid + next.x] = true;
			}
		}
	}
	return false;
}

void SceneMaze::Exit()
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
