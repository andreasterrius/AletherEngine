//
// Created by Alether on 6/21/2025.
//
module graphics;

Font::Font(vector<char> data) : data(data) {}

int Font::get_byte_size() { return data.size(); }

void *Font::get_ptr() { return data.data(); }
