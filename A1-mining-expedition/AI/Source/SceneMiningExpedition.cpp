#include "SceneMiningExpedition.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <limits>
#include "StatesGrunts.h"
#include "StatesDetonator.h"
#include "StatesPlayer.h"
#include "StatesCommon.h"
#include "SceneData.h"
#include "PostOfficeV2.h"
#include "GameObjectV2.h"

static SceneData& sceneData = *SceneData::GetInstance();

SceneMiningExpedition::SceneMiningExpedition()
{
}

SceneMiningExpedition::~SceneMiningExpedition()
{

}

void SceneMiningExpedition::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 200.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	Math::InitRNG();

	//// Camera
	Application::m_ismousescrollymoved = false;
	Application::m_mousescrollx = 0.f;
	Application::m_mousescrolly = 0.f;
	m_cameraZoom = 2;
	camera.position.x = -25;
	camera.target.x = -25;

	Math::InitRNG();

	m_PostOffice = PostOfficeV2::GetInstance();
	m_PostOffice->Register("SceneMining", this);

	sceneData.SetObjectCount(0);
	sceneData.SetFishCount(0);

	sceneData.SetGridNo(40);
	sceneData.SetGridSize(m_worldHeight / sceneData.GetGridNo());
	sceneData.SetGridOffset(sceneData.GetGridSize() / 2);

	// Resources
	for (int i = 0; i < 35; ++i)
	{
		GameObjectV2* ore = new GameObjectV2(GameObjectV2::GO_ORE);
		ore->active = true;
		ore->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
		m_resourceList.push_back(ore);
	}
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
	m_DropPod->pos = Vector3(sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), 0);

	// Wave Timer
	m_waveTimer = 15;

	// Crystal Count
	crystals = 0;
	numPlayers = 0;
	nextFrame = setUp = end = win = false;
}

