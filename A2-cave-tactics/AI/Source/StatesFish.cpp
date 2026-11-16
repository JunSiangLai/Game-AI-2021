#include "StatesFish.h"

#pragma region toofull
StateTooFull::StateTooFull(const std::string & stateID, GameObject * go)
	: State(stateID),
	m_go(go)
{
}

StateTooFull::~StateTooFull()
{
}

void StateTooFull::Enter()
{
	m_go->moveSpeed = 2;
}

void StateTooFull::Update(double dt)
{
	m_go->energy -= dt;
	if (m_go->energy < 10)
		m_go->sm->SetNextState("fishFull");
}

void StateTooFull::Exit()
{
}
#pragma endregion

#pragma region hungry
StateHungry::StateHungry(const std::string& stateID, GameObject* go) : State(stateID),
m_go(go)
{

}
StateHungry::~StateHungry()
{

}

void StateHungry::Enter()
{
	m_go->moveSpeed = 7.f;
}
void StateHungry::Update(double dt)
{
	m_go->energy -= dt;
	if (m_go->energy >= 10)
		m_go->sm->SetNextState("fishTooFull");
	else if (m_go->energy < 10 && m_go->energy >= 5)
		m_go->sm->SetNextState("fishFull");
	else if (m_go->energy <= 0)
		m_go->sm->SetNextState("fishDead");
}
void StateHungry::Exit()
{

}
#pragma endregion

#pragma region full
StateFull::StateFull(const std::string& stateID, GameObject* go) : State(stateID),
m_go(go)
{

}
StateFull::~StateFull()
{

}

void StateFull::Enter()
{
	m_go->moveSpeed = 10.f;
}
void StateFull::Update(double dt)
{
	m_go->energy -= dt;
	if (m_go->energy < 5)
		m_go->sm->SetNextState("fishHungry");
	else if (m_go->energy > 10)
		m_go->sm->SetNextState("fishTooFull");
}
void StateFull::Exit()
{

}
#pragma endregion

#pragma region dead
StateDead::StateDead(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateDead::~StateDead()
{
}

void StateDead::Enter()
{
	m_go->moveSpeed = 0;
}

void StateDead::Update(double dt)
{
	m_go->countDown -= dt;
	if (m_go->countDown <= 0)
	{
		m_go->active = false;
		--m_go->fishCount;
	}
}

void StateDead::Exit()
{

}
#pragma endregion