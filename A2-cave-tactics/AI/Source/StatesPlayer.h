#pragma once

#include "State.h"

// Shared ~~~~~~~~~~~
class StatePlayerSearch : public State
{
	GameObjectV2* m_go;
public:
	StatePlayerSearch(const std::string& stateID);
	virtual ~StatePlayerSearch();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePlayerMine : public State
{
	GameObjectV2* m_go;
public:
	StatePlayerMine(const std::string& stateID);
	virtual ~StatePlayerMine();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePlayerDeposit : public State
{
	GameObjectV2* m_go;
public:
	StatePlayerDeposit(const std::string& stateID);
	virtual ~StatePlayerDeposit();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePlayerRun : public State
{
	GameObjectV2* m_go;
public:
	StatePlayerRun(const std::string& stateID);
	virtual ~StatePlayerRun();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StatePlayerExtract : public State
{
	GameObjectV2* m_go;
public:
	StatePlayerExtract(const std::string& stateID);
	virtual ~StatePlayerExtract();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};


// Driller ~~~~~~~~~~~
class StateDrillerAttack : public State
{
	GameObjectV2* m_go;
public:
	StateDrillerAttack(const std::string& stateID);
	virtual ~StateDrillerAttack();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};

class StateDrillerDefend : public State
{
	GameObjectV2* m_go;
public:
	StateDrillerDefend(const std::string& stateID);
	virtual ~StateDrillerDefend();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};


// Engineer ~~~~~~~~~~~
class StateEngineerDefend : public State
{
	GameObjectV2* m_go;
public:
	StateEngineerDefend(const std::string& stateID);
	virtual ~StateEngineerDefend();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void Exit();
};
