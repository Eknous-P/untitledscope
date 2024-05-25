#include "audio.h"

AudioInput::AudioInput(unsigned int frameSize, unsigned int bufferSize, unsigned char channelsDef, unsigned int sampleRateDef) {
  isGood = Pa_Initialize()==paNoError;
  running = false;
  conf.channels = channelsDef;
  conf.sampleRate = sampleRateDef;
  conf.frameSize = frameSize;

  conf.device = 0;

  buffer.size = bufferSize;
  buffer.data = new float*[conf.channels];
  for (unsigned char i = 0; i < conf.channels; i++) {
    buffer.data[i] = new float[buffer.size];
  }
}

int AudioInput::_PaCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData ) {
    return ((AudioInput*)userData)->bufferGetCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    const float *audIn = (const float*)inputBuffer;
  
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    unsigned char j = 0;

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < buffer.size*conf.channels; buffer.index++) {
        buffer.data[buffer.index] = 0;
      }
    } else {
      // push vaules back
      for (j = 0; j < conf.channels; j++) {
        memcpy(buffer.data[j],
        buffer.data[j] + framesPerBuffer,
        (buffer.size-framesPerBuffer)*sizeof(float));
      }
      // get data
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        for (j = 0; j < conf.channels; j++) {
        // buffer.data[buffer.size - framesPerBuffer + buffer.index] = *audIn++;
          buffer.data[j][buffer.size - framesPerBuffer + buffer.index] = *audIn++;
        }
      }
    }
    return paContinue;
}

std::vector<DeviceEntry> AudioInput::enumerateDevs() {
  devs.clear();
  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
    if (info == NULL) continue;
    if (info->maxInputChannels < 1) continue;
    if (info->maxOutputChannels > 0) continue;
    devs.push_back(DeviceEntry(i,
      std::to_string(i) + ": " +
      std::string(Pa_GetHostApiInfo(info->hostApi)->name) + " | " + 
      std::string(info->name)));
  }
  return devs;
}

int AudioInput::init(PaDeviceIndex dev) {
  if (!isGood) return NOGOOD;
  if (running) return NOERR;
  int nDevs = Pa_GetDeviceCount();
  if (nDevs == 0) return NODEVS;
  if (nDevs < 0) return nDevs;

  streamParams.device = dev;
  if (streamParams.device == paNoDevice) return NODEV;
  streamParams.channelCount = conf.channels;
  streamParams.sampleFormat = paFloat32;
  streamParams.suggestedLatency = Pa_GetDeviceInfo(streamParams.device)->defaultLowInputLatency;
  streamParams.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
    &stream,
    &streamParams,
    NULL,
    conf.sampleRate,
    conf.frameSize,
    paClipOff,
    &AudioInput::_PaCallback,
    this
  );
  Pa_Sleep(100);

  if (err!=paNoError) return err;

  err = Pa_StartStream(stream);
  
  conf.device=dev;
  running = err==paNoError;
  return err;
}

int AudioInput::stop() {
  if (!running) return 0;
  running = false;
  return Pa_CloseStream(stream);
}

AudioInput::~AudioInput() {
  if (isGood) Pa_Terminate();
  if (buffer.data) {
    // for (unsigned char i = 0; i < conf.channels; i++) {
    //   delete[] buffer.data[i];
    //   buffer.data[i] = NULL;
    // }
    delete[] buffer.data;
    buffer.data = NULL;
  }
}

float *AudioInput::getData(unsigned char chan) {
  return buffer.data[chan];
}

const PaDeviceInfo *AudioInput::getDeviceInfo() {
  return Pa_GetDeviceInfo(conf.device);
}