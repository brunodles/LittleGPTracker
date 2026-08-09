
#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_

#include "System/FileSystem/FileSystem.h"
#include "AudioFile.h"

// Why the last WavFile::Open() call failed. The import dialog surfaces
// this to the user instead of the generic "failed to import sample".
enum WavFileError {
	WAVERR_NONE = 0,
	WAVERR_NOT_RIFF,       // file does not start with RIFF
	WAVERR_NOT_WAVE,       // RIFF container is not a WAVE file
	WAVERR_BAD_FMT_CHUNK,  // no fmt chunk found
	WAVERR_BAD_FMT_SIZE,   // fmt chunk smaller than 16 bytes
	WAVERR_UNSUPPORTED_COMPRESSION, // compression code != 1 (float/ADPCM/...)
	WAVERR_UNSUPPORTED_BIT_DEPTH,   // bit depth not 8 or 16
};

class WavFile:public AudioFile {

protected: // Factory - see Load method
	WavFile(I_File *file) ;
public:
	virtual ~WavFile() ;
	static WavFile *Open(const char *) ;
	virtual void *GetSampleBuffer(int note) ;
	virtual int GetSize(int note) ;
	virtual int GetSampleRate(int note) ;
	virtual int GetChannelCount(int note) ;
	virtual int GetRootNote(int note) ;
	bool GetBuffer(long start,long sampleCount) ; // values in smples
	void Close() ;
	virtual bool IsMulti() {return false ; } ;
	// Reason the most recent Open() call failed. Meaningful only when
	// Open() returned 0.
	static WavFileError GetLastError();

protected:
	long readBlock(long position,long count) ;
private:
	I_File *file_ ;  // File
	void *readBuffer_ ; // Temp read buffer
	int readBufferSize_; // Read buffer size
	short *samples_ ; // sample buffer size (16 bits)
	int sampleBufferSize_ ;
	int size_ ; // number of samples
	int sampleRate_ ; // sample rate
	int channelCount_ ; // mono / stereo
	int bytePerSample_ ; // original file is in 8/16bit
	int dataPosition_ ; // offset in file to get to data

	static int bufferChunkSize_ ;
	static bool initChunkSize_ ;
	static WavFileError lastError_ ;
} ;
#endif
