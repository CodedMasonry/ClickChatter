#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

#include <iostream>
#include <string>

static constexpr ma_format DEVICE_FORMAT = ma_format_f32;
static constexpr ma_uint32 DEVICE_CHANNELS = 2;
static constexpr ma_uint32 DEVICE_SAMPLE_RATE = 48000;

static std::string get_input() {
  std::string line;
  std::cout << "[I]: ";
  std::getline(std::cin, line);
  return line;
}

static void data_callback(ma_device *device, void *pOutput,
                          const void * /*pInput*/, ma_uint32 frameCount) {
  static_cast<MorseCodeProvider *>(device->pUserData)
      ->fill(device, pOutput, frameCount);
}

int main() {
  ma_waveform waveform{};
  ma_device device{};
  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format = DEVICE_FORMAT;
  config.playback.channels = DEVICE_CHANNELS;
  config.sampleRate = DEVICE_SAMPLE_RATE;
  config.dataCallback = data_callback;

  if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) {
    std::cerr << "Failed to open playback device.\n";
    return -4;
  }

  MorseCodeProvider provider(&device, &waveform);
  device.pUserData = &provider;
  provider.enqueue("SA");

  if (ma_device_start(&device) != MA_SUCCESS) {
    std::cerr << "Failed to start playback device.\n";
    ma_device_uninit(&device);
    return -5;
  }

  for (std::string input;;) {
    input = get_input();
    if (input == "exit")
      break;
    std::cout << "[O]: " << morse_to_symbols(encode_morse(input)) << "\n";
    provider.enqueue(input);
  }

  ma_device_uninit(&device);
  ma_waveform_uninit(&waveform);
}
