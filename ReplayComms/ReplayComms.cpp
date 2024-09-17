#include "pch.h"
#include "ReplayComms.h"


BAKKESMOD_PLUGIN(ReplayComms, "Replay Audio", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void ReplayComms::onLoad()
{
	_globalCvarManager = cvarManager;

	audio_file_path = gameWrapper->GetBakkesModPath() / "replayComms";

	//LOG("Plugin loaded!");
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	DEBUGLOG("Loading");

	// initialize portaudio
	auto err = Pa_Initialize();
	if (err != paNoError) {
		DEBUGLOG("PortAudio error: {}", Pa_GetErrorText(err));
	}

	isRecording = false;
	//every time an online game starts start recording audio
	gameWrapper->HookEvent("Function TAGame.Team_TA.PostBeginPlay", std::bind(&ReplayComms::startRecording, this));
	//when a game ends stop recording
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&ReplayComms::stopRecording, this));
}

void ReplayComms::onUnload()
{
	auto err = Pa_Terminate();
	if (err != paNoError) {
		DEBUGLOG("PortAudio error: {}", Pa_GetErrorText(err));
	}
	DEBUGLOG("Unloading");
}


void ReplayComms::startRecording()
{
	if (isRecording) {
		return;
	}
	isRecording = true;
	DEBUGLOG("Recording Audio");

	numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.5 * NUM_CHANNELS));
	numBytes = numSamples * sizeof(SAMPLE);
	ThreadedFileWriter fileWriter(audio_file_dir / filename, numSamples * sizeof(SAMPLE), 3);
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