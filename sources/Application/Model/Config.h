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
    bool fullscreen;
    // *key: SCREENMULT
    int screenMultiply;

    // Project
    // *key: VOLUME
    int volume;
    // *key: ROOTFOLDER
	char* rootPath;
	// *key: AUTO_LOAD_LAST
    bool projectAutoLoadEnabled;

    // Sample Configs
    // *key: SAMPLELIB
    char *sampleLibPath;
	// *key: SAMPLELOADCHUNKSIZE
    int sampleChunkSize;

    // Wave File
    // *key: PRELISTENATTENUATION
    bool wavePreListenAttenuation;
    // *key: LEGACYDOWNSAMPLING
    bool waveLegacyDownSampling;

	// Midi
	// *key: MIDICTRLDEVICE
  	char *midiControlDevice;
	// *key: MIDIDELAY
    int midiDelay;
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
    int inputKeyDelay;
    // *key: KEYREPEAT
    int inputKeyRepeat;
    /** Invert trigger keys  in some consoles */
	// *key: INVERT
    bool inputKeyInvertTriggers;

    // Theme
    // *key: ALTROWNUMBER
    int altRowNumber;
    // *key: MAJORBEATNUMBER
    int majorBeatNumber;
    // *key: FONTTYPE
	char *fontType;

	/** Whether to show column titles in song, chain and phrase views */
    // *key: SHOW_COLUMN_TITLES,
    bool isColumnTitleEnabled;

    // Logs
    // *key: DUMPEVENT
    bool dumpEvent;

#ifdef PLATFORM_CAANOO
    // CAANOO specific
    // *key: CAANOO_DSP
    char *caanooDsp;
    // *key: CAANOO_MIXER
    char *caanooMixer;
    // *key: CAANOO_MIDIDEVICE
    char *caanooMidiDevice;
#endif

#ifdef PLATFORM_GP2X
    // GP2X specific
    // *key: GP2X_DSP
    char *gp2xDsp;
    // *key: GP2X_MIXER
    char *gp2xMixer;
#endif

};

#endif
