#pragma once

#include "SingletonTemplate.h"
#include <string>
#include <map>
#include "ObjectBaseV2.h"
#include "MessageV2.h"

class PostOfficeV2 : public Singleton<PostOfficeV2>
{
	friend Singleton<PostOfficeV2>;

public:
	void Register(const std::string &address, ObjectBaseV2 *object);
	bool Send(const std::string &address, MessageV2 *message);

	//void BroadCast(Message *message);

private:
	PostOfficeV2();
	~PostOfficeV2();
	std::map<std::string, ObjectBaseV2*> m_addressBook;
};