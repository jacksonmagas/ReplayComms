#include "pch.h"
#include "ReplayComms.h"


BAKKESMOD_PLUGIN(ReplayComms, "Replay Audio", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void ReplayComms::onLoad()
{
	_globalCvarManager = cvarManager;

	//LOG("Plugin loaded!");
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	DEBUGLOG("Loading");

	isRecording = false;
	//every time an online game starts start recording audio
	gameWrapper->HookEvent("Function TAGame.Team_TA.PostBeginPlay", std::bind(&ReplayComms::startRecording, this));
	//when a game ends stop recording
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&ReplayComms::stopRecording, this));
}

void ReplayComms::onUnload()
{
	DEBUGLOG("Unloading");
}


void ReplayComms::startRecording()
{
	if (isRecording) {
		return;
	}
	isRecording = true;
	DEBUGLOG("Recording Audio");
}


void ReplayComms::stopRecording()
{
	if (!isRecording) {
		DEBUGLOG("Recording not active");
		return;
	}
	isRecording = false;
	DEBUGLOG("Ending Recording");
}