#include "StatesShark.h"

#pragma region crazy
StateCrazy::StateCrazy(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateCrazy::~StateCrazy()
{

}

void StateCrazy::Enter()
{
	m_go->moveSpeed = 15;
}

void StateCrazy::Update(double dt)
{
	if (m_go->fishCount < 12)
	{
		m_go->sm->SetNextState("sharkNaughty");
	}
}

void StateCrazy::Exit()
{

}
#pragma endregion

#pragma region naughty
StateNaughty::StateNaughty(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateNaughty::~StateNaughty()
{

}

void StateNaughty::Enter()
{
	m_go->moveSpeed = 13;
}

void StateNaughty::Update(double dt)
{
	if (m_go->fishCount > 10)
		m_go->sm->SetNextState("sharkCrazy");
	else if (m_go->fishCount < 6)
		m_go->sm->SetNextState("sharkHappy");
}

void StateNaughty::Exit()
{

}
#pragma endregion

#pragma region happy
StateHappy::StateHappy(const std::string& stateID, GameObject* go)
	: State(stateID),
	m_go(go)
{
}

StateHappy::~StateHappy()
{

}

void StateHappy::Enter()
{
	m_go->moveSpeed = 10;
}

void StateHappy::Update(double dt)
{
	if (m_go->fishCount > 4)
		m_go->sm->SetNextState("sharkNaughty");
}

void StateHappy::Exit()
{

}
#pragma endregion