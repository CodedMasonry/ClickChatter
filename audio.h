#pragma once
#include <deque>
#include <miniaudio.h>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

static constexpr int SAMPLE_RATE = 48000;
static constexpr int UNIT_MS = 75;
static constexpr int UNIT_SAMPLES = SAMPLE_RATE * UNIT_MS / 1000;

struct Segment {
  bool tone;
  ma_uint32 remaining; // frames
};

std::vector<Segment> encode_morse(const std::string &text);
std::string morse_to_symbols(const std::vector<Segment> &segments);

class MorseCodeProvider {
public:
  MorseCodeProvider(ma_device *device, ma_waveform *target);

  void enqueue(std::string_view text);
  void fill(ma_device *device, void *pOutput, ma_uint32 frameCount);

private:
  ma_waveform_config wave_config;
  ma_waveform *waveform;
  std::deque<Segment> queue;
  std::mutex mtx;
};
