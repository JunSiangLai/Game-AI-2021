
#include "GameObject.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue) 
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	sm(nullptr),
	fishCount(0)
{
}

GameObject::~GameObject()
{
	if (sm != nullptr)
	{
		delete sm;
		sm = nullptr;
	}
}

void GameObject::Update(double dt)
{
	
}