#pragma once

enum EventType
{
	EVENT_ENTER,
	EVENT_PICKUP,
	EVENT_UPDATE,
	EVENT_TIMEOUT,
	EVENT_ENCOUNTER,
	EVENT_DIE
};

struct Event
{
	EventType type;
	Quest_Scripted* quest;
};