void SceneMiningExpedition::SetUp()
{
	// Set Up
	std::cout << "\x1b[2J" << "\x1b[2J\x1b[H";
	if (numPlayers == 0)
	{
		std::cout << "Number of Players? (Min:1 - Max:4)" << std::endl;
		static bool OneButtonState = false;
		if (!OneButtonState && Application::IsKeyPressed('1'))
		{
			OneButtonState = true;
		}
		else if (OneButtonState && !Application::IsKeyPressed('1'))
		{
			OneButtonState = false;
			numPlayers = 1;
		}
		static bool TwoButtonState = false;
		if (!TwoButtonState && Application::IsKeyPressed('2'))
		{
			TwoButtonState = true;
		}
		else if (TwoButtonState && !Application::IsKeyPressed('2'))
		{
			TwoButtonState = false;
			numPlayers = 2;
		}
		static bool ThreeButtonState = false;
		if (!ThreeButtonState && Application::IsKeyPressed('3'))
		{
			ThreeButtonState = true;
		}
		else if (ThreeButtonState && !Application::IsKeyPressed('3'))
		{
			ThreeButtonState = false;
			numPlayers = 3;
		}
		static bool FourButtonState = false;
		if (!FourButtonState && Application::IsKeyPressed('4'))
		{
			FourButtonState = true;
		}
		else if (FourButtonState && !Application::IsKeyPressed('4'))
		{
			FourButtonState = false;
			numPlayers = 4;
		}

	}
	else
	{
		std::cout << numPlayers << " Players Selected." << std::endl;

		if (m_playerList.size() != numPlayers)
		{
			int choice = 0;
			std::cout << "Player " << m_playerList.size() + 1  << ": Engineer(1) Or Driller(2)" << std::endl;

			static bool OneButtonState = false;
			if (!OneButtonState && Application::IsKeyPressed('1'))
			{
				OneButtonState = true;
			}
			else if (OneButtonState && !Application::IsKeyPressed('1'))
			{
				OneButtonState = false;
				choice = 1;
			}
			static bool TwoButtonState = false;
			if (!TwoButtonState && Application::IsKeyPressed('2'))
			{
				TwoButtonState = true;
			}
			else if (TwoButtonState && !Application::IsKeyPressed('2'))
			{
				TwoButtonState = false;
				choice = 2;
			}

			if (choice != 0)
			{
				switch (choice)
				{
				case 1:
				{
					// Engineer
					GameObjectV2* m_Engineer = new GameObjectV2();
					m_Engineer->type = GameObjectV2::GO_ENGINEER;
					m_Engineer->active = true;
					m_Engineer->baseSpeed = m_Engineer->moveSpeed = 10;
					m_Engineer->maxHealth = m_Engineer->currHealth = 200;
					m_Engineer->atkSpd = 0.6;
					m_Engineer->target.SetZero();
					m_Engineer->pos = m_Engineer->step = Vector3(sceneData.GetGridOffset() + (18 + m_playerList.size()) * sceneData.GetGridSize(), sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), 0);
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
					break;
				}
				case 2:
				{
					// Driller
					GameObjectV2* m_Driller = new GameObjectV2();
					m_Driller->type = GameObjectV2::GO_DRILLER;
					m_Driller->active = true;
					m_Driller->baseSpeed = m_Driller->moveSpeed = 10;
					m_Driller->maxHealth = m_Driller->currHealth = 200;
					m_Driller->atkSpd = 1;
					m_Driller->target.SetZero();
					m_Driller->pos = m_Driller->step = Vector3(sceneData.GetGridOffset() + (18 + m_playerList.size()) * sceneData.GetGridSize(), sceneData.GetGridOffset() + 20 * sceneData.GetGridSize(), 0);
					m_Driller->sm = DrillerState;
					m_Driller->SetNextState("Search");
					msg = new MessageV2(m_Driller, MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
					m_PostOffice->Send("SceneMining", msg);

					m_playerList.push_back(m_Driller);
					break;
				}
				default:
					std::cout << "Invalid Choice" << std::endl;
					break;
				}
			}
		}
		else 
		{
			nextFrame = true;

			for (int i = 0; i < 30; ++i)
			{
				GameObjectV2* grunt = new GameObjectV2(GameObjectV2::GO_GRUNT);
				grunt->active = false;
				grunt->baseSpeed = 12;
				grunt->atkSpd = 1;
				grunt->maxHealth = 40 * (numPlayers * 0.5);
				grunt->sm = GruntState;

				m_gruntList.push_back(grunt);
			}
			for (int i = 0; i < 5; ++i)
			{
				GameObjectV2* detonator = new GameObjectV2(GameObjectV2::GO_DETONATOR);
				detonator->active = false;
				detonator->baseSpeed = 6;
				detonator->atkSpd = 2;
				detonator->maxHealth = 300 * (numPlayers * 0.5);
				detonator->sm = DetonatorState;

				m_detonatorList.push_back(detonator);
			}
		}
	}
}

GameObjectV2* SceneMiningExpedition::FetchGO(GameObjectV2::GAMEOBJECT_TYPE type)
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

