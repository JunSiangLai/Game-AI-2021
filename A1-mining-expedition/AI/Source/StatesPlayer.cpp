#include "StatesPlayer.h"
#include "PostOfficeV2.h"
#include "SceneData.h"
#include "GameObjectV2.h"

// Shared ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~


// StatePlayerSearch ~~~~~~~~~~~~~~~~~~
StatePlayerSearch::StatePlayerSearch(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StatePlayerSearch::~StatePlayerSearch()
{
}

void StatePlayerSearch::Enter()
{
}

void StatePlayerSearch::Update(double dt)
{
	// State Change
	// Wave Spawns
	if (GetGO()->currHealth <= GetGO()->maxHealth * 0.1 && GetGO()->wave == true)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Run");
		return;
	}
	if (GetGO()->wave)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		if (GetGO()->type == GameObjectV2::GO_DRILLER)
		{
			if (GetGO()->currHealth > GetGO()->maxHealth * 0.5)
			{
				GetGO()->SetNextState("DriAttack");
			}
			else
			{
				msg = new MessageV2(GetGO(), MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
				GetGO()->m_PostOffice->Send("SceneMining", msg);
				GetGO()->SetNextState("DriDefend");
			}
		}
		else
		{
			GetGO()->SetNextState("EngDefend");
		}
		return;
	}

	// Search
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0,0,0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	if ((GetGO()->closestObject->pos - GetGO()->pos).Length() < 1)
	{
		GetGO()->SetNextState("Mine");
	}
	else 
	{
		GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StatePlayerSearch::Exit()
{
}


// StatePlayerMine ~~~~~~~~~~~~~~
StatePlayerMine::StatePlayerMine(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StatePlayerMine::~StatePlayerMine()
{
}

void StatePlayerMine::Enter()
{
	GetGO()->moveSpeed = 0;
	GetGO()->countDown = 5;
	GetGO()->currOre = GetGO()->closestObject;
}

void StatePlayerMine::Update(double dt)
{
	// State Change
	// Wave Spawns
	if (GetGO()->currHealth <= GetGO()->maxHealth * 0.1 && GetGO()->wave == true)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Run");
		return;
	}
	if (GetGO()->wave)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		if (GetGO()->type == GameObjectV2::GO_DRILLER)
		{
			if (GetGO()->currHealth > GetGO()->maxHealth * 0.5)
			{
				GetGO()->SetNextState("DriAttack");
			}
			else
			{
				msg = new MessageV2(GetGO(), MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
				GetGO()->m_PostOffice->Send("SceneMining", msg);
				GetGO()->SetNextState("DriDefend");
			}
		}
		else
		{
			GetGO()->SetNextState("EngDefend");
		}
		return;
	}

	// Mine
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->countDown -= dt;
	if (GetGO()->countDown < 0 || GetGO()->currOre->active == false)
	{
		GetGO()->closestObject->active = false;
		GetGO()->SetNextState("Search");
	}
}

void StatePlayerMine::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StatePlayerRun ~~~~~~~~~~~~~~
StatePlayerRun::StatePlayerRun(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StatePlayerRun::~StatePlayerRun()
{
}

void StatePlayerRun::Enter()
{
	/*GetGO()->moveSpeed = 0;*/
}

void StatePlayerRun::Update(double dt)
{
	// State Change
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}
	if (GetGO()->wave == false)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Search");
		return;
	}
	if (GetGO()->currHealth >= GetGO()->maxHealth * 0.25)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		if (GetGO()->type == GameObjectV2::GO_DRILLER)
		{
			msg = new MessageV2(GetGO(), MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send("SceneMining", msg);
			GetGO()->SetNextState("DriDefend");
		}
		else
		{
			GetGO()->SetNextState("EngDefend");
		}
		return;
	}

	// Run
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	Vector3 dir = GetGO()->closestObject->pos - GetGO()->pos;
	GetGO()->target = GetGO()->pos - dir.Normalized();
}

void StatePlayerRun::Exit()
{
}

StatePlayerExtract::StatePlayerExtract(const std::string& stateID)
	: State(stateID)
{
}

StatePlayerExtract::~StatePlayerExtract()
{
}

void StatePlayerExtract::Enter()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}

void StatePlayerExtract::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DROPPOD, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->target = GetGO()->closestObjectPos;

	if ((GetGO()->closestObjectPos - GetGO()->pos).Length() < SceneData::GetInstance()->GetGridOffset())
	{
		GetGO()->escaped = true;
		GetGO()->active = false;
	}
}

