#include "PostOfficeV2.h"

void PostOfficeV2::Register(const std::string & address, ObjectBaseV2 * object)
{
	if (!object)
		return;
	if (m_addressBook.find(address) != m_addressBook.end())
		return;
	m_addressBook.insert(std::pair<std::string, ObjectBaseV2*>(address, object));
}

bool PostOfficeV2::Send(const std::string & address, MessageV2 * message)
{
	if (!message)
		return false;
	std::map<std::string, ObjectBaseV2*>::iterator it = m_addressBook.find(address);
	if (m_addressBook.find(address) == m_addressBook.end())
	{
		delete message;
		return false;
	}
	ObjectBaseV2 *object = (ObjectBaseV2*)it->second;
	return object->Handle(message);
}

PostOfficeV2::PostOfficeV2()
{
}

PostOfficeV2::~PostOfficeV2()
{
	m_addressBook.clear();
}
