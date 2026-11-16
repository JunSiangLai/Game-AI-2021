#pragma once

#include "GameObject.h"
#include <string>

struct MessageV2
{
	enum MSG_TYPE
	{
		MSG_NONE = 0,

		// Game Objects
		MSG_NEARESTPLAYER,
		MSG_NEARDETONATOR,
		MSG_NEARESTENEMY,
		MSG_ENGINEERLOCATION,
		MSG_NEARESTORE,
		MSG_TURRET,
		MSG_DMG,
		MSG_EXPLOSION,
		MSG_DROPPOD,
		MSG_CRYSTALMINED,

		// Scene
		MSG_WAVE,

		MSG_TOTAL,
	};
	MessageV2(GameObjectV2* goValue, MSG_TYPE typeValue, float thresholdValue, Vector3 pos, bool isTrue)
		: go(goValue), type(typeValue), threshold(thresholdValue), pos(pos), isTrue(isTrue) {}
	virtual ~MessageV2() {}

	GameObjectV2* go;
	MSG_TYPE type;
	float threshold;
	Vector3 pos;
	bool isTrue;
};