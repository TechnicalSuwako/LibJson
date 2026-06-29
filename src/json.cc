/*
# 076 Free License

Copyright (c) テクニカル諏訪子

Permission is hereby granted to any person obtaining a copy of the software
LibJson (the "Software") to use, modify, merge, copy, publish, distribute,
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
#include <suwa/json.hh>
#include <util/file.hh>
#include <charconv>
#include <utility>
#include <cassert>
#include <sstream>

namespace json {
  ParseResult Parser::make_error(Error err) const {
    return { std::make_unique<Value>(), err, m_Pos };
  }

  void Parser::skip_whitespace() {
    while (m_Pos < m_Text.size()) {
      char c = m_Text[m_Pos];
      if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
      ++m_Pos;
    }
  }

  char Parser::peek() const {
    return m_Pos < m_Text.size() ? m_Text[m_Pos] : '\0';
  }

  char Parser::consume() {
    return m_Pos < m_Text.size() ? m_Text[m_Pos++] : '\0';
  }

  bool Parser::expect(char c) {
    if (peek() == c) {
      consume();
      return true;
    }

    return false;
  }

  ParseResult Parser::parse_value() {
    skip_whitespace();
    if (m_Pos >= m_Text.size()) return make_error(Error::UnexpectedEnd);

    char c = peek();
    if (c == '{') return parse_object();
    if (c == '[') return parse_array();
    if (c == '"') {
      auto str = parse_string();
      if (!str) return make_error(Error::UnterminatedString);
      return { std::make_unique<Value>(std::move(*str)) };
    }

    if (m_Text.substr(m_Pos, 4) == "true") {
      m_Pos += 4;
      return { std::make_unique<Value>(true) };
    }

    if (m_Text.substr(m_Pos, 5) == "false") {
      m_Pos += 5;
      return { std::make_unique<Value>(false) };
    }

    if (m_Text.substr(m_Pos, 4) == "null") {
      m_Pos += 4;
      return { std::make_unique<Value>(nullptr) };
    }

    if ((c >= '0' && c <= '9') || c == '-' || c == '+') {
      auto num = parse_number();
      if (num) return { std::make_unique<Value>(*num) };
      return make_error(Error::InvalidNumber);
    }

    return make_error(Error::InvalidValue);
  }

  ParseResult Parser::parse_object() {
    if (!expect('{')) return make_error(Error::SyntaxError);

    Object obj;
    skip_whitespace();

    if (peek() == '}') {
      consume();
      return { std::make_unique<Value>(std::move(obj)) };
    }

    while (true) {
      skip_whitespace();
      auto keyopt = parse_string();
      if (!keyopt) return make_error(Error::SyntaxError);

      skip_whitespace();
      if (!expect(':')) return make_error(Error::SyntaxError);

      auto valres = parse_value();
      if (!valres.ok()) return valres;

      obj.insert(std::move(*keyopt), std::move(*valres.value));

      skip_whitespace();
      if (peek() == '}') {
        consume();
        break;
      }
      if (!expect(',')) return make_error(Error::SyntaxError);
    }

    return { std::make_unique<Value>(std::move(obj)) };
  }

  ParseResult Parser::parse_array() {
    if (!expect('[')) return make_error(Error::SyntaxError);

    Value::Array arr;
    skip_whitespace();

    if (peek() == ']') {
      consume();
      return { std::make_unique<Value>(std::move(arr)) };
    }

    while (true) {
      auto valres = parse_value();
      if (!valres.ok()) return valres;
      arr.push_back(std::move(*valres.value));

      skip_whitespace();
      if (peek() == ']') {
        consume();
        break;
      }
      if (!expect(',')) return make_error(Error::SyntaxError);
    }

    return { std::make_unique<Value>(std::move(arr)) };
  }

  std::optional<string> Parser::parse_string() {
    if (!expect('"')) return std::nullopt;

    string res;
    res.reserve(32);

    while (m_Pos < m_Text.size()) {
      char c = consume();
      if (c == '"') return res;
      if (c == '\\') {
        if (m_Pos >= m_Text.size()) return std::nullopt;
        char esc = consume();
        switch (esc) {
          case '"': res += '"'; break;
          case '\\': res += '\\'; break;
          case '/': res += '/'; break;
          case 'b': res += '\b'; break;
          case 'f': res += '\f'; break;
          case 'n': res += '\n'; break;
          case 'r': res += '\r'; break;
          case 't': res += '\t'; break;
          default: res += esc;
        }
      } else {
        res += c;
      }
    }

    return std::nullopt;
  }

  std::optional<f64> Parser::parse_number() {
    cstr start = m_Text.data() + m_Pos;
    f64 num = 0.0;
    auto [ptr, ec] = std::from_chars(start, m_Text.data() + m_Text.size(), num);

    if (ec != std::errc() || ptr == start) return std::nullopt;

    m_Pos += (ptr - start);
    return num;
  }
  
  // 公開API
  ParseResult Value::parse(std::string_view jsonText) {
    Parser p(jsonText);
    return p.parse();
  }

  ParseResult Parser::parse() {
    skip_whitespace();
    ParseResult res = parse_value();
    if (!res.ok()) return res;

    skip_whitespace();
    if (m_Pos < m_Text.size()) {
      assert(false && "JSON値の後でデータがある");
    }

    return res;
  }

  ParseResult Parser::parse_from_file(const string &path) {
    vector<u8> data = read_file(path, false);
    string content(data.begin(), data.end());
    return Parser(content).parse();
  }

  string Value::serialize(const SerializeOptions &opts) const {
    return serialize_impl(opts, 0);
  }
  
  string Value::serialize_impl(const SerializeOptions &opts, size_t depth) const {
    if (is_null())   return "null";
    if (is_bool())   return as_bool() ? "true" : "false";
    if (is_number()) {
      std::ostringstream oss;
      oss << as_number();
      return oss.str();
    }
    if (is_string())
      return serialize_string(as_string());

    if (is_array()) {
      const auto &arr = as_array();

      string out = "[";
      if (opts.pretty && !arr.empty())
        out += "\n";

      for (size_t i = 0; i < arr.size(); ++i) {
        if (opts.pretty)
          out += make_indent(depth + 1, opts.indent);

        out += arr[i].serialize_impl(opts, depth + 1);

        if (i + 1 < arr.size())
          out += ",";

        if (opts.pretty)
          out += "\n";
      }

      if (opts.pretty && !arr.empty())
        out += make_indent(depth, opts.indent);

      out += "]";
      return out;
    }

    if (is_object()) {
      const auto &obj = as_object();

      string out = "{";

      if (opts.pretty && !obj.empty())
        out += "\n";

      size_t i = 0;
      for (const auto &[key, val] : obj) {
        if (opts.pretty)
          out += make_indent(depth + 1, opts.indent);

        out += serialize_string(key);
        out += opts.pretty ? ": " : ":";
        out += val.serialize_impl(opts, depth + 1);

        if (++i < obj.size())
          out += ",";

        if (opts.pretty)
          out += "\n";
      }

      if (opts.pretty && !obj.empty())
        out += make_indent(depth, opts.indent);

      out += "}";
      return out;
    }

    return "null";
  }

  bool Value::serialize_to_file(const string &path, const SerializeOptions &opts) const {
    return write_file(path, { serialize(opts) }, false);
  }

  string Value::make_indent(size_t depth, size_t width) {
    return string(depth * width, ' ');
  }
  
  string Value::serialize_string(const string &s) {
    string out = "\"";

    for (char c : s) {
      switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += c; break;
      }
    }

    out += "\"";
    return out;
  }

  void Object::insert(string key, Value value) {
    auto it = m_Index.find(key);
    if (it != m_Index.end()) {
      m_Data[it->second].second = std::move(value);
    } else {
      m_Index[key] = m_Data.size();
      m_Data.emplace_back(std::move(key), std::move(value));
    }
  }

  Value *Object::get(std::string_view key) {
    auto it = m_Index.find(key);
    return it != m_Index.end() ? &m_Data[it->second].second : nullptr;
  }

  const Value *Object::get(std::string_view key) const {
    auto it = m_Index.find(key);
    return it != m_Index.end() ? &m_Data[it->second].second : nullptr;
  }

  bool Object::contains(std::string_view key) const {
    return m_Index.contains(key);
  }

  //////////////

  std::optional<bool> Value::get_bool() const {
    if (auto *b = std::get_if<bool>(&m_Value)) return *b;
    return std::nullopt;
  }

  std::optional<f64> Value::get_number() const {
    if (auto *b = std::get_if<f64>(&m_Value)) return *b;
    return std::nullopt;
  }

  std::optional<string> Value::get_string() const {
    if (auto *b = std::get_if<string>(&m_Value)) return *b;
    return std::nullopt;
  }

  Value &Value::operator[](size_t index) {
    if (!is_array()) as_array() = Array{};

    auto &arr = as_array();
    if (index >= arr.size()) arr.resize(index + 1);
    return arr[index];
  }

  Value &Value::operator[](std::string_view key) {
    if (!is_object()) as_object() = Object{};

    auto *val = as_object().get(key);
    if (!val) {
      as_object().insert(string(key), Value{});
      val = as_object().get(key);
    }
    return *val;
  }
} // namespace json
