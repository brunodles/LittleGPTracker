#include "Config.h"

/**
 * Compares two strings for equality.
 * @param key The first string to compare.
 * @param value The second string to compare.
 * @param caseSensitive Whether the comparison should be case-sensitive
 * (default: false for case-insensitive).
 * @return true if both strings are equal, false otherwise. Returns false if
 * either string is null.
 */
inline bool isKeyEqualTo(const char* key, const char* value, bool caseSensitive = false) {
	if (!key || !value) return false;
	if (caseSensitive) {
		return strcmp(key, value) == 0;
	}
	return strcasecmp(key, value) == 0;
}

/**
 * Converts a string to a boolean value.
 * @param value The string to convert. Can be null or empty.
 * @return true if value starts with 'Y', 'y', 'T', or 't', or equals "YES" or
 * "TRUE" (case-insensitive). false otherwise.
 */
inline bool strToBool(const char* value) {
    if (!value) return false;
	char firstChar = tolower(value[0]);
	if (firstChar == '1' || firstChar == 'y' || firstChar == 't') return true;
    if (strcasecmp(value, "yes") == 0) return true;
    if (strcasecmp(value, "true") == 0) return true;
    return false;
}

/**
 * Converts a string to an integer value.
 * @param value The string to convert. Can be null or empty.
 * @return The integer value parsed from the string using atoi().
 *         Returns 0 if the string is null, empty, or not a valid integer.
 */
inline int strToInt(const char* value) {
	return atoi(value);
}

