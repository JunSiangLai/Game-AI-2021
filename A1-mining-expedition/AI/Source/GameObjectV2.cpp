#include "GameObjectV2.h"
#include <limits>

GameObjectV2::GameObjectV2(GAMEOBJECT_TYPE typeValue)
	: type(typeValue),
	countDown(0),
	scale(5, 5, 5),
	target(0, 0, 0),
	step(0, 0, 0),
	closestObjectPos(0, 0, 0),
	active(false),
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
	sceneData = SceneData::GetInstance();

}

std::set<std::string> GameObjectV2::IDs;

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

	// State
	if (sm != NULL)
	{
		sm->Update(this, dt);
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
	Vector3 dir = target - pos;
	if (target.IsZero())
	{
		if ((step - pos).Length() < moveSpeed * dt)
		{
			pos = step;
			int random = Math::RandIntMinMax(1, 4);
			switch (random)
			{
			case 1:
				step.y += 2 * sceneData->GetGridOffset();
				break;
			case 2:
				step.y -= 2 * sceneData->GetGridOffset();
				break;
			case 3:
				step.x -= 2 * sceneData->GetGridOffset();
				break;
			case 4:
				step.x =+ 2 * sceneData->GetGridOffset();
				break;
			}
			step.x = Math::Clamp(step.x, sceneData->GetGridOffset() + 0 * sceneData->GetGridSize(), sceneData->GetGridOffset() + 39 * sceneData->GetGridSize());
			step.y = Math::Clamp(step.y, sceneData->GetGridOffset() + 0 * sceneData->GetGridSize(), sceneData->GetGridOffset() + 39 * sceneData->GetGridSize());
		}
	}
	else
	{
		if ((step - pos).Length() < moveSpeed * dt)
		{
			pos += (step - pos) - std::numeric_limits<float>::min();
			try
			{
				std::vector<int> path; // Up1, Down2, Left3, Right4
				dir.Normalize();
				if (dir.y > 0)
				{
					path.push_back(1);
				}
				else if (dir.y < 0)
				{
					path.push_back(2);
				}
				if (dir.x > 0)
				{
					path.push_back(4);
				}
				else if (dir.x < 0)
				{
					path.push_back(3);
				}
				int random = Math::RandIntMinMax(0, path.size() - 1);
				switch (path[random])
				{
				case 1:
					step.y += 2 * sceneData->GetGridOffset();
					break;
				case 2:
					step.y -= 2 * sceneData->GetGridOffset();
					break;
				case 3:
					step.x -= 2 * sceneData->GetGridOffset();
					break;
				case 4:
					step.x += 2 * sceneData->GetGridOffset();
					break;
				}
				step.x = Math::Clamp(step.x, sceneData->GetGridOffset() + 0 * sceneData->GetGridSize(), sceneData->GetGridOffset() + 39 * sceneData->GetGridSize());
				step.y = Math::Clamp(step.y, sceneData->GetGridOffset() + 0 * sceneData->GetGridSize(), sceneData->GetGridOffset() + 39 * sceneData->GetGridSize());
			}
			catch (DivideByZero)
			{
			}
		}
	}

	dir = step - pos;
	try 
	{
		dir.Normalize();
	}
	catch (DivideByZero)
	{
	}
	pos += dir * moveSpeed * static_cast<float>(dt);
}