void StatePlayerExtract::Exit()
{
}

// Driller ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~


// StateDrillerAttack ~~~~~~~~~~~~~~
StateDrillerAttack::StateDrillerAttack(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateDrillerAttack::~StateDrillerAttack()
{
}

void StateDrillerAttack::Enter()
{
	/*GetGO()->moveSpeed = 0;*/
	GetGO()->countDown = 0;
}

void StateDrillerAttack::Update(double dt)
{
	// State Change
	if (GetGO()->wave == false)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Search");
		return;
	}
	if (GetGO()->currHealth <= GetGO()->maxHealth * 0.5)
	{
		if (!GetGO()->EngineerDead)
		{
			MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send("SceneMining", msg);
			GetGO()->SetNextState("DriDefend");
		}
		if (GetGO()->currHealth <= GetGO()->maxHealth * 0.1)
		{
			GetGO()->SetNextState("Run");
		}
	}
	

	// Attack
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->atkCD -= dt;
	GetGO()->countDown -= dt;
	if (GetGO()->closestObjectDist > 4)
	{
		GetGO()->target = GetGO()->closestObject->pos;

	}
	else
	{
		if (GetGO()->atkCD <= 0)
		{
			MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 20.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
			GetGO()->atkCD = GetGO()->atkSpd;
			GetGO()->countDown = 0.5;
		}
		else 
		{
			Vector3 dir = GetGO()->closestObject->pos - GetGO()->pos;
			GetGO()->target = GetGO()->pos - dir.Normalized() * 2.5;
		}
	}
	if (GetGO()->countDown > 0)
	{
		GetGO()->moveSpeed = 0;
	}
	else 
	{
		GetGO()->moveSpeed = GetGO()->baseSpeed;
	}
}

void StateDrillerAttack::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StateDrillerDefend ~~~~~~~~~~~~~~
StateDrillerDefend::StateDrillerDefend(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateDrillerDefend::~StateDrillerDefend()
{
}

void StateDrillerDefend::Enter()
{
	/*GetGO()->moveSpeed = 0;*/
}

void StateDrillerDefend::Update(double dt)
{
	if (GetGO()->wave == false)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Search");
		return;
	}
	if (GetGO()->currHealth > GetGO()->maxHealth * 0.5)
	{
		GetGO()->SetNextState("DriAttack");
	}
	if (GetGO()->closestObjectPos.IsZero())
	{
		GetGO()->SetNextState("DriAttack");
		GetGO()->EngineerDead = true;
	}
	if (GetGO()->currHealth <= GetGO()->maxHealth * 0.1)
	{
		GetGO()->SetNextState("Run");
	}

	// Defend
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_ENGINEERLOCATION, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->atkCD -= dt;
	if ( (GetGO()->closestObjectPos - GetGO()->pos).Length() > 2.5)
	{
		GetGO()->moveSpeed = GetGO()->baseSpeed;
		GetGO()->target = GetGO()->closestObjectPos;
	}
	else 
	{
		GetGO()->moveSpeed = 0;
		if (GetGO()->atkCD <= 0 && GetGO()->closestObjectDist < 4)
		{
			MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 20.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
			GetGO()->atkCD = GetGO()->atkSpd;
		}
	}
}

void StateDrillerDefend::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}

// Engineer ~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~


// StateEngineerDefend ~~~~~~~~~~~~~~
StateEngineerDefend::StateEngineerDefend(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateEngineerDefend::~StateEngineerDefend()
{
}

void StateEngineerDefend::Enter()
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_TURRET, 0.f, Vector3(0, 0, 0), true);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->moveSpeed = 0;
}

void StateEngineerDefend::Update(double dt)
{
	if (GetGO()->wave == false)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTORE, 0.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Search");
		return;
	}
	if (GetGO()->currHealth <= GetGO()->maxHealth * 0.2)
	{
		GetGO()->SetNextState("Run");
	}

	// Defend
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTENEMY, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_WAVE, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->atkCD -= dt;
	if (GetGO()->atkCD <= 0 && GetGO()->closestObjectDist < 4)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 15.f, Vector3(0, 0, 0), false);
		GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
		GetGO()->atkCD = GetGO()->atkSpd;
	}
}

void StateEngineerDefend::Exit()
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_TURRET, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}