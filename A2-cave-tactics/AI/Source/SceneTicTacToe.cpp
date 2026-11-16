#include "SceneTicTacToe.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneTicTacToe::SceneTicTacToe()
{
}

SceneTicTacToe::~SceneTicTacToe()
{
}

void SceneTicTacToe::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 3;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_bCrossTurn = true;
	m_bGameOver = false;
	m_winner = WHO_CONTENT::WHO_NONE;

	for (int i = 0; i < 9; ++i)
	{
		m_grid[i] = WHO_NONE;
	}
}

GameObject* SceneTicTacToe::FetchGO()
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

//Exercise: This function should return 0 to 8, i.e. AI player's decision to put circle on one of the grids
int SceneTicTacToe::GetAIDecision()
{
	/*if (m_grid[0] == WHO_CIRCLE && m_grid[1] == WHO_CIRCLE)
	{
		return 2;
	}*/

	//randomize for now cos im lazy to read on minmax
	int randGrid;
	randGrid = Math::RandIntMinMax(0, 8);
	return randGrid;
}

void SceneTicTacToe::Update(double dt)
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
		for (int i = 0; i < 9; ++i)
		{
			if (m_grid[i] != WHO_NONE)
			{
				m_grid[i] = WHO_NONE;
			}
		}

		m_bGameOver = false;
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
		//Exercise: Game inputs
		
		int gridX = posX / m_gridSize;
		int gridY = posY / m_gridSize;

		if (gridX < 3 && gridY < 3)
		{
			int index = gridX + 3 * gridY;
			
			if (m_grid[index] == WHO_NONE)
			{
				if (m_bCrossTurn)
				{
					m_grid[index] = WHO_CROSS;
					m_bCrossTurn = false;
				}

				/*else
				{
					m_grid[index] = WHO_CIRCLE;
					m_bCrossTurn = true;
				}*/
			}
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

	if (!m_bCrossTurn)
	{
		int circleDecision = GetAIDecision();
		//Exercise: Call GetAIDecision() and put circle on grid
		if (m_grid[circleDecision] == WHO_NONE)
		{
			m_grid[circleDecision] = WHO_CIRCLE;
			m_bCrossTurn = true;
		}
	}

	//Game Logic Section
	//Exercise: Check draw, cross wins or circle wins

	/*if (m_grid[0] == WHO_CROSS && m_grid[1] == WHO_CROSS && m_grid[2] == WHO_CROSS)
	{
		m_winner = WHO_CROSS;
		m_bGameOver = true;
	}

	else if (m_grid[0] == WHO_CROSS && m_grid[4] == WHO_CROSS && m_grid[8] == WHO_CROSS)
	{
		m_winner = WHO_CROSS;
		m_bGameOver = true;
	}

	else if (m_grid[0] == WHO_CROSS && m_grid[3] == WHO_CROSS && m_grid[6] == WHO_CROSS)
	{
		m_winner = WHO_CROSS;
		m_bGameOver = true;
	}

	else if (m_grid[3] == WHO_CROSS && m_grid[5] == WHO_CROSS && m_grid[8] == WHO_CROSS)
	{
		m_winner = WHO_CROSS;
		m_bGameOver = true;
	}

	else if (m_grid[3] == WHO_CROSS && m_grid[4] == WHO_CROSS && m_grid[5] == WHO_CROSS)
	{
		m_winner = WHO_CROSS;
		m_bGameOver = true;
	}*/

	for (int i = 0; i < 9; ++i)
	{
		
			if ((m_grid[i] == WHO_CROSS && m_grid[i + 1] == WHO_CROSS && m_grid[i + 2] == WHO_CROSS) || 
				(m_grid[i] == WHO_CROSS && m_grid[i + 3] == WHO_CROSS && m_grid[i + 6] == WHO_CROSS) ||
				(m_grid[2] == WHO_CROSS && m_grid[4] == WHO_CROSS && m_grid[6] == WHO_CROSS) ||
				(m_grid[0] == WHO_CROSS && m_grid[4] == WHO_CROSS && m_grid[8] == WHO_CROSS))
			{
				m_winner = WHO_CROSS;
				m_bGameOver = true;
			}

			if (m_grid[i] == WHO_CIRCLE && m_grid[i + 1] == WHO_CIRCLE && m_grid[i + 2] == WHO_CIRCLE || 
				(m_grid[i] == WHO_CIRCLE && m_grid[i + 3] == WHO_CIRCLE && m_grid[i + 6] == WHO_CIRCLE) ||
				(m_grid[2] == WHO_CIRCLE && m_grid[4] == WHO_CIRCLE && m_grid[6] == WHO_CIRCLE) ||
				(m_grid[0] == WHO_CIRCLE && m_grid[4] == WHO_CIRCLE && m_grid[8] == WHO_CIRCLE))
			{
				m_winner = WHO_CIRCLE;
				m_bGameOver = true;
			}
		
	}
}


void SceneTicTacToe::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_CROSS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CROSS], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_CIRCLE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneTicTacToe::Render()
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

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_TICTACTOE], false);
	modelStack.PopMatrix();

	for (int i = 0; i < 9; ++i)
	{
		int gridX = i % 3;
		int gridY = i / 3;

		modelStack.PushMatrix();
		modelStack.Translate(m_gridOffset + gridX * m_gridSize, m_gridOffset + gridY * m_gridSize, 0.0f);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);

		if (m_grid[i] == WHO_CROSS)
			RenderMesh(meshList[GEO_CROSS], false);
		else if(m_grid[i] == WHO_CIRCLE)
			RenderMesh(meshList[GEO_CIRCLE], false);
		modelStack.PopMatrix();
	}

	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (go->active)
		{
			RenderGO(go);
		}
	}

	//On screen text
	static std::string winnerText[] = {"Draw", "Cross wins", "Circle wins"};

	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Count:" << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn:" << (m_bCrossTurn ? "Cross" : "Circle");
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);

	if (m_bGameOver)
	{
		ss.str("");
		ss << winnerText[static_cast<int>(m_winner)];
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 15);
	}

	RenderTextOnScreen(meshList[GEO_TEXT], "Tic Tac Toe (R to reset)", Color(0, 1, 0), 3, 50, 0);
}

void SceneTicTacToe::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if (m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
