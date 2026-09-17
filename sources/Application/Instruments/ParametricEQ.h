#ifndef _PARAMETRIC_EQ_H_
#define _PARAMETRIC_EQ_H_

#include <math.h>

namespace EqUtils {

static const int kEqBandCount = 6;

inline float DecodeFrequency(unsigned short value) {
    float norm = float(value) / 65535.0f;
    return 20.0f * powf(1000.0f, norm);
}

inline float DecodeGainDb(unsigned char value) {
    return -24.0f + (48.0f * float(value) / 255.0f);
}

inline float DecodeQ(unsigned char value) {
    return 0.25f + (11.75f * float(value) / 255.0f);
}

inline void DecodeGainQ(unsigned short packed, float &gainDb, float &q) {
    gainDb = DecodeGainDb((unsigned char)((packed >> 8) & 0xFF));
    q = DecodeQ((unsigned char)(packed & 0xFF));
}

inline void DecodeGainQ(unsigned short packed, float *gainDb, float *q) {
    float gain = 0.0f;
    float quality = 0.0f;
    DecodeGainQ(packed, gain, quality);
    if (gainDb) {
        *gainDb = gain;
    }
    if (q) {
        *q = quality;
    }
}

class ParametricEQ {
public:
    ParametricEQ() {
        Reset(44100.0f);
    }

    void Reset(float sampleRate) {
        sampleRate_ = (sampleRate > 0.0f) ? sampleRate : 44100.0f;
        for (int i = 0; i < kEqBandCount; ++i) {
            bands_[i].Reset();
        }
    }

    void SetBand(int band, float frequency, float gainDb, float q) {
        if ((band < 0) || (band >= kEqBandCount)) {
            return;
        }
        bands_[band].SetPeaking(frequency, gainDb, q, sampleRate_);
    }

    float Process(float sample) {
        for (int i = 0; i < kEqBandCount; ++i) {
            sample = bands_[i].Process(sample);
        }
        return sample;
    }

private:
    class Biquad {
    public:
        Biquad() { Reset(); }

        void Reset() {
            b0_ = 1.0f;
            b1_ = 0.0f;
            b2_ = 0.0f;
            a1_ = 0.0f;
            a2_ = 0.0f;
            z1_ = 0.0f;
            z2_ = 0.0f;
        }

        void SetPeaking(float frequency, float gainDb, float q, float sampleRate) {
            if (sampleRate <= 0.0f) {
                sampleRate = 44100.0f;
            }
            if (frequency < 20.0f) {
                frequency = 20.0f;
            }
            float nyquist = sampleRate * 0.5f;
            if (frequency > nyquist - 1.0f) {
                frequency = nyquist - 1.0f;
            }
            if (q < 0.1f) {
                q = 0.1f;
            }

            const float pi = 3.14159265358979323846f;
            float omega = 2.0f * pi * frequency / sampleRate;
            float sinOmega = sinf(omega);
            float cosOmega = cosf(omega);
            float alpha = sinOmega / (2.0f * q);
            float a = powf(10.0f, gainDb / 40.0f);

            float b0 = 1.0f + alpha * a;
            float b1 = -2.0f * cosOmega;
            float b2 = 1.0f - alpha * a;
            float a0 = 1.0f + alpha / a;
            float a1 = -2.0f * cosOmega;
            float a2 = 1.0f - alpha / a;

            b0_ = b0 / a0;
            b1_ = b1 / a0;
            b2_ = b2 / a0;
            a1_ = a1 / a0;
            a2_ = a2 / a0;
        }

        float Process(float sample) {
            float out = (b0_ * sample) + z1_;
            z1_ = (b1_ * sample) - (a1_ * out) + z2_;
            z2_ = (b2_ * sample) - (a2_ * out);
            return out;
        }

    private:
        float b0_;
        float b1_;
        float b2_;
        float a1_;
        float a2_;
        float z1_;
        float z2_;
    };

    class Band {
    public:
        Band() {
            Reset();
        }

        void Reset() {
            biquad_.Reset();
        }

        void SetPeaking(float frequency, float gainDb, float q, float sampleRate) {
            biquad_.SetPeaking(frequency, gainDb, q, sampleRate);
        }

        float Process(float sample) {
            return biquad_.Process(sample);
        }

    private:
        Biquad biquad_;
    };

    float sampleRate_;
    Band bands_[kEqBandCount];
};

} // namespace EqUtils

#endif
