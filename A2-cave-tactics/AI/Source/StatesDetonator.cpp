#include "StatesDetonator.h"
#include "PostOfficeV2.h"
#include "GameObjectV2.h"

// StatePound ~~~~~~~~~~~
StatePound::StatePound(const std::string& stateID)
	: State(stateID)
{
	////m_postOffice = PostOfficeV2::GetInstance();
}

StatePound::~StatePound()
{
}

void StatePound::Enter()
{
	GetGO()->moveSpeed = 0;
	GetGO()->countDown = 0.5;
}

void StatePound::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);
	// Suicide
	if (GetGO()->currHealth <= (GetGO()->maxHealth * 0.10))
	{
		GetGO()->SetNextState("SuicideBomb");
		return;
	}
	// Die
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}

	GetGO()->atkCD -= dt;
	GetGO()->countDown -= dt;
	if (GetGO()->countDown <= 0)
	{
		GetGO()->moveSpeed = GetGO()->baseSpeed;
	}
	// FireBall
	if (GetGO()->closestObjectDist > 5)
	{
		GetGO()->SetNextState("Fireball");
	}
	else 
	{
		if (GetGO()->atkCD <= 0 && GetGO()->closestObjectDist < 2.5)
		{
			MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 25.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
			GetGO()->atkCD = GetGO()->atkSpd;
			GetGO()->moveSpeed = 0;
			GetGO()->countDown = 0.5;
		}
	}

	if (GetGO()->target != GetGO()->closestObject->pos)
	{
		if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
			GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StatePound::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StateFireball ~~~~~~~~~
StateFireball::StateFireball(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateFireball::~StateFireball()
{
}

void StateFireball::Enter()
{
	/*GetGO()->moveSpeed = 0;*/
}

void StateFireball::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);

	// Suicide
	if (GetGO()->currHealth <= (GetGO()->maxHealth * 0.10))
	{
		GetGO()->SetNextState("SuicideBomb");
		return;
	}
	// Die
	if (GetGO()->currHealth <= 0.f)
	{
		GetGO()->SetNextState("Death");
		return;
	}

	GetGO()->atkCD -= dt;
	// Pound
	if (GetGO()->closestObjectDist <= 5)
	{
		GetGO()->SetNextState("Pound");
	}
	else
	{
		if (GetGO()->atkCD <= 0 && GetGO()->closestObjectDist <= 20)
		{
			MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_DMG, 50.f, Vector3(0, 0, 0), false);
			GetGO()->m_PostOffice->Send(GetGO()->closestObject->ID, msg);
			GetGO()->atkCD = GetGO()->atkSpd;
		}
	}

	if (GetGO()->target != GetGO()->closestObject->pos)
	{
		if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
			GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StateFireball::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}


// StateSuicideBomb ~~~~~~~~~
StateSuicideBomb::StateSuicideBomb(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateSuicideBomb::~StateSuicideBomb()
{
}

void StateSuicideBomb::Enter()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed * 1.2;
	GetGO()->countDown = 1.5f;
}

void StateSuicideBomb::Update(double dt)
{
	MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_NEARESTPLAYER, 0.f, Vector3(0, 0, 0), false);
	GetGO()->m_PostOffice->Send("SceneMining", msg);

	// Blow Up
	GetGO()->countDown -= dt;
	if (GetGO()->countDown <= 0.f)
	{
		GetGO()->currHealth = 0.f;
	}
	else 
	{
		if (GetGO()->target != GetGO()->closestObject->pos)
		{
			if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
				GetGO()->target = GetGO()->closestObject->pos;
		}
	}
	// Die
	if (GetGO()->currHealth <= 0.f)
	{
		MessageV2* msg = new MessageV2(GetGO(), MessageV2::MSG_EXPLOSION, 0.f, GetGO()->pos, false);
		GetGO()->m_PostOffice->Send("SceneMining", msg);
		GetGO()->SetNextState("Death");
	}

	if (GetGO()->target != GetGO()->closestObject->pos)
	{
		if (GetGO()->AStar(MazePt(GetGO()->pos.x, GetGO()->pos.y), MazePt(GetGO()->closestObject->pos.x, GetGO()->closestObject->pos.y)))
			GetGO()->target = GetGO()->closestObject->pos;
	}
}

void StateSuicideBomb::Exit()
{
	GetGO()->moveSpeed = GetGO()->baseSpeed;
}