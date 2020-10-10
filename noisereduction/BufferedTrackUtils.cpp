#include "BufferedTrackUtils.h"
#include "loguru.hpp"
#include <memory>

std::vector<BufferedInputTrack> BufferedTrackUtils::readTracksFromContext(const SndContext& ctx)
{
    std::vector<BufferedInputTrack> tracks;

    for (int channel = 0; channel < ctx.info.channels; channel++) {
        LOG_F(INFO, "Reading channel %d", channel);
        BufferedInputTrack track = readOneTrackFromContext(ctx, channel);
        tracks.push_back(track);
    }

    return tracks;
}

BufferedInputTrack BufferedTrackUtils::readOneTrackFromContext(const SndContext &ctx, int channel)
{
    const size_t frameCount = (size_t)ctx.info.frames;
    FloatVector buffer(frameCount);
    
    sf_seek(ctx.file, 0, SEEK_SET);
    float* writePtr = &buffer[0];

    // can only read full frames from libsnd.
    // will probably be a lot faster to read the whole thing and then split it
    for (size_t frame = 0; frame < frameCount; frame++) {
        float frameBuffer[ctx.info.channels];
        size_t read = sf_readf_float(ctx.file, frameBuffer, 1);

        if (read == 0) {
            LOG_F(WARNING, "Couldn't read data from input file");
            break;
        }

        *writePtr = frameBuffer[channel];
        writePtr++;
    }

    return BufferedInputTrack(buffer);
}

void BufferedTrackUtils::writeTracksToFile(const std::vector<BufferedOutputTrack> &tracks, const char* path, int channels, int sampleRate)
{
    SF_INFO info = { };
    info.channels = channels;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    info.samplerate = sampleRate;

    SNDFILE* sf = sf_open(path, SFM_WRITE, &info);
    if (!sf) {
        throw std::runtime_error("Cannot open output file");
    }

    size_t frameCount = tracks[0].Length();
    LOG_F(INFO, "Writing %zd frames to file", frameCount);

    // copy audio to buffer so that channels are interleaved
    const size_t bufferFrames = 1024;
    const size_t bufferSize = channels * bufferFrames;
    auto buffer = std::make_unique<float[]>(bufferSize);

    int frames = 0;

    for (size_t frame = 0; frame < frameCount; frame++) {
        for (int channel = 0; channel < channels; channel++) {
            buffer[frames * channels + channel] = tracks[channel].Buffer()[frame];
        }

        frames++;

        if (frames == bufferFrames) {
            sf_count_t written = sf_writef_float(sf, buffer.get(), bufferFrames);
            assert(written > 0);
            frames = 0;
        }
    }

    // write remaining frames left in buffer
    if (frames > 0) {
        sf_count_t written = sf_writef_float(sf, buffer.get(), frames);
        assert(written > 0);
    }

    sf_close(sf);
}