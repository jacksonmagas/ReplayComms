#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "portaudio.h"

#include "version.h"
#include <ThreadedFileWriter.hpp>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class ReplayComms: public BakkesMod::Plugin::BakkesModPlugin
	//,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{
	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad() override;
	virtual void onUnload() override; // Uncomment and implement if you need a unload method
	void startRecording();
	void stopRecording();
	std::unique_ptr<ThreadedFileWriter> fileWriter;
	std::filesystem::path audio_file_dir;

	bool isRecording;
	
	int myMemberCallback(const void *input, 
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags);
	
	static int myPaCallback(const void *input,
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData ) {
	 return ((ReplayComms*)userData)->myMemberCallback(input, output, frameCount, timeInfo, statusFlags);
}
public:
	//void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
