#pragma once

#include "State.h"

class StatePound : public State
{
	GameObjectV2* m_go;
public:
	StatePound(const std::string& stateID);
	virtual ~StatePound();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateFireball : public State
{
	GameObjectV2* m_go;
public:
	StateFireball(const std::string& stateID);
	virtual ~StateFireball();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateSuicideBomb : public State
{
	GameObjectV2* m_go;
public:
	StateSuicideBomb(const std::string& stateID);
	virtual ~StateSuicideBomb();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};