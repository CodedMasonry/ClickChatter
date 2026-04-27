#include "audio.h"
#include <cctype>
#include <cstring>

// ── constants ────────────────────────────────────────────────────────────────

static constexpr int DOT_UNITS = 1;
static constexpr int DASH_UNITS = 3;
static constexpr int SYMBOL_GAP = 1;
static constexpr int LETTER_GAP = 3;
static constexpr int WORD_GAP = 7;

// ── helpers ──────────────────────────────────────────────────────────────────

static std::vector<int> letter_to_morse(char letter) {
  constexpr int DOT = 0, DASH = 1;
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

std::vector<Segment> encode_morse(const std::string &text) {
  std::vector<Segment> result;
  bool first_letter = true;

  auto samples = [](int units) {
    return static_cast<ma_uint32>(units * UNIT_SAMPLES);
  };
  auto tone = [&](int units) { result.push_back({true, samples(units)}); };
  auto silence = [&](int units) { result.push_back({false, samples(units)}); };

  for (char c : text) {
    if (c == ' ') {
      silence(WORD_GAP);
      first_letter = true;
      continue;
    }
    auto symbols = letter_to_morse(c);
    if (symbols.empty())
      continue;

    if (!first_letter)
      silence(LETTER_GAP);
    first_letter = false;

    for (size_t j = 0; j < symbols.size(); ++j) {
      if (j > 0)
        silence(SYMBOL_GAP);
      tone(symbols[j] == 0 ? DOT_UNITS : DASH_UNITS);
    }
  }
  return result;
}

std::string morse_to_symbols(const std::vector<Segment> &segments) {
  std::string result;
  const ma_uint32 word_gap_samples = static_cast<ma_uint32>(4 * UNIT_SAMPLES);

  for (const auto &seg : segments) {
    if (seg.tone) {
      result += (seg.remaining == UNIT_SAMPLES) ? '.' : '-';
    } else if (seg.remaining == word_gap_samples) {
      result += ' ';
    }
    // inter-letter and inter-symbol gaps produce no visible character
  }
  return result;
}

// ── MorseCodeProvider ────────────────────────────────────────────────────────

MorseCodeProvider::MorseCodeProvider(ma_device *device, ma_waveform *target)
    : waveform(target) {
  wave_config = ma_waveform_config_init(
      device->playback.format, device->playback.channels, device->sampleRate,
      ma_waveform_type_square,
      0.2,  // amplitude
      300); // frequency (Hz)
  ma_waveform_init(&wave_config, waveform);
}

void MorseCodeProvider::enqueue(std::string_view text) {
  auto encoded = encode_morse(std::string(text));
  std::lock_guard<std::mutex> lock(mtx);
  queue.insert(queue.end(), encoded.begin(), encoded.end());
}

void MorseCodeProvider::fill(ma_device *device, void *pOutput,
                             ma_uint32 frameCount) {
  std::lock_guard<std::mutex> lock(mtx);

  const ma_uint32 bpf = ma_get_bytes_per_frame(device->playback.format,
                                               device->playback.channels);
  auto out_at = [&](ma_uint32 offset) -> uint8_t * {
    return static_cast<uint8_t *>(pOutput) + offset * bpf;
  };

  ma_uint32 written = 0;
  while (written < frameCount) {
    if (queue.empty()) {
      std::memset(out_at(written), 0, (frameCount - written) * bpf);
      break;
    }

    Segment &seg = queue.front();
    ma_uint32 can = std::min(seg.remaining, frameCount - written);

    if (seg.tone)
      ma_waveform_read_pcm_frames(waveform, out_at(written), can, nullptr);
    else
      std::memset(out_at(written), 0, can * bpf);

    written += can;
    seg.remaining -= can;
    if (seg.remaining == 0)
      queue.pop_front();
  }
}
