#include "StateMachineV2.h"
#include "GameObjectV2.h"

StateMachineV2::StateMachineV2()
{
}

StateMachineV2::~StateMachineV2()
{
	for (std::map<std::string, State*>::iterator it = m_stateMap.begin(); it != m_stateMap.end(); ++it)
	{
		delete it->second;
	}
	m_stateMap.clear();
}

State* StateMachineV2::getState(const std::string& nextStateID)
{
	std::map<std::string, State*>::iterator it = m_stateMap.find(nextStateID);
	if (it != m_stateMap.end())
	{
		return (State*)it->second;
	}
	return nullptr;
}

void StateMachineV2::AddState(State *newState)
{
	if (!newState)
		return;
	if (m_stateMap.find(newState->GetStateID()) != m_stateMap.end())
		return;
	m_stateMap.insert(std::pair<std::string, State*>(newState->GetStateID(), newState));
}


void StateMachineV2::Update(GameObjectV2* go, double dt)
{
	if (go->m_nextState != go->m_currState)
	{
		if (go->m_currState != NULL)
		{
			go->m_currState->Exit();
		}
		go->m_currState = go->m_nextState;
		go->m_currState->Init(go);
		go->m_currState->Enter();
	}
	go->m_currState->Init(go);
	go->m_currState->Update(dt);
}

bool StateMachineV2::Handle(MessageV2* message)
{
	MessageV2* Message = dynamic_cast<MessageV2*>(message);
	if (Message)
	{
		m_inbox.push(Message);
		return true;
	}
	return false;
}

void StateMachineV2::Read()
{
}
