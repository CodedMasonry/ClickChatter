#include <cctype>
#include <vector>

const int dot = 0;
const int dash = 1;

std::vector<int> letter_to_morse(char letter) {
  char val = std::tolower(letter);
  switch (letter) {
  case 'a':
    return {dot, dash};
  case 'b':
    return {dash, dot, dot, dot};
  case 'c':
    return {dash, dot, dash, dot};
  case 'd':
    return {dash, dot, dot};
  case 'e':
    return {dot};
  case 'f':
    return {dot, dot, dash, dot};
  case 'g':
    return {dash, dash, dot};
  case 'h':
    return {dot, dot, dot, dot};
  case 'i':
    return {dot, dot};
  case 'j':
    return {dot, dash, dash, dash};
  case 'k':
    return {dash, dot, dash};
  case 'l':
    return {dot, dash, dot, dot};
  case 'm':
    return {dash, dash};
  case 'n':
    return {dash, dot};
  case 'o':
    return {dash, dash, dash};
  case 'p':
    return {dot, dash, dash, dot};
  case 'q':
    return {dash, dash, dot, dash};
  case 'r':
    return {dash, dot, dash};
  case 's':
    return {dot, dot, dot};
  case 't':
    return {dash};
  case 'u':
    return {dot, dot, dash};
  case 'v':
    return {dot, dot, dot, dash};
  case 'w':
    return {dot, dash, dash};
  case 'x':
    return {dash, dot, dot, dash};
  case 'y':
    return {dash, dot, dash, dash};
  case 'z':
    return {dash, dash, dot, dot};
  default:
    return {-1};
  }
};