void SceneMiningExpedition::Update(double dt)
{
	if (end)
	{
		return;
	}
	if (!nextFrame)
	{
		SetUp();
		return;
	}
	else 
	{
		setUp = true;
	}

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

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
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
	//static bool bSpaceState = false;
	/*if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}*/
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

	// WAVE ~~~~~~~~~~~~~~~~~~~~
	m_waveTimer -= dt;
	if (m_waveTimer <= 0 && wave == false)
	{
		wave = true;
		int grunts = Math::RandIntMinMax(8 * (m_playerList.size() * 0.5), 15 * (m_playerList.size() * 0.5));
		int detonator = Math::RandIntMinMax(1 * (m_playerList.size() * 0.5), 2 * (m_playerList.size() * 0.5));
		for (int i = 0; i < 5; ++i)
		{
			GameObjectV2* enemy = FetchGO(GameObjectV2::GO_GRUNT);
			enemy->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
			enemy->step = enemy->pos;
			msg = new MessageV2(enemy, MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", msg);
			msg = new MessageV2(enemy, MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), false);
			m_PostOffice->Send("SceneMining", msg);
		}
		for (int i = 0; i < 1; ++i)
		{
			GameObjectV2* enemy = FetchGO(GameObjectV2::GO_DETONATOR);
			enemy->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetGridNo() - 1) * sceneData.GetGridSize(), 0);
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
			m_waveTimer = Math::RandIntMinMax(20, 35);
		}
	}

	// Process Messages
	Read();

	if (crystals >= 10)
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

	// Grunt
	for (auto it = m_gruntList.begin(); it != m_gruntList.end(); ++it)
	{
		GameObjectV2* grunt = (GameObjectV2*)*it;
		if (grunt->active)
		{
			grunt->Update(dt * m_speed);
		}
	}

	// Detonator
	for (auto it = m_detonatorList.begin(); it != m_detonatorList.end(); ++it)
	{
		GameObjectV2* detonator = (GameObjectV2*)*it;
		if (detonator->active)
		{
			detonator->Update(dt* m_speed);
		}
	}

	// Turret
	for (auto it = m_turretList.begin(); it != m_turretList.end(); ++it)
	{
		GameObjectV2* turret = (GameObjectV2*)*it;
		if (turret->active)
		{
			turret->Update(dt * m_speed);
		}
	}
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

	// Reset mouseWheel
	Application::m_ismousescrollymoved = false;
	Application::m_mousescrollx = 0.f;
	Application::m_mousescrolly = 0.f;
	//SceneData::GetInstance()->SetObjectCount(objectCount);
}


void SceneMiningExpedition::RenderGO(GameObjectV2* go, float zOffset)
{
	std::ostringstream ss;
	switch (go->type)
	{
	case GameObjectV2::GO_DRILLER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
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
	case GameObjectV2::GO_ENGINEER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
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
	case GameObjectV2::GO_GRUNT:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
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
	case GameObjectV2::GO_DETONATOR:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
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
	case GameObjectV2::GO_ORE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ORE], false);
		modelStack.PopMatrix();
		break;
	case GameObjectV2::GO_TURRET:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(1, 1, 1);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObjectV2::GO_DROPPOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zOffset);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DROPPOD], false);
		modelStack.PopMatrix();
		break;
	}

}

