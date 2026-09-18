#ifndef _PROJECT_H_
#define _PROJECT_H_

#include "Song.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Persistency/Persistent.h"
#include "Foundation/Variables/VariableContainer.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Observable.h"

#define VAR_TEMPO MAKE_FOURCC('T', 'M', 'P', 'O')
#define VAR_MASTERVOL   	MAKE_FOURCC('M', 'S', 'T', 'R')
#define VAR_WRAP        	MAKE_FOURCC('W', 'R', 'A', 'P')
#define VAR_MIDIDEVICE  	MAKE_FOURCC('M', 'I', 'D', 'I')
#define VAR_TRANSPOSE   	MAKE_FOURCC('T', 'R', 'S', 'P')
#define VAR_SOFTCLIP 		MAKE_FOURCC('S', 'F', 'T', 'C')
#define VAR_SOFTCLIP_GAIN 	MAKE_FOURCC('S', 'F', 'G', 'N')
#define VAR_PREGAIN   		MAKE_FOURCC('P', 'R', 'G', 'N')
#define VAR_SCALE 			MAKE_FOURCC('S', 'C', 'A', 'L')
#define VAR_RENDER MAKE_FOURCC('R', 'N', 'D', 'R')

#define PROJECT_NUMBER "1"
#define PROJECT_RELEASE "6"
#define BUILD_COUNT "0-bacon20"

#define MAX_TAP 3
#define EQ_PRESET_BANDS 6
#define MAX_EQ_PRESET_COUNT 32
#define EQ_PRESET_NAME_MAX 16

struct EqPresetBand {
  unsigned short frequency;
  unsigned short gainQ;
};

struct EqPreset {
  bool used;
  unsigned short id;
  char name[EQ_PRESET_NAME_MAX + 1];
  EqPresetBand bands[EQ_PRESET_BANDS];
};

class Project: public Persistent,public VariableContainer,I_Observer  {
public:
  Project();
  ~Project();
  void Purge();
  void PurgeInstruments(bool removeFromDisk);

  Song *song_;

  int GetMasterVolume();
  bool Wrap();
  void OnTempoTap();
  void NudgeTempo(int value);
  int GetScale();
  int GetTempo(); // Takes nudging into account
  int GetTranspose();
  int GetSoftclip();
  int GetSoftclipGain();
  int GetPregain();
  int GetRenderMode();
  void Trigger();

  static const unsigned int MAX_RENDER_MODE = 3;
  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

  InstrumentBank *GetInstrumentBank();

  int GetEqPresetCount() const;
  bool GetEqPreset(int index, EqPreset &out) const;
  bool SetEqPreset(int index, const EqPreset &preset);
  bool RemoveEqPreset(int index);
  bool RenameEqPreset(int index, const char *name);
  int FindEqPresetById(unsigned short id) const;
  unsigned short AllocateEqPresetId();

  virtual void SaveContent(TiXmlNode *node);
  virtual void RestoreContent(TiXmlElement *element);

  void LoadFirstGen(const char *root);

protected:
  void buildMidiDeviceList();

private:
  InstrumentBank *instrumentBank_;
  char **midiDeviceList_;
  int midiDeviceListSize_;
  int tempoNudge_;
  unsigned long lastTap_[MAX_TAP];
  unsigned int tempoTapCount_;

  EqPreset eqPresets_[MAX_EQ_PRESET_COUNT];
  unsigned short nextEqPresetId_;
};
#endif
