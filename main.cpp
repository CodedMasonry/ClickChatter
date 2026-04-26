#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"
#include <miniaudio.h>
#include <stdio.h>

#define DEVICE_FORMAT ma_format_f32
#define DEVICE_CHANNELS 2
#define DEVICE_SAMPLE_RATE 48000

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
  ((MorseCodeProvider *)pDevice->pUserData)->fill(pDevice, pOutput, frameCount);
  (void)pInput;
}

int main(int argc, char **argv) {
  ma_waveform targetData;
  ma_device_config deviceConfig;
  ma_device device;

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = DEVICE_FORMAT;
  deviceConfig.playback.channels = DEVICE_CHANNELS;
  deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = NULL; // placeholder

  if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    return -4;
  }

  MorseCodeProvider provider(&device, &targetData);
  device.pUserData = &provider;

  provider.enqueue("Hello World");

  if (ma_device_start(&device) != MA_SUCCESS) {
    printf("Failed to start playback device.\n");
    ma_device_uninit(&device);
    return -5;
  }

  printf("Press Enter to quit...\n");
  getchar();

  ma_device_uninit(&device);
  ma_waveform_uninit(&targetData);

  (void)argc;
  (void)argv;
  return 0;
}
