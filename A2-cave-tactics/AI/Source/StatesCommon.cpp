#include "StatesCommon.h"
#include "PostOfficeV2.h"
#include "GameObjectV2.h"

StateDeath::StateDeath(const std::string& stateID)
	: State(stateID)
{
	//m_postOffice = PostOfficeV2::GetInstance();
}

StateDeath::~StateDeath()
{
}

void StateDeath::Enter()
{
	GetGO()->countDown = 1.f;
	GetGO()->moveSpeed = 0.f;
}

void StateDeath::Update(double dt)
{
	GetGO()->countDown -= dt;
	if (GetGO()->countDown < 0.f)
	{
		GetGO()->active = false;
	}
}

void StateDeath::Exit()
{
}