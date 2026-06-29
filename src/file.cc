/*
# 076 Free License

Copyright (c) テクニカル諏訪子

Permission is hereby granted to any person obtaining a copy of the software
C++ Utils (the "Software") to use, modify, merge, copy, publish, distribute,
sublicense, and/or sell copies of the Software, subject to the following conditions:

    1. **Origin Attribution**:
       - You must not misrepresent the origin of the Software; you must not claim
         you created the original Software.
    2. **Notice Preservation**:
       - This license and the above copyright notice must remain intact in all copies
         of the source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <util/file.hh>

#include <iostream>
#include <fstream>

vector<u8> read_file(const string &filename, bool binary) {
  std::ios::openmode flags = std::ios::in | std::ios::ate;
  //if (binary) flags |= std::ios::binary;
  flags |= std::ios::binary;

  std::ifstream file(filename, flags);
  if (!file) {
    throw std::runtime_error("ファイルを開くに失敗： " + filename);
  }

  auto size = file.tellg();
  file.clear();
  file.seekg(0, std::ios::beg);

  vector<u8> data(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char *>(data.data()), size)) {
    throw std::runtime_error("ファイルの読み込みに失敗。");
  }

  file.close();

  return data;
}

bool write_file(const string &filename, vector<string> contents, bool binary) {
  std::ios::openmode flags;
  if (binary) {
    flags = std::ios::binary | std::ios::ate;
  } else {
    flags = std::ios::out | std::ios::trunc;
  }

  std::ofstream file(filename, flags);
  if (!file) {
    throw std::runtime_error("ファイルを開くに失敗： " + filename);
  }

  for (const auto &c : contents) {
    file << c;
    file << "\n";
  }

  file.close();

  return true;
}
