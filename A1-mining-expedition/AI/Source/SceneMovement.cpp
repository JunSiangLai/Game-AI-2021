#include "SceneMovement.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "StatesFish.h"
#include "StatesShark.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

static SceneData& sceneData = *SceneData::GetInstance();

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

	PostOffice::GetInstance()->Register("Scene", this);

	sceneData.SetObjectCount(0);
	sceneData.SetFishCount(0);

	sceneData.SetGridNo(20);
	sceneData.SetGridSize(m_worldHeight / sceneData.GetGridNo());
	sceneData.SetGridOffset(sceneData.GetGridSize() / 2);
	/*m_noGrid = 20;
	sceneData.GetGridSize() = m_worldHeight / m_noGrid;
	sceneData.GetGridOffset() = sceneData.GetGridSize() / 2;
	m_hourOfTheDay = 0;*/

	GameObject *go = FetchGO(GameObject::GO_SHARK);
	go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
	go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
	go->target = go->pos;
}

GameObject* SceneMovement::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active && go->type == type)
		{
			go->active = true;
			return go;
		}
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		GameObject *go = new GameObject(type);
		m_goList.push_back(go);
		if (type == GameObject::GO_FISH)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateTooFull("TooFull", go));
			go->sm->AddState(new StateFull("Full", go));
			go->sm->AddState(new StateHungry("Hungry", go));
			go->sm->AddState(new StateDead("Dead", go));
		}
		else if (type == GameObject::GO_SHARK)
		{
			go->sm = new StateMachine();
			go->sm->AddState(new StateCrazy("Crazy", go));
			go->sm->AddState(new StateNaughty("Naughty", go));
			go->sm->AddState(new StateHappy("Happy", go));
		}
	}
	return FetchGO(type);
}

void SceneMovement::Update(double dt)
{
	SceneBase::Update(dt);

	while (!m_messageQueue.empty())
	{
		Process(m_messageQueue.front());
		m_messageQueue.pop();
	}
	
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
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
		GameObject *go = FetchGO(GameObject::GO_FISH);
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
		go->target = go->pos;
		go->steps = 0;
		go->energy = 8.f;
		go->nearest = NULL;
		go->sm->SetNextState("Full");
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
	static bool bVState = false;
	if (!bVState && Application::IsKeyPressed('V'))
	{
		bVState = true;
		GameObject *go = FetchGO(GameObject::GO_FISHFOOD);
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
		go->target = go->pos;
		go->moveSpeed = 1.f;
	}
	else if (bVState && !Application::IsKeyPressed('V'))
	{
		bVState = false;
	}
	static bool bBState = false;
	if (!bBState && Application::IsKeyPressed('B'))
	{
		bBState = true;
		GameObject *go = FetchGO(GameObject::GO_SHARK);
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
		go->target = go->pos;
	}
	else if (bBState && !Application::IsKeyPressed('B'))
	{
		bBState = false;
	}

	//StateMachine
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		if (go->sm)
			go->sm->Update(dt);
	}

	//External triggers
	static const float SHARK_DIST = 5.f * sceneData.GetGridSize();
	static const float FOOD_DIST = 20.f * sceneData.GetGridSize();
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		if (go->type == GameObject::GO_FISH)
		{
			//go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			for (std::vector<GameObject *>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject *go2 = (GameObject *)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_SHARK)
				{
					float distance = (go->pos - go2->pos).Length();
					if (distance < sceneData.GetGridSize())
					{
						go->energy = -1;
					}

					else if (distance < SHARK_DIST && distance < nearestDistance && go->sm->GetCurrentState() == "Full")
					{
						nearestDistance = distance;
						go->nearest = go2;
					}
				}
				else if (go2->type == GameObject::GO_FISHFOOD)
				{
					float distance = (go->pos - go2->pos).Length();

					if (distance < SceneData::GetInstance()->GetGridSize())
					{
						go->energy += 2.5f;
						go2->active = false;
					}

					if (distance < FOOD_DIST && distance < nearestDistance && go->sm->GetCurrentState() == "Hungry")
					{
						nearestDistance = distance;
						go->nearest = go2;
					}
				}
			}
		}
		else if (go->type == GameObject::GO_SHARK)
		{
			go->nearest = NULL;
			float nearestDistance = FLT_MAX;
			float highestEnergy = FLT_MIN;
			for (std::vector<GameObject *>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
			{
				GameObject *go2 = (GameObject *)*it2;
				if (!go2->active)
					continue;
				if (go2->type == GameObject::GO_FISH)
				{
					if (go->sm->GetCurrentState() == "Crazy")
					{
						if (go2->energy > highestEnergy)
						{
							highestEnergy = go2->energy;
							go->nearest = go2;
						}
					}

					if (go->sm->GetCurrentState() == "Naughty")
					{
						float distance = (go->pos - go2->pos).Length();
						if (distance < nearestDistance && (go2->sm->GetCurrentState() == "TooFull" || go2->sm->GetCurrentState() == "Full"))
						{
							nearestDistance = distance;
							go->nearest = go2;
						}
					}
				}
			}
		}
	}

	//Movement Section
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
			continue;
		Vector3 dir = go->target - go->pos;
		if (dir.Length() < go->moveSpeed * dt * m_speed)
		{
			//GO->pos reach target
			go->pos = go->target;
			float random = Math::RandFloatMinMax(0.f, 1.f);
			if (random < 0.25f && go->moveRight)
				go->target = go->pos + Vector3(sceneData.GetGridSize(), 0, 0);
			else if (random < 0.5f && go->moveLeft)
				go->target = go->pos + Vector3(-sceneData.GetGridSize(), 0, 0);
			else if (random < 0.75f && go->moveUp)
				go->target = go->pos + Vector3(0, sceneData.GetGridSize(), 0);
			else if(go->moveDown)
				go->target = go->pos + Vector3(0, -sceneData.GetGridSize(), 0);
			if (go->target.x < 0 || go->target.x > sceneData.GetGridNo() * sceneData.GetGridSize() || go->target.y < 0 || go->target.y > sceneData.GetGridNo() * sceneData.GetGridSize())
				go->target = go->pos;
		}
		else
		{
			try
			{
				dir.Normalize();
				go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
			}
			catch (DivideByZero)
			{
			}
		}
	}

	//Counting objects
	int objectCount = 0;
	int fishCount = 0;
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (go->Handle(new MessageCheckActive()))
		{
			++objectCount;

			if (go->type == GameObject::GO_FISH)
			{
				++fishCount;
			}
		}
	}
	SceneData::GetInstance()->SetObjectCount(objectCount);
	SceneData::GetInstance()->SetFishCount(fishCount);
}


