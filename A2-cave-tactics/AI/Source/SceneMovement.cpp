#include "SceneMovement.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneMovement::SceneMovement()
{
}

SceneMovement::~SceneMovement()
{
}

void SceneMovement::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 20;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;
	m_hourOfTheDay = 0;
	elapsedTime = 0;

	
}

GameObject* SceneMovement::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active && type == go->type)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(type);
		m_goList.push_back(go);
		go->id = m_goList.size();

		if (go->type == GameObject::GO_FISH)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateTooFull("fishTooFull", go));
			go->sm->AddState(new StateFull("fishFull", go));
			go->sm->AddState(new StateHungry("fishHungry", go));
			go->sm->AddState(new StateDead("fishDead", go));
		}

		if (go->type == GameObject::GO_SHARK)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateTooFull("sharkHappy", go));
			go->sm->AddState(new StateFull("sharkCrazy", go));
			go->sm->AddState(new StateHungry("sharkNaughty", go));
		}
	}
	return FetchGO(type);
}

void SceneMovement::Update(double dt)
{
	SceneBase::Update(dt);

	static const float BALL_SPEED = 15.f;
	static const float HOUR_SPEED = 1.f;

	elapsedTime += static_cast<float>(dt) * 1;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	
	if(Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}

	m_hourOfTheDay += HOUR_SPEED * static_cast<float>(dt) * m_speed;
	if (m_hourOfTheDay >= 24.f)
		m_hourOfTheDay = 0;

	//Input Section
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState &&  Application::IsKeyPressed(VK_SPACE))
	{					  
		int randX = Math::RandIntMinMax(0, 19);
		int randY =	Math::RandIntMinMax(0, 19);

		bSpaceState = true;
		GameObject *go = FetchGO(GameObject::GO_FISH);
		go->id = m_objectCount;
		//go->currState = GameObject::STATE::STATE_TOOFULL;
		go->sm->SetNextState("fishFull");
		go->active = true;
		go->fishCount += 1;
		go->energy = 10;
		//go->moveSpeed = 5.f;
		go->scale.Set(m_gridSize * 1.f, m_gridSize * 1.f, m_gridSize * 1.f);
		go->pos.Set(m_gridOffset + randX * m_gridSize, m_gridOffset + randY * m_gridSize, 0);
		go->target = go->pos;
		go->countDown = 3.f;
		go->steps = 0;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	static bool rState = false;
	if (!rState && Application::IsKeyPressed('R'))
	{
		rState = true;
		int randX = Math::RandIntMinMax(0, 19);
		int randY = Math::RandIntMinMax(0, 19);

		GameObject* go = FetchGO(GameObject::GO_FISHFOOD);
		//go->id = m_objectCount;
		go->scale.Set(m_gridSize * 1.f, m_gridSize * 1.f, m_gridSize * 1.f);
		go->pos.Set(m_gridOffset + randX * m_gridSize, m_gridOffset + randY * m_gridSize, 0);
		go->target = go->pos;
		go->moveSpeed = 3.f;
	}

	else if (rState && !Application::IsKeyPressed('R'))
	{
		rState = false;
	}

	static bool tState = false;
	if (!tState && Application::IsKeyPressed('T'))
	{
		tState = true;
		int randX = Math::RandIntMinMax(0, 19);
		int randY = Math::RandIntMinMax(0, 19);

		GameObject* go = FetchGO(GameObject::GO_SHARK);
		go->id = m_objectCount;
		go->moveSpeed = 10.f;
		go->active = true;
		go->sm->SetNextState("sharkHappy");
		//go->type = GameObject::GO_SHARK;
		go->scale.Set(m_gridSize * 1.f, m_gridSize * 1.f, m_gridSize * 1.f);
		go->pos.Set(m_gridOffset + randX * m_gridSize, m_gridOffset + randY * m_gridSize, 0);
		go->target = go->pos;
		go->steps = 0;
	}

	else if (tState && !Application::IsKeyPressed('T'))
	{
		tState = false;
	}
	//state machine
	for (GameObject* go : m_goList)
	{
		if (go->active && go->sm != nullptr)
		{
			go->sm->Update(dt * m_speed);
		}
	}

	//External triggers
	static const float SHARKDIST = 10.f * m_gridSize;
	static const float FOODDIST = 20.f * m_gridSize;

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
			continue;

		if (go->type == GameObject::GO_FISH)
		{
			go->nearest = NULL;
			float nearestDist = FLT_MAX;
			std::string fishState = go->sm->GetCurrentState();
			for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject* go2 = (GameObject*)*it2;
				if (!go2->active)
					continue;
				if (go->type == GameObject::GO_SHARK)
				{
					float distance = (go->pos - go2->pos).Length();

					if (distance < m_gridSize)
					{
						go->energy -= 1;
					}
					else if (distance < SHARKDIST && distance < nearestDist && fishState == "fishFull")
					{
						nearestDist = distance;
						go->nearest = go2;
					}
				}
				else if (go2->type == GameObject::GO_FISHFOOD)
				{
					float dist = (go->pos - go2->pos).Length();

					if (dist < m_gridSize)
					{
						go->energy += 2.5;
						go2->active = false;
						--m_objectCount;
					}
					else if (dist < FOODDIST && dist < nearestDist && fishState == "fishHungry")
					{
						nearestDist = dist;
						go->nearest = go2;
					}
				}
			}
		}
	}

	//Movement Section
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;

		if(go->active)
		{
			Vector3 foodPos;
			if (go->type == GameObject::GO_FISHFOOD)
			{
				foodPos = go->pos;
			}

			Vector3 dir = go->target - go->pos;
			if (dir.Length() < BALL_SPEED * dt * m_speed)
			{
				//GO->pos reach target
				go->pos = go->target;
				float random = Math::RandFloatMinMax(0.f, 1.f);
				//Exercise: use probability to decide go up or right
				/*if (rand() % 2)
					go->target.Set(go->pos.x, go->pos.y + 5);
				else
					go->target.Set(go->pos.x + 5, go->pos.y);*/

				++go->steps;
				if (
					go->target.x < m_gridOffset + m_gridSize ||
					go->target.x > -m_gridOffset + (m_noGrid - 1) * m_gridSize||
					go->target.y < m_gridOffset + m_gridSize ||
					go->target.y > -m_gridOffset + (m_noGrid - 1) * m_gridSize 
					)
				{
					go->target.x *= -1;
					continue;
				}

				//Exercise: change the conditions so that the game objects can move randomly
				if (random > 0.f)
					go->target.Set(go->pos.x, go->pos.y + 5);
				if (random > 0.25f)
					go->target.Set(go->pos.x, go->pos.y - 5);
				if (random > 0.5f)
					go->target.Set(go->pos.x + 5, go->pos.y);
				if (random > 0.75f)
					go->target.Set(go->pos.x - 5, go->pos.y);
			}
			else
			{
				dir.Normalize();
				go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
			}
		}
		//go->Update(dt);
	}
}


