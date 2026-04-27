#pragma once
#include <deque>
#include <miniaudio.h>
#include <mutex>
#include <string>
#include <vector>

#define SAMPLE_RATE 48000
#define UNIT_MS 75 // ms per dot
#define UNIT_SAMPLES (SAMPLE_RATE * UNIT_MS / 1000)

struct Segment {
  bool tone;
  ma_uint32 remaining; // in frames
};

static std::vector<int> letter_to_morse(char letter);

std::vector<Segment> encode_morse(const std::string &text);

std::string morse_to_symbols(const std::vector<Segment> &segments);

class MorseCodeProvider {
public:
  MorseCodeProvider(ma_device *device, ma_waveform *target);

  // Call this to queue a string for playback
  void enqueue(std::string_view text);

  // Called from data_callback — fills pOutput with tone or silence
  void fill(ma_device *device, void *pOutput, ma_uint32 frameCount);

private:
  ma_waveform_config wave_config;
  ma_waveform *waveform;

  std::deque<Segment> queue;
  std::mutex mtx;

  void push_symbol(bool tone, int units);
};