void SceneMovement::RenderGO(GameObject *go)
{
	std::ostringstream ss;
	switch(go->type)
	{
	case GameObject::GO_BALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);

		ss << go->id;
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISH:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->sm)
		{
			if (go->sm->GetCurrentState() == "TooFull")
				RenderMesh(meshList[GEO_TOOFULL], false);
			else if (go->sm->GetCurrentState() == "Full")
				RenderMesh(meshList[GEO_FULL], false);
			else if (go->sm->GetCurrentState() == "Hungry")
				RenderMesh(meshList[GEO_HUNGRY], false);
			else
				RenderMesh(meshList[GEO_DEAD], false);
		}

		ss.precision(3);
		ss << go->energy;
		modelStack.Scale(0.5f, 0.5f, 0.5f);
		modelStack.Translate(0, -sceneData.GetGridSize() / 4, 0);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHARK:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->sm)
		{
			if (go->sm->GetCurrentState() == "Crazy")
				RenderMesh(meshList[GEO_CRAZY], false);
			else if (go->sm->GetCurrentState() == "Happy")
				RenderMesh(meshList[GEO_HAPPY], false);
			else
				RenderMesh(meshList[GEO_SHARK], false);
		}
		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISHFOOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_FISHFOOD], false);
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

	zOffset = 0;
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			zOffset += 0.001f;
			RenderGO(go);
			
			if (go->nearest != nullptr && go->nearest->active)
				RenderLine(go->pos, go->nearest->pos);
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
	ss << "Count:" << SceneData::GetInstance()->GetObjectCount();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Fishes:" << SceneData::GetInstance()->GetFishCount();//m_numGO[GameObject::GO_FISH];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 18);

	ss.str("");
	ss << "Shark:" << m_numGO[GameObject::GO_SHARK];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 15);

	ss.str("");
	ss << "Queue:" << m_messageQueue.size();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 21);

	ss.str("");
	ss << "Food:" << m_numGO[GameObject::GO_FISHFOOD];
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 12);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Aquarium", Color(0, 1, 0), 3, 50, 0);
}

