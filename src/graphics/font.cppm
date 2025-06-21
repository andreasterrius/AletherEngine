//
// Created by Alether on 6/21/2025.
//
module;

#include <vector>

export module graphics:font;

using namespace std;

export namespace ale::graphics {
class Font {
private:
  vector<char> data;

public:
  Font(vector<char> data);
  int get_byte_size();
  void *get_ptr();
};
} // namespace ale::graphics
