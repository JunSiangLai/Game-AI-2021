#include "StatesGrunts.h"
#include "PostOfficeV2.h"
#include "GameObjectV2.h"

// StateChase ~~~~~~~~~~~
StateChase::StateChase(const std::string& stateID)
	: State(stateID)
{
	//m_PostOffice = PostOfficeV2::GetInstance();
}

StateChase::~StateChase()
{
}

void StateChase::Enter()
{
	/*GetGO()->moveSpeed = 0;*/
}

void StateChase::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	// Flee
	if (GetGO()->currHealth <= (GetGO()->maxHealth * 0.1) && !GetGO()->nearDetonator)
	{
		GetGO()->SetNextState("Flee");
		return;
	}
	// Attack
	if (GetGO()->closestObjectDist < 1)
	{
		GetGO()->SetNextState("Attack");
		return;
	}
	// Death
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}

	// Chase
	if (GetGO()->target != GetGO()->closestObject->pos)
	{
		if(GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
			GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StateChase::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StateAttack ~~~~~~~~~
StateAttack::StateAttack(const std::string& stateID)
	: State(stateID)
{
	//m_PostOffice = PostOfficeV2::GetInstance();
}

StateAttack::~StateAttack()
{
}

void StateAttack::Enter()
{
	GetGO()->moveSpeed = 0;
	GetGO()->countDown = 1.5;
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 15.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
}

void StateAttack::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	// Flee
	if (GetGO()->currHealth <= (GetGO()->maxHealth * 0.1) && !GetGO()->nearDetonator)
	{
		GetGO()->SetNextState("Flee");
		return;
	}
	// Death
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}

	// Attack
	GetGO()->countDown -= dt;
	if (GetGO()->countDown <= 0)
	{
		GetGO()->SetNextState("Chase");
	}

	if (GetGO()->target != GetGO()->closestObject->pos)
	{
		if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
			GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StateAttack::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StateFlee ~~~~~~~~~
StateFlee::StateFlee(const std::string& stateID)
	: State(stateID)
{
	//m_PostOffice = PostOfficeV2::GetInstance();
}

StateFlee::~StateFlee()
{
}

void StateFlee::Enter()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed * 0.8;
}

void StateFlee::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	msg = new MessageV2(GetGO(), MessageV2::MSG_NEARDETONATOR, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	// Die
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}
	if (GetGO()->nearDetonator || GetGO()->currHealth > (GetGO()->maxHealth * 0.1))
	{
		GetGO()->SetNextState("Chase");
	}

	Vector3 dir = GetGO()->closestObject->pos - GetGO()->pos;
	Vector3 run;
	if (dir.x >= 0)
		run.x = GetGO()->pos.x - 1;
	else
		run.x = GetGO()->pos.x + 1;
	if (dir.y >= 0)
		run.y = GetGO()->pos.y - 1;
	else
		run.y = GetGO()->pos.y + 1;
	if (GetGO()->target != run)
	{
		if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
		{
			GetGO()->target = GetGO()->closestObject->pos;
		}
	}
}

void StateFlee::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}