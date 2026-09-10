#ifndef _VOWEL_FILTER_H_
#define _VOWEL_FILTER_H_

#include <math.h>

class VowelFilter {
public:
    VowelFilter() {
        Reset(44100.0f);
    }

    void Reset(float sampleRate) {
        sampleRate_ = (sampleRate > 0.0f) ? sampleRate : 44100.0f;
        phaseByte_ = 0;
        amount_ = 0.0f;
        enabled_ = false;
        band1_.Reset();
        band2_.Reset();
    }

    void SetValue(unsigned short value) {
        unsigned char phase = (unsigned char)((value >> 8) & 0xFF);
        unsigned char amount = (unsigned char)(value & 0xFF);

        amount_ = float(amount) / 255.0f;
        if (phase == 0 || amount == 0) {
            enabled_ = false;
            band1_.Reset();
            band2_.Reset();
            return;
        }

        phase = normalizePhase(phase);
        if (!enabled_ || phase != phaseByte_) {
            phaseByte_ = phase;
            updateCoefficients(phase);
        }
        enabled_ = true;
    }

    bool Enabled() const {
        return enabled_;
    }

    float Process(float sample) {
        if (!enabled_) {
            return sample;
        }

        float wet = (band1_.Process(sample) + band2_.Process(sample)) * 0.5f * 1.8f;
        return sample * (1.0f - amount_) + (wet * amount_);
    }

private:
    struct Biquad {
        float b0_;
        float b1_;
        float b2_;
        float a1_;
        float a2_;
        float z1_;
        float z2_;

        Biquad() {
            Reset();
        }

        void Reset() {
            b0_ = 1.0f;
            b1_ = 0.0f;
            b2_ = 0.0f;
            a1_ = 0.0f;
            a2_ = 0.0f;
            z1_ = 0.0f;
            z2_ = 0.0f;
        }

        void SetBandpass(float frequency, float q, float sampleRate) {
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
            float a0 = 1.0f + alpha;

            b0_ = alpha / a0;
            b1_ = 0.0f;
            b2_ = -alpha / a0;
            a1_ = (-2.0f * cosOmega) / a0;
            a2_ = (1.0f - alpha) / a0;
        }

        float Process(float sample) {
            float out = b0_ * sample + z1_;
            z1_ = b1_ * sample - a1_ * out + z2_;
            z2_ = b2_ * sample - a2_ * out;
            return out;
        }
    };

    static unsigned char normalizePhase(unsigned char phase) {
        if (phase < 0x10) {
            return 0x10;
        }
        if (phase > 0x60) {
            return 0x60;
        }
        return phase;
    }

    void updateCoefficients(unsigned char phase) {
        static const float kFormantF1[5] = {730.0f, 530.0f, 270.0f, 520.0f, 300.0f};
        static const float kFormantF2[5] = {1090.0f, 1840.0f, 2290.0f, 1190.0f, 870.0f};
        static const float kQ1 = 8.0f;
        static const float kQ2 = 10.0f;

        float f1 = kFormantF1[0];
        float f2 = kFormantF2[0];

        if (phase < 0x60) {
            int segment = (int)(phase - 0x10) / 0x10;
            int step = (int)(phase - 0x10) % 0x10;
            float t = float(step) / 16.0f;
            int next = (segment + 1) % 5;
            f1 = lerp(kFormantF1[segment], kFormantF1[next], t);
            f2 = lerp(kFormantF2[segment], kFormantF2[next], t);
        }

        band1_.SetBandpass(f1, kQ1, sampleRate_);
        band2_.SetBandpass(f2, kQ2, sampleRate_);
    }

    static float lerp(float a, float b, float t) {
        return a + ((b - a) * t);
    }


    float sampleRate_;
    unsigned char phaseByte_;
    float amount_;
    bool enabled_;
    Biquad band1_;
    Biquad band2_;
};

#endif