void SceneMiningExpedition::Render()
{
	if (!setUp)
	{
		return;
	}

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

	RenderMesh(meshList[GEO_AXES], false);

	for (int i = 0; i < 2; ++i)
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_worldHeight * 0.5f + m_worldHeight * i, m_worldHeight * 0.5f, -1.f);
		modelStack.Scale(m_worldHeight, m_worldHeight, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldHeight * 0.5f + m_worldHeight * i, m_worldHeight * 1.5f, -1.f);
		modelStack.Scale(m_worldHeight, m_worldHeight, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();
	}

	for (int i = 0; i < sceneData.GetGridNo(); ++i)
	{
		for (int j = 0; j < sceneData.GetGridNo(); ++j)
		{
			Vector3 gridPos = Vector3(j * sceneData.GetGridSize() + sceneData.GetGridOffset(),
										i * sceneData.GetGridSize() + sceneData.GetGridOffset(),0);

			modelStack.PushMatrix();
			modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
			modelStack.Scale(5, 5, 5);
			RenderMesh(meshList[GEO_GROUND], false);
			modelStack.PopMatrix();
			
			gridPos.z = 0.001f;
			if (i == 0)
			{
				switch (j)
				{
				case 0:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_CORNER], false);
					modelStack.PopMatrix();
					break;
				case 39:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Rotate(90.f, 0, 0, 1);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_CORNER], false);
					modelStack.PopMatrix();
					break;
				default:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_SIDE], false);
					modelStack.PopMatrix();
					break;
				}
			}
			else if (i == 39)
			{
				switch (j)
				{
				case 0:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Rotate(-90.f, 0, 0, 1);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_CORNER], false);
					modelStack.PopMatrix();
					break;
				case 39:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Rotate(-180.f, 0, 0, 1);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_CORNER], false);
					modelStack.PopMatrix();
					break;
				default:
					modelStack.PushMatrix();
					modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
					modelStack.Rotate(-180.f, 0, 0, 1);
					modelStack.Scale(5, 5, 5);
					RenderMesh(meshList[GEO_SIDE], false);
					modelStack.PopMatrix();
					break;
				}
			}
			else if (j == 0)
			{
				modelStack.PushMatrix();
				modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
				modelStack.Rotate(-90.f, 0, 0, 1);
				modelStack.Scale(5, 5, 5);
				RenderMesh(meshList[GEO_SIDE], false);
				modelStack.PopMatrix();
			}
			else if (j == 39)
			{
				modelStack.PushMatrix();
				modelStack.Translate(gridPos.x, gridPos.y, gridPos.z);
				modelStack.Rotate(90.f, 0, 0, 1);
				modelStack.Scale(5, 5, 5);
				RenderMesh(meshList[GEO_SIDE], false);
				modelStack.PopMatrix();
			}
		}
	}

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
	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, 2);

	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, 4);

	ss.str("");
	ss << "Count:" << SceneData::GetInstance()->GetObjectCount();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, 6);

	ss.str("");
	ss.precision(3);
	ss << "Wave:" << m_waveTimer;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, 8);

	ss.str("");
	ss.precision(3);
	ss << "Enemies Left:" << enemies;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, 10);

	int playernum = 0;
	int y = 10;
	for (auto it = m_playerList.begin(); it != m_playerList.end(); ++it)
	{
		GameObjectV2* player = (GameObjectV2*)*it;
		++playernum;
		y += 4;
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
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, y - 2);

		ss.str("");
		ss.precision(3);
		ss << "Player " << playernum << ": (" << playerClass
			<< ") Health:" << player->currHealth;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 2, 55, y);

	}

	if (end)
	{
		switch (win)
		{
		case true:
			ss.str("");
			ss << "WIN";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 10, 40, 40);
			break;
		case false:
			ss.str("");
			ss << "LOSE";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.2, 0.2, 0), 10, 40, 40);
			break;
		default:
			break;
		}
	}

	RenderTextOnScreen(meshList[GEO_TEXT], "Cave", Color(0.2, 0.2, 0), 2, 55, 0);
}

void SceneMiningExpedition::RenderLine(Vector3 origin, Vector3 destination) 
{
	Vector3 dist = destination - origin;

	float l = dist.Length();
	modelStack.PushMatrix();
	modelStack.Translate(origin.x, origin.y, 5);
	modelStack.Rotate(Math::RadianToDegree(atan2f(dist.y, dist.x)), 0, 0, 1);
	modelStack.Scale(l, l, 1);
	RenderMesh(meshList[GEO_LINE], false);
	modelStack.PopMatrix();
}

void SceneMiningExpedition::Exit() 
{
	SceneBase::Exit();
	//Cleanup GameObjects
	/*while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}*/
} 

bool SceneMiningExpedition::Handle(MessageV2* Message)
{
	MessageV2* messageWRU = dynamic_cast<MessageV2*>(Message);
	if (messageWRU)
	{
		m_inbox.push(Message);
		return true;
	}
	return false;
}

void SceneMiningExpedition::Read()
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
					turret->pos = message->go->pos + Vector3(Math::RandFloatMinMax(-3, 3), Math::RandFloatMinMax(-3, 3), 0);
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
			crystals = 35 - message->threshold;
			break;
		}
		}
		delete message;
		m_inbox.pop();
	}
}