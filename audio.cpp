#include "audio.h"
#include <cctype>
#include <cstring>

// ── helpers ──────────────────────────────────────────────────────────────────

static const int DOT = 0;
static const int DASH = 1;
static const int SILENCE = -1;

static std::vector<int> letter_to_morse(char letter) {
  switch (std::tolower(letter)) {
  case 'a':
    return {DOT, DASH};
  case 'b':
    return {DASH, DOT, DOT, DOT};
  case 'c':
    return {DASH, DOT, DASH, DOT};
  case 'd':
    return {DASH, DOT, DOT};
  case 'e':
    return {DOT};
  case 'f':
    return {DOT, DOT, DASH, DOT};
  case 'g':
    return {DASH, DASH, DOT};
  case 'h':
    return {DOT, DOT, DOT, DOT};
  case 'i':
    return {DOT, DOT};
  case 'j':
    return {DOT, DASH, DASH, DASH};
  case 'k':
    return {DASH, DOT, DASH};
  case 'l':
    return {DOT, DASH, DOT, DOT};
  case 'm':
    return {DASH, DASH};
  case 'n':
    return {DASH, DOT};
  case 'o':
    return {DASH, DASH, DASH};
  case 'p':
    return {DOT, DASH, DASH, DOT};
  case 'q':
    return {DASH, DASH, DOT, DASH};
  case 'r':
    return {DOT, DASH, DOT};
  case 's':
    return {DOT, DOT, DOT};
  case 't':
    return {DASH};
  case 'u':
    return {DOT, DOT, DASH};
  case 'v':
    return {DOT, DOT, DOT, DASH};
  case 'w':
    return {DOT, DASH, DASH};
  case 'x':
    return {DASH, DOT, DOT, DASH};
  case 'y':
    return {DASH, DOT, DASH, DASH};
  case 'z':
    return {DASH, DASH, DOT, DOT};
  default:
    return {};
  }
}

MorseCodeProvider::MorseCodeProvider(ma_device *device, ma_waveform *target)
    : waveform1(target) {
  wave_config = ma_waveform_config_init(
      device->playback.format, device->playback.channels, device->sampleRate,
      ma_waveform_type_square,
      0.05, // amplitude
      600);
  ma_waveform_init(&wave_config, waveform1);
}

void MorseCodeProvider::push_symbol(bool tone, int units) {
  queue.push_back({tone, (ma_uint32)(units * UNIT_SAMPLES)});
}

void MorseCodeProvider::enqueue(const std::string &text) {
  std::lock_guard<std::mutex> lock(mtx);

  bool first_word = true;
  bool first_letter = true;

  for (size_t i = 0; i < text.size(); ++i) {
    char c = text[i];

    if (c == ' ') {
      // Inter-word gap: 7 units total, but 3 were already added after
      // the last letter, so add 4 more.
      push_symbol(false, 4);
      first_letter = true;
      continue;
    }

    auto symbols = letter_to_morse(c);
    if (symbols.empty())
      continue;

    // Inter-letter gap (3 units) — skip before the very first letter
    if (!first_letter) {
      push_symbol(false, 3);
    }
    first_letter = false;

    for (size_t j = 0; j < symbols.size(); ++j) {
      if (j > 0)
        push_symbol(false, 1); // inter-symbol gap

      if (symbols[j] == DOT)
        push_symbol(true, 1);
      else if (symbols[j] == DASH)
        push_symbol(true, 3);
    }
  }
}

void MorseCodeProvider::fill(ma_device *device, void *pOutput,
                             ma_uint32 frameCount) {
  std::lock_guard<std::mutex> lock(mtx);

  // byte width of one frame (all channels)
  const ma_uint32 bpf = ma_get_bytes_per_frame(device->playback.format,
                                               device->playback.channels);

  ma_uint32 written = 0;
  uint8_t *out = (uint8_t *)pOutput;

  while (written < frameCount) {
    if (queue.empty()) {
      // Nothing queued — output silence for the rest of the buffer
      std::memset(out + written * bpf, 0, (frameCount - written) * bpf);
      written = frameCount;
      break;
    }

    Segment &seg = queue.front();
    ma_uint32 can = std::min(seg.remaining, frameCount - written);

    if (seg.tone) {
      // Ask miniaudio to render `can` frames of the waveform
      ma_waveform_read_pcm_frames(waveform1, out + written * bpf, can, NULL);
    } else {
      std::memset(out + written * bpf, 0, can * bpf);
    }

    written += can;
    seg.remaining -= can;
    if (seg.remaining == 0)
      queue.pop_front();
  }
}
