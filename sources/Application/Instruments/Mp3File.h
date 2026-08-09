#ifndef _MP3_FILE_H_
#define _MP3_FILE_H_

#include "AudioFile.h"

// Why the last Mp3File::Open() call failed. Mirrors WavFileError so the
// import dialog can surface a specific reason instead of a generic one.
enum Mp3FileError {
	MP3ERR_NONE = 0,
	MP3ERR_OPEN_FAILED,     // file could not be opened
	MP3ERR_READ_FAILED,     // file could not be read
	MP3ERR_DECODE_FAILED,   // minimp3 rejected the stream
	MP3ERR_OUT_OF_MEMORY,   // could not allocate sample buffer
} ;

class Mp3File:public AudioFile {

protected:
	Mp3File() ;
public:
	virtual ~Mp3File() ;
	static Mp3File *Open(const char *) ;
	virtual void *GetSampleBuffer(int note) ;
	virtual int GetSize(int note) ;
	virtual int GetSampleRate(int note) ;
	virtual int GetChannelCount(int note) ;
	virtual int GetRootNote(int note) ;
	bool GetBuffer(long start,long sampleCount) ; // values in samples
	void Close() ;
	virtual bool IsMulti() {return false ; } ;

	// Reason the most recent Open() call failed. Meaningful only when
	// Open() returned 0.
	static Mp3FileError GetLastError();

protected:
	short *samples_ ; // decoded PCM16 samples (interleaved)
	int size_ ; // number of samples per channel
	int sampleRate_ ; // sample rate
	int channelCount_ ; // mono / stereo
	long readPosition_ ; // offset used by GetBuffer/GetSampleBuffer

	static Mp3FileError lastError_ ;
} ;
#endif
