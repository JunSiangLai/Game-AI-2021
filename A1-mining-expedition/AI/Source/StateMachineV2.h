#pragma once

#include <string>
#include <map>
#include "ObjectBaseV2.h"
#include "State.h"

class StateMachineV2 : public ObjectBaseV2
{
	std::map<std::string, State*> m_stateMap;

public:
	StateMachineV2();
	~StateMachineV2();
	State* getState(const std::string& nextStateID);
	void AddState(State *newState);
	void Update(GameObjectV2 *go, double dt);

	virtual bool Handle(MessageV2* message);
	virtual void Read();
};