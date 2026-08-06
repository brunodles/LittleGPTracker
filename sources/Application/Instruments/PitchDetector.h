#ifndef _PITCH_DETECTOR_H_
#define _PITCH_DETECTOR_H_

// One-shot pitch detection for raw sample buffers.
//
// Strategy:
//  1. Zero-crossing fast path: if the region is clearly periodic (loop with
//     consistent period), derive the frequency arithmetically from the
//     average crossing distance. No DSP, cheap on GBA.
//  2. Autocorrelation fallback: capped window (2048-4096 samples), normalized
//     autocorrelation with parabolic peak interpolation for sub-sample lag.
//
// Returns the fundamental frequency in Hz, or 0.0f when no reliable pitch is
// found (region too short, noise, transient, etc). Callers must treat 0.0f as
// "unknown".

class PitchDetector {
public:
    // samples: interleaved 16-bit PCM as stored in the sample pool.
    // sampleCount: total number of samples (all channels).
    // sampleRate: native rate of the buffer (e.g. 44100).
    // channelCount: interleave factor (1 = mono, 2 = stereo).
    static float DetectPitch(const short *samples,
                             int sampleCount,
                             int sampleRate,
                             int channelCount);

    // Midify from a frequency in Hz: 69 + 12*log2(freq/440).
    static float FreqToMidi(float freq);

private:
    static float DetectZeroCrossing(const short *samples,
                                    int sampleCount,
                                    int sampleRate,
                                    int channelCount);
    static float DetectAutocorrelation(const short *samples,
                                       int sampleCount,
                                       int sampleRate,
                                       int channelCount);
};

#endif // _PITCH_DETECTOR_H_
