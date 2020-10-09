/**********************************************************************
  Audacity: A Digital Audio Editor
  NoiseReduction.h
  Dominic Mazzoni
  Vaughan Johnson (Preview)
  Paul Licameli
**********************************************************************/
#pragma once

#include <memory>
#include "Utils.h"
#include "BufferedInputTrack.h"
#include "BufferedOutputTrack.h"

#define DB_TO_LINEAR(x) (pow(10.0, (x) / 20.0))
#define LINEAR_TO_DB(x) (20.0 * log10(x))

typedef char *samplePtr;

class BufferedNoiseReductionWorker;
class BufferedStatistics;
class BufferedNoiseReduction {
public:
    struct Settings {
        Settings();

        size_t WindowSize() const { return 1u << (3 + mWindowSizeChoice); }
        unsigned StepsPerWindow() const { return 1u << (1 + mStepsPerWindowChoice); }
        bool       mDoProfile;
        double     mNewSensitivity;   // - log10 of a probability... yeah.
        double     mFreqSmoothingBands; // really an integer
        double     mNoiseGain;         // in dB, positive
        double     mAttackTime;        // in secs
        double     mReleaseTime;       // in secs

        // Advanced:
        double     mOldSensitivity;    // in dB, plus or minus

        // Basic:
        int        mNoiseReductionChoice;

        // Advanced:
        int        mWindowTypes;
        int        mWindowSizeChoice;
        int        mStepsPerWindowChoice;
        int        mMethod;
    };

    BufferedNoiseReduction(BufferedNoiseReduction::Settings& settings, double sampleRate);
    void ProfileNoise(BufferedInputTrack& inputTrack);
    void ReduceNoise(BufferedInputTrack& inputTrack, BufferedOutputTrack &outputTrack);
private:
    std::unique_ptr<BufferedStatistics> mStatistics;
    BufferedNoiseReduction::Settings mSettings;
    double mSampleRate;
};
