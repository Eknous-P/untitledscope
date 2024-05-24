#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

#include "../shared.h"

enum AudioErrors {
  NOERR=0,
  NOINIT,
  NODEVS,
  NODEV,
  NOSTART,
  NOGOOD
};

class AudioInput {
  private:

    struct AudioConfig{
      PaDeviceIndex device;
      int sampleRate;
      int frameSize;
      unsigned char channels;
    };

    struct AudioBuffer {
      float *data;
      unsigned long int size;
      unsigned long int index;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    std::vector<DeviceEntry> devs;
  
    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    bool isGood, running;

    int bufferGetCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long int framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags);

    static int _PaCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long int framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags,
      void *userData );
  
  public:
    AudioInput(unsigned int frameSize, unsigned int bufferSize,unsigned char channelsDef, unsigned int sampleRateDef);
    std::vector<DeviceEntry> enumerateDevs();
    int init(PaDeviceIndex dev);
    int stop();
    float *getData();
    const PaDeviceInfo* getDeviceInfo();
    ~AudioInput();
};

class AudioProcess {
  private:
    float *dataIn, *dataOut;
    unsigned long int dataSize, i;
    float *alignRamp;
  public:
    void writeDataIn(float* d);
    float *getDataIn();
    float *getDataOut();
    void derive();
    void integrate();

    float *alignWave(float trigger,unsigned long int waveLen, long int offset, bool edge); // true -> falling, false ->rising

    AudioProcess(unsigned int bufferSizeDef);
    ~AudioProcess();
};