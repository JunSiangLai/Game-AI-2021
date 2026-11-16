#pragma once

#include "State.h"

class StateChase : public State
{
	GameObjectV2* m_go;
public:
	StateChase(const std::string& stateID);
	virtual ~StateChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateAttack : public State
{
	GameObjectV2* m_go;
public:
	StateAttack(const std::string& stateID);
	virtual ~StateAttack();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateFlee : public State
{
	GameObjectV2* m_go;
public:
	StateFlee(const std::string& stateID);
	virtual ~StateFlee();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};