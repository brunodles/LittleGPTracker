#ifndef _AUDIO_FILE_H_
#define _AUDIO_FILE_H_

#include "SoundSource.h"

// Common interface for file-backed sample sources (WAV, MP3).
//
// SoundSource is the contract the engine consumes (render, slicing, loops).
// AudioFile adds the two file-oriented operations the load path and the
// preview streamer need on top of it:
//
//   - GetBuffer(start, sampleCount): make the samples in [start, start+count)
//     available through GetSampleBuffer(). WavFile reads them from disk on
//     demand; Mp3File decodes the whole file up front and treats this as a
//     no-op.
//   - Close(): release the file handle / decoder state once the sample has
//     been fully buffered.
class AudioFile : public SoundSource {
public:
	AudioFile() {} ;
	virtual ~AudioFile() {} ;
	virtual bool GetBuffer(long start, long sampleCount) = 0 ;
	virtual void Close() = 0 ;
} ;

#endif