void SceneMovement::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;

	case GameObject::GO_SHARK:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		
		if (go->sm->GetCurrentState() == "sharkNaughty")
			RenderMesh(meshList[GEO_SHARK], false);
		else if (go->sm->GetCurrentState() == "sharkHappy")
			RenderMesh(meshList[GEO_SHARKHAPPY], false);
		else if (go->sm->GetCurrentState() == "sharkCrazy")
			RenderMesh(meshList[GEO_SHARKCRAZY], false);

		modelStack.PopMatrix();
		break;

	case GameObject::GO_FISHFOOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_FISHFOOD], false);
		modelStack.PopMatrix();
		break;

	case GameObject::GO_FISH:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + (float)go->id * 0.01f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->sm->GetCurrentState() == "fishTooFull")
			RenderMesh(meshList[GEO_TOOFULLFISH], false);
		else if (go->sm->GetCurrentState() == "fishFull")
			RenderMesh(meshList[GEO_FULLFISH], false);
		else if (go->sm->GetCurrentState() == "fishHungry")
			RenderMesh(meshList[GEO_HUNGRYFISH], false);
		else if (go->sm->GetCurrentState() == "fishDead")
			RenderMesh(meshList[GEO_DEADFISH], false);

		modelStack.PopMatrix();
		
		break;
	}
}

void SceneMovement::Render()
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
	RenderMesh(meshList[GEO_BG], false);
	modelStack.PopMatrix();

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
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
	ss << "Count:" << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss.precision(4);
	ss << "Hour:" << m_hourOfTheDay;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Movement", Color(0, 1, 0), 3, 50, 0);
}

void SceneMovement::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