void SceneMovement::RenderLine(Vector3 origin, Vector3 destination)
{
	Vector3 dist = destination - origin;

	float l = dist.Length();
	modelStack.PushMatrix();
	modelStack.Translate(origin.x, origin.y, 0);
	modelStack.Rotate(Math::RadianToDegree(atan2f(dist.y, dist.x)), 0, 0, 1);
	modelStack.Scale(l, l, 1);
	RenderMesh(meshList[GEO_LINE], false);
	modelStack.PopMatrix();
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

	while (!m_messageQueue.empty())
	{
		delete m_messageQueue.front();
		m_messageQueue.pop();
	}
}

bool SceneMovement::Handle(Message* message)
{
	if (m_messageQueue.size() < 1000)
		m_messageQueue.push(message);
	else
		delete message;
	return true;
}

void SceneMovement::Process(Message* message)
{
	MessageWRU* messageWRU = dynamic_cast<MessageWRU*>(message);
	if (messageWRU)
	{
		//process messageWRU
		switch (messageWRU->type)
		{
			case MessageWRU::NEAREST_FULLFISH:
			{
				float nearestDist = FLT_MAX;

				for (std::vector<GameObject*>::iterator it = m_goList.begin(); it < m_goList.end(); ++it)
				{
					GameObject* go = (GameObject*)*it;
					if (go->active)
					{
						if (go->type == GameObject::GO_FISH)
						{
							if (go->sm->GetCurrentState() == "TooFull" || go->sm->GetCurrentState() == "Full")
							{
								float distSq = (go->pos - messageWRU->go->pos).LengthSquared();

								if (nearestDist < distSq)
								{
									nearestDist = distSq;
									messageWRU->go->nearest = go;
								}
							}
						}
					}
				}
			}
			break;

			case MessageWRU::NEAREST_FISHFOOD:
			{
				float nearestDist = FLT_MAX;

				for (std::vector<GameObject*>::iterator it = m_goList.begin(); it < m_goList.end(); ++it)
				{
					GameObject* go = (GameObject*)*it;
					if (go->active)
					{
						if (go->type == GameObject::GO_FISHFOOD)
						{
							float distSq = (go->pos - messageWRU->go->pos).LengthSquared();

							if (nearestDist < distSq)
							{
								nearestDist = distSq;
								messageWRU->go->nearest = go;
							}
						}
					}
				}
			}
			break;

			case MessageWRU::NEAREST_SHARK:
			{
				float nearestDist = FLT_MAX;

				for (std::vector<GameObject*>::iterator it = m_goList.begin(); it < m_goList.end(); ++it)
				{
					GameObject* go = (GameObject*)*it;
					if (go->type == GameObject::GO_SHARK)
					{
						float distSq = (go->pos - messageWRU->go->pos).LengthSquared();

						if (nearestDist < distSq)
						{
							nearestDist = distSq;
							messageWRU->go->nearest = go;
						}
					}
				}
			}
			break;

			case MessageWRU::HIGHEST_ENERGYFISH:
			{
				float highestEnergy = 0;

				for (std::vector<GameObject*>::iterator it = m_goList.begin(); it < m_goList.end(); ++it)
				{
					GameObject* go = (GameObject*)*it;

					if (!go->active)
						continue;
					if (go->energy > highestEnergy)
					{
						highestEnergy = go->energy;
						messageWRU->go->nearest = go;
					}
				}
			}
			break;
		}

		delete message;
		return;
	}

	MessageSpawnFood* messageSpawnFishFood = dynamic_cast<MessageSpawnFood*>(message);

	if (messageSpawnFishFood) {
		const float gridOffset = sceneData.GetGridOffset();
		const float gridCellSize = sceneData.GetGridSize();

		int offsetFromGOX = Math::RandIntMinMax(-2, 2); //< 3 grids away
		int offsetFromGOY = Math::RandIntMinMax(-2, 2); //< 3 grids away

		int cellX = Math::Clamp(static_cast<int>((messageSpawnFishFood->go->pos.x - gridOffset) / gridCellSize) + offsetFromGOX, 0, sceneData.GetGridNo() - 1);
		int cellY = Math::Clamp(static_cast<int>((messageSpawnFishFood->go->pos.y - gridOffset) / gridCellSize) + offsetFromGOY, 0, sceneData.GetGridNo() - 1);

		const float xPos = cellX * gridCellSize + gridOffset;
		const float yPos = cellY * gridCellSize + gridOffset;

		GameObject* go = FetchGO(GameObject::GO_FISHFOOD);
		go->id = sceneData.GetObjectCount();
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(xPos, yPos, 0.0f);
		go->target = go->pos;
		go->steps = 0;
		go->moveSpeed = 2.0f;
		go->nearest = nullptr;
	}
	delete message;
	return;
}