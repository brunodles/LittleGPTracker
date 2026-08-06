#include "PitchDetector.h"

#include <math.h>

// ---------------------------------------------------------------------------
// Pitch range limits (Hz). The detector is designed for musical samples, not
// sub-bass or ultrasound.
// ---------------------------------------------------------------------------

static const float kMinPitchHz = 30.0f;
static const float kMaxPitchHz = 2000.0f;

// Minimum signal amplitude (16-bit) to consider a region "audible". Very quiet
// regions are noise, not pitch.
static const int kMinAmplitude = 64;

// Autocorrelation window cap: we never need more than this many samples to
// find a period, no matter how long the region is.
static const int kMaxAnalysisSamples = 4096;

// Minimum correlation (normalized) to accept a lag as a real period.
static const float kMinCorrelation = 0.35f;

// ---------------------------------------------------------------------------

static inline float AbsSample(const short *samples,
                              int index,
                              int channelCount) {
    // Take the first channel (or average two when stereo) to build a mono view.
    if (channelCount == 1) {
        return (float)samples[index];
    }
    float sum = 0.0f;
    for (int c = 0; c < channelCount; c++) {
        sum += (float)samples[index * channelCount + c];
    }
    return sum / (float)channelCount;
}

float PitchDetector::FreqToMidi(float freq) {
    if (freq <= 0.0f) return 0.0f;
    return 69.0f + 12.0f * logf(freq / 440.0f) / logf(2.0f);
}

float PitchDetector::DetectPitch(const short *samples,
                                 int sampleCount,
                                 int sampleRate,
                                 int channelCount) {
    if (!samples || sampleCount < 8 || sampleRate <= 0 || channelCount < 1) {
        return 0.0f;
    }

    // Check the region is audible at all (peak amplitude).
    int peak = 0;
    for (int i = 0; i < sampleCount; i++) {
        int s = samples[i * channelCount]; // first channel is enough
        if (s < 0) s = -s;
        if (s > peak) peak = s;
    }
    if (peak < kMinAmplitude) {
        return 0.0f;
    }

    // Fast path: periodic regions (loops, one-cycle samples) are handled by
    // pure zero-crossing arithmetic, no DSP at all.
    float zc = DetectZeroCrossing(samples, sampleCount, sampleRate, channelCount);
    if (zc > 0.0f) {
        return zc;
    }

    // Fallback: autocorrelation on a capped window.
    return DetectAutocorrelation(samples, sampleCount, sampleRate, channelCount);
}

// ---------------------------------------------------------------------------
// Zero-crossing fast path
// ---------------------------------------------------------------------------
//
// Counts ascending zero crossings and the distance between them. If the
// distances are consistent (low variance), the signal is periodic and the
// period = mean distance. This is exact for one-cycle and N-cycle loops.
//
// Returns frequency in Hz, or 0.0f if the region is not clearly periodic.

float PitchDetector::DetectZeroCrossing(const short *samples,
                                        int sampleCount,
                                        int sampleRate,
                                        int channelCount) {
    // Collect ascending zero crossings: s[i-1] < 0 and s[i] >= 0.
    // We look at the first channel only.
    enum { kMaxCrossings = 64 };
    int crossings[kMaxCrossings];
    int crossingCount = 0;

    int prev = samples[0 * channelCount];
    for (int i = 1; i < sampleCount && crossingCount < kMaxCrossings; i++) {
        int cur = samples[i * channelCount];
        if (prev < 0 && cur >= 0) {
            crossings[crossingCount++] = i;
        }
        prev = cur;
    }

    // Need at least 3 crossings (2 full periods) to trust the average.
    if (crossingCount < 3) {
        return 0.0f;
    }

    // Compute distances between consecutive crossings.
    float minDist = 1e9f;
    float maxDist = 0.0f;
    float sumDist = 0.0f;
    int n = crossingCount - 1;
    for (int i = 0; i < n; i++) {
        float d = (float)(crossings[i + 1] - crossings[i]);
        sumDist += d;
        if (d < minDist) minDist = d;
        if (d > maxDist) maxDist = d;
    }
    float meanDist = sumDist / (float)n;

    // Period consistency check: if the distances are wildly different, the
    // region has multiple frequencies (chord, noise, transient). A periodic
    // waveform has distances within a small band of each other.
    if (meanDist <= 0.0f) return 0.0f;
    float spread = (maxDist - minDist) / meanDist;
    if (spread > 0.25f) {
        return 0.0f;
    }

    float freq = (float)sampleRate / meanDist;

    // Sanity: reject out-of-range pitch and periods that are just one sample
    // apart (near-DC noise).
    if (freq < kMinPitchHz || freq > kMaxPitchHz) {
        return 0.0f;
    }
    if (meanDist < 2.0f) {
        return 0.0f;
    }

    return freq;
}

