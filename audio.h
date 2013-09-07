/// @file audio.h
/// @brief audio utilities
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-03

#ifndef AUDIO_H
#define AUDIO_H

#include <alsa/asoundlib.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace soma
{

/* ALSA is overly complicated for what we're trying to do...
 *
class alsa_audio
{
    private:
    snd_pcm_t *handle;
    static const int SAMPLE_RATE = 44100;
    public:
    audio ()
        : handle (0)
    {
        int err;
        if ((err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
            throw std::runtime_error (snd_strerror (err));
        if ((err = snd_pcm_set_params (handle,
                    SND_PCM_FORMAT_U8,
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    1, // mono
                    SAMPLE_RATE,
                    0, // soft-resample
                    250000)) < 0) // 1/4 sec latency
            throw std::runtime_error (snd_strerror (err));
        sleep (1);
    }
    ~audio ()
    {
        sleep (1);
        snd_pcm_close (handle);
    }
    /// @see http://en.wikipedia.org/wiki/Piano_key_frequencies
    void play (int freq = 262, int millisecs = 1000) const
    {
        const int SAMPLES = SAMPLE_RATE * millisecs / 1000;
        std::vector<uint8_t> samples (SAMPLES);
        for (int t = 0; t < SAMPLES; ++t)
            samples[t] = (sin (t * 2.0 * M_PI / SAMPLE_RATE * freq) + 1.0) * 128.0;
        snd_pcm_sframes_t frames = snd_pcm_writei (handle, &samples[0], samples.size ());
        if (frames < 0)
            frames = snd_pcm_recover (handle, frames, 0);
        if (frames < 0)
            throw std::runtime_error (snd_strerror (frames));
    }
};
*/

class audio
{
    private:
    const int sr;
    const std::string cmd;
    const std::string flags;
    FILE *fp;
    public:
    audio ()
        : sr (8000)
        , cmd ("aplay --quiet --format=U8 --channels=1 --rate=8000")
        , flags ("w")
        , fp (popen (cmd.c_str (), flags.c_str ()))
    {
        if (!fp)
            throw std::runtime_error ("cannot open pipe to 'aplay' process");
        sleep (1);
    }
    ~audio ()
    {
        pclose (fp);
        sleep (1);
    }
    /// @see http://en.wikipedia.org/wiki/Piano_key_frequencies
    void play (int freq = 262, int millisecs = 1000) const
    {
        const size_t samples = sr * millisecs / 1000;
        std::vector<unsigned char> s (samples);
        for (size_t t = 0; t < samples; ++t)
            s[t] = (sin (t * 2.0 * M_PI / sr * freq) + 1.0) * 128.0;
        size_t n = fwrite (&s[0], samples, 1, fp);
        if (n != 1)
            throw std::runtime_error ("could not write to audio device");
        fflush (fp);
    }
};

}

#endif
