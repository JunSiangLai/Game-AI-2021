#pragma once

#include <string>
#include <queue>
#include "MessageV2.h"

class ObjectBaseV2
{
public:
	virtual ~ObjectBaseV2() {}
	virtual bool Handle(MessageV2 *message) = 0;
	virtual void Read () = 0;
protected:
	std::queue<MessageV2*> m_inbox;
};