Config::Config() {
    // initialize default values

	// Screen
	fullscreen = false;
	screenMultiply = -1;
	// Project
	volume = -1;
    projectAutoLoadEnabled = true;
	// Sample Configs
	sampleChunkSize = -1;
	// Wave File
	wavePreListenAttenuation = 1;
	// Midi
    midiDelay = -1;
	// Wip Key/input Config
	inputKeyDelay = -1;
	inputKeyRepeat = -1;
	inputKeyInvertTriggers = false;
	// Theme
	altRowNumber = 4;
	majorBeatNumber = 4;
    isColumnTitleEnabled = false;
	// log
	dumpEvent = false;

	// load from config file

    Path path("bin:config.xml") ;
	Trace::Log("CONFIG","Got config path=%s",path.GetPath().c_str()) ;
	TiXmlDocument *document=new TiXmlDocument(path.GetPath());
    bool loadOkay = document->LoadFile();

    if (loadOkay) {
        // Check first node is CONFIG/ GPCONFIG

		TiXmlNode* rootnode = 0;

        rootnode = document->FirstChild("CONFIG");
        if (!rootnode) {
            rootnode = document->FirstChild("GPCONFIG");
        }

        if (rootnode) {
			TiXmlElement *rootelement = rootnode->ToElement();
			TiXmlNode *node = rootelement->FirstChildElement() ;

            // Loop on all children

            if (node) {
                TiXmlElement *element = node->ToElement();
                while (element) {
                    const char *key = element->Value();
                    const char *value=element->Attribute("value") ;
                    if (!value) {
                        value=element->Attribute("VALUE") ;
                    }
                    if (key && value) {
                        if (isKeyEqualTo(key, "VOLUME")) {
                            volume = strToInt(value);
                        } else if (isKeyEqualTo(key, "ROOTFOLDER")) {
                            rootPath = (char*)value;
                        } else if (isKeyEqualTo(key, "AUTO_LOAD_LAST")) {
                            projectAutoLoadEnabled = strToBool(value);
                        } else if (isKeyEqualTo(key, "SAMPLELIB")) {
                            sampleLibPath = (char*)value;
                        } else if (isKeyEqualTo(key, "SAMPLELOADCHUNKSIZE")) {
                            sampleChunkSize = strToInt(value);
                        } else if (isKeyEqualTo(key, "PRELISTENATTENUATION")) {
                            wavePreListenAttenuation = strToBool(value);
                        } else if (isKeyEqualTo(key, "LEGACYDOWNSAMPLING")) {
                            waveLegacyDownSampling = strToBool(value);
                        } else if (isKeyEqualTo(key, "MIDICTRLDEVICE")) {
                            midiControlDevice = (char*)value;
                        } else if (isKeyEqualTo(key, "MIDIDELAY")) {
                            midiDelay = strToInt(value);
                        } else if (isKeyEqualTo(key, "MIDISENDSYNC")) {
                            midiSendSync = strToBool(value);
                        } else if (isKeyEqualTo(key, "AUDIOAPI")) {
                            audioApi = (char*)value;
                        } else if (isKeyEqualTo(key, "AUDIODEVICE")) {
                            audioDevice = (char*)value;
                        } else if (isKeyEqualTo(key, "AUDIOBUFFERSIZE")) {
                            audioBufferSize = strToInt(value);
                        } else if (isKeyEqualTo(key, "AUDIOPREBUFFERCOUNT")) {
                            audioPreBufferCount = strToInt(value);
                        } else if (isKeyEqualTo(key, "KEYDELAY")) {
                            inputKeyDelay = strToInt(value);
                        } else if (isKeyEqualTo(key, "KEYREPEAT")) {
                            inputKeyRepeat = strToInt(value);
                        } else if (isKeyEqualTo(key, "INVERT")) {
                            inputKeyInvertTriggers = strToBool(value);
                        } else if (isKeyEqualTo(key, "ALTROWNUMBER")) {
                            altRowNumber = strToInt(value);
                        } else if (isKeyEqualTo(key, "MAJORBEATNUMBER")) {
                            majorBeatNumber = strToInt(value);
                        } else if (isKeyEqualTo(key, "FONTTYPE")) {
                            fontType = (char*)value;
                        } else if (isKeyEqualTo(key, "SHOW_COLUMN_TITLES")) {
                            isColumnTitleEnabled = strToBool(value);
                        } else if (isKeyEqualTo(key, "DUMPEVENT")) {
                            dumpEvent = strToBool(value);
                        } else if (isKeyEqualTo(key, "FULLSCREEN")) {
                            fullscreen = strToBool(value);
                        } else if (isKeyEqualTo(key, "SCREENMULT")) {
                            screenMultiply = strToInt(value);

#ifdef PLATFORM_CAANOO
						} else if (isKeyEqualTo(key, "CAANOO_DSP")) {
							caanooDsp = (char*)value;
						} else if (isKeyEqualTo(key, "CAANOO_MIXER")) {
							caanooMixer = (char*)value;
						} else if (isKeyEqualTo(key, "CAANOO_MIDIDEVICE")) {
							caanooMidiDevice = (char*)value;
#endif
#ifdef PLATFORM_GP2X
						} else if (isKeyEqualTo(key, "GP2X_DSP")) {
							gp2xDsp = (char*)value;
						} else if (isKeyEqualTo(key, "GP2X_MIXER")) {
							gp2xMixer = (char*)value;
#endif								
                        } else {
                            Variable *v=new Variable(key,0,value) ;
							Insert(v) ;
                        }
                    }
                    element = element->NextSiblingElement();
                }
            }
    }
    } else {
        Trace::Log("CONFIG", "No (bad?) config.xml");
    }
    delete(document) ;
}

//------------------------------------------------------------------------------

Config::~Config() {}

//------------------------------------------------------------------------------

const char *Config::GetValue(const char *key) 
{
	Variable *v=FindVariable(key) ;
	if (v) {
		Trace::Log("CONFIG","Got value for %s=%s",key,v->GetString()) ;
	}
	return v?v->GetString():0 ;
} ;


//------------------------------------------------------------------------------

void Config::ProcessArguments(int argc,char **argv) 
{
	for (int i=1;i<argc;i++) {
		char *pos ;
		char *arg=argv[i] ;
		while (*arg=='-') arg++ ;
		if ((pos=strchr(arg,'='))!=0) {
			*pos=0 ;
			Variable *v=FindVariable(arg) ;
			if (v) {
				v->SetString(pos+1) ;
			} else {
				Variable *v=new Variable(arg,0,pos+1) ;
				Insert(v) ;
			}
		}
	}
} ;

//------------------------------------------------------------------------------
