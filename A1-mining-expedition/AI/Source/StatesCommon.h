#pragma once

#include "State.h"

class StateDeath : public State
{
	GameObjectV2* m_go;
public:
	StateDeath(const std::string& stateID);
	virtual ~StateDeath();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};