// ---------------------------------------------------------------------------
// Autocorrelation fallback
// ---------------------------------------------------------------------------
//
// Computes the normalized autocorrelation over a capped analysis window and
// finds the lag with the strongest peak (excluding lag 0). Parabolic
// interpolation on the peak gives sub-sample period precision.
//
// Returns frequency in Hz, or 0.0f if no correlation peak is strong enough.

float PitchDetector::DetectAutocorrelation(const short *samples,
                                           int sampleCount,
                                           int sampleRate,
                                           int channelCount) {
    int window = sampleCount;
    if (window > kMaxAnalysisSamples) {
        window = kMaxAnalysisSamples;
    }

    // Build a mono float window and remove DC offset.
    // We analyze the whole window if it fits; otherwise take the first chunk.
    float *buf = new float[window];
    float sum = 0.0f;
    for (int i = 0; i < window; i++) {
        float s = AbsSample(samples, i, channelCount);
        buf[i] = s;
        sum += s;
    }
    float mean = sum / (float)window;
    for (int i = 0; i < window; i++) {
        buf[i] -= mean;
    }

    // Energy of the window (for normalization).
    float energy = 0.0f;
    for (int i = 0; i < window; i++) {
        energy += buf[i] * buf[i];
    }
    if (energy < 1.0f) {
        delete[] buf;
        return 0.0f;
    }

    // Lag range: from maxPitch (short period) to minPitch (long period).
    int minLag = (int)((float)sampleRate / kMaxPitchHz);
    int maxLag = (int)((float)sampleRate / kMinPitchHz);
    if (minLag < 1) minLag = 1;
    if (maxLag >= window) maxLag = window - 1;
    if (maxLag <= minLag) {
        delete[] buf;
        return 0.0f;
    }

    // Find the lag with maximum normalized correlation (excluding lag 0).
    float bestCorr = 0.0f;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; lag++) {
        float corr = 0.0f;
        for (int i = 0; i + lag < window; i++) {
            corr += buf[i] * buf[i + lag];
        }
        corr /= energy;
        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    if (bestLag == 0 || bestCorr < kMinCorrelation) {
        delete[] buf;
        return 0.0f;
    }

    // A single period in the window is not enough to confirm periodicity:
    // the region must span at least two periods of the candidate pitch.
    if (bestLag * 2 > window) {
        delete[] buf;
        return 0.0f;
    }

    // Sharpness check: a real tone has a deep valley at lag/2 (anti-phase),
    // corr(lag/2) ~ -1 for a sine. A smooth single arc (one cycle, short
    // regions) has high correlation at every small lag and no valley — that
    // is how spurious short-lag peaks get rejected. This must run even when
    // bestLag/2 falls below minLag: for a 65 Hz arc the spurious peak sits at
    // minLag (22) and the valley at lag 11 is what exposes it.
    {
        int halfLag = bestLag / 2;
        if (halfLag < 1) halfLag = 1;
        float halfCorr = 0.0f;
        for (int i = 0; i + halfLag < window; i++) {
            halfCorr += buf[i] * buf[i + halfLag];
        }
        halfCorr /= energy;
        if (halfCorr > 0.5f * bestCorr) {
            delete[] buf;
            return 0.0f;
        }
    }

    // Parabolic interpolation around the peak for sub-sample precision.
    // corr(lag-1), corr(lag), corr(lag+1) -> vertex offset in [-0.5, 0.5].
    // We already have bestCorr for bestLag; compute the neighbors via a
    // second pass (cheap, single lag band).
    float c0 = 0.0f, c1 = bestCorr, c2 = 0.0f;
    {
        float energyInv = 1.0f / energy;
        if (bestLag > minLag) {
            float acc = 0.0f;
            int lag = bestLag - 1;
            for (int i = 0; i + lag < window; i++) {
                acc += buf[i] * buf[i + lag];
            }
            c0 = acc * energyInv;
        }
        if (bestLag < maxLag) {
            float acc = 0.0f;
            int lag = bestLag + 1;
            for (int i = 0; i + lag < window; i++) {
                acc += buf[i] * buf[i + lag];
            }
            c2 = acc * energyInv;
        }
    }

    delete[] buf;

    float denom = c0 - 2.0f * c1 + c2;
    float offset = 0.0f;
    if (fabsf(denom) > 1e-6f) {
        offset = 0.5f * (c0 - c2) / denom;
        if (offset > 0.5f) offset = 0.5f;
        if (offset < -0.5f) offset = -0.5f;
    }

    float period = (float)bestLag + offset;
    if (period < 1.0f) return 0.0f;

    float freq = (float)sampleRate / period;
    if (freq < kMinPitchHz || freq > kMaxPitchHz) {
        return 0.0f;
    }

    return freq;
}
