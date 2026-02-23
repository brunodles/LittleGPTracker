#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "System/Console/Trace.h"
#include "Externals/TinyXML/tinyxml.h"

/**
 * Return the first value if it is bigger than or equal to minValue(inclusive),
 * the second one otherwise
 *
 * Eg.:
 * * If firstValue=1 and minValue=0, it will return the firstValue.
 * * If firstValue=1 and minValue=1, it will return the firstValue.
 * * if firstValue=0 and minValue=1, it will return the fallbackValue.
 * * If firstValue=-1 and minValue=0, it will return the fallbackValue.
 */
inline int intConfOrFallback(int firstValue, int fallbackValue, int minValue = 0) {
	return firstValue >= minValue ? firstValue : fallbackValue;
}

class Config: public T_Singleton<Config>,public VariableContainer {
  public:
    Config();
	~Config();
	const char *GetValue(const char *key);
	void ProcessArguments(int argc, char **argv);

	// Screen
	// *key: FULLSCREEN
	bool fullscreen = false;
	// *key: SCREENMULT
	int screenMultiply = -1;

	// Project
	// *key: VOLUME
	int volume = -1;
	// *key: ROOTFOLDER
	char* rootPath;
	// *key: AUTO_LOAD_LAST
    bool projectAutoLoadEnabled = true;

	// Sample Configs
	// *key: SAMPLELIB
	char *sampleLibPath;
	// *key: SAMPLELOADCHUNKSIZE
	int sampleChunkSize = -1;

	// Wave File
	// *key: PRELISTENATTENUATION
	bool wavePreListenAttenuation = 1;
	// *key: LEGACYDOWNSAMPLING
    bool waveLegacyDownSampling;

	// Midi
	// *key: MIDICTRLDEVICE
  	char *midiControlDevice;
	// *key: MIDIDELAY
    int midiDelay = -1;
	// *key: MIDISENDSYNC
	bool midiSendSync;

	// Audio Configs
	// *key: AUDIOAPI
	char *audioApi;
	// *key: AUDIODEVICE
	char *audioDevice;
	// *key: AUDIOBUFFERSIZE
	int audioBufferSize;
	// *key: AUDIOPREBUFFERCOUNT
	int audioPreBufferCount;
				
	// Wip Key/input Config
	// *key: KEYDELAY
	int inputKeyDelay = -1;
	// *key: KEYREPEAT
	int inputKeyRepeat = -1;
	/** Invert trigger keys  in some consoles */
	// *key: INVERT
	bool inputKeyInvertTriggers = false;

	// Theme
	// *key: ALTROWNUMBER
	int altRowNumber = 4;
	// *key: MAJORBEATNUMBER
	int majorBeatNumber = 4;
	// *key: FONTTYPE
	char *fontType;

	/** Whether to show column titles in song, chain and phrase views */
	// *key: SHOW_COLUMN_TITLES, 
    bool isColumnTitleEnabled = false;

	// Logs
	// *key: DUMPEVENT
	bool dumpEvent = false;

};

#endif
