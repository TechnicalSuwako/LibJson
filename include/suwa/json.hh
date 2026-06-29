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
#ifndef JSON_HH
#define JSON_HH

#include <util/types.hh>
#include <unordered_map>
#include <string_view>
#include <variant>
#include <optional>
#include <memory>

namespace json {
  enum class Error {
    None,
    SyntaxError,
    UnexpectedEnd,
    InvalidValue,
    InvalidNumber,
    UnterminatedString,
  }; // enum class Error

  struct SerializeOptions {
    bool pretty = false;
    size_t indent = 2;
  };

  struct ParseResult;
  struct StringHash;
  class Object;
  class Value;

  struct ParseResult {
    std::unique_ptr<Value> value;
    Error error = Error::None;
    size_t errPos = 0;
    bool ok() const { return error == Error::None; }
  }; // struct ParseResult

  struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const {
      return std::hash<std::string_view>{}(sv);
    }
  }; // struct StringHash

  class Object {
    public:
      Object() = default;

      Object(const Object &) = delete;
      Object &operator=(const Object &) = delete;

      Object(Object &&) noexcept = default;
      Object &operator=(Object &&) noexcept = default;

      Value &operator[](std::string_view key);
      const Value &operator[](std::string_view key) const;

      void insert(string key, Value value);
      Value *get(std::string_view key);
      const Value *get(std::string_view key) const;
      bool contains(std::string_view key) const;

    public:
      bool empty() const { return m_Data.empty(); }
      size_t size() const { return m_Data.size(); }
      auto begin() { return m_Data.begin(); }
      auto end() { return m_Data.end(); }
      auto begin() const { return m_Data.begin(); }
      auto end() const { return m_Data.end(); }

    private:
      using Pair = std::pair<string, std::unique_ptr<Value>>;
      vector<Pair> m_Data;
      std::unordered_map<string, size_t, StringHash, std::equal_to<>> m_Index;
  }; // struct Object

  class Value {
    public:
      using Array = vector<Value>;
      using Object = json::Object;

      using Variant = std::variant<
        std::monostate, // null
        bool,
        f64,
        string,
        Array,
        Object
      >;

      // 配列
      Value &operator[](size_t index) {
        if (!is_array()) as_array() = Array{};

        auto &arr = as_array();
        if (index >= arr.size()) arr.resize(index + 1);
        return arr[index];
      }

      // オブジェクト
      Value &operator[](std::string_view key) {
        if (!is_object()) as_object() = Object{};

        auto *val = as_object().get(key);
        if (!val) {
          as_object().insert(string(key), Value{});
          val = as_object().get(key);
        }
        return *val;
      }

    public:
      Value() = default;
      Value(std::nullptr_t) : m_Value(std::monostate{}) {}
      Value(bool b) : m_Value(b) {}
      Value(f64 n) : m_Value(n) {}
      Value(f32 n) : m_Value(n) {}
      Value(i64 n) : m_Value(static_cast<f64>(n)) {}
      Value(i32 n) : m_Value(static_cast<f64>(n)) {}
      Value(u64 n) : m_Value(static_cast<f64>(n)) {}
      Value(u32 n) : m_Value(static_cast<f64>(n)) {}
      Value(string s) : m_Value(std::move(s)) {}
      Value(cstr s) : m_Value(string(s)) {}
      Value(Array &&arr) : m_Value(std::move(arr)) {}
      Value(Object &&obj) : m_Value(std::move(obj)) {}

    public:
      bool is_null() const { return std::holds_alternative<std::monostate>(m_Value); }
      bool is_bool() const { return std::holds_alternative<bool>(m_Value); }
      bool is_number() const { return std::holds_alternative<f64>(m_Value); }
      bool is_string() const { return std::holds_alternative<string>(m_Value); }
      bool is_array() const { return std::holds_alternative<Array>(m_Value); }
      bool is_object() const { return std::holds_alternative<Object>(m_Value); }

    public:
      bool as_bool() const { return std::get<bool>(m_Value); }
      f64 as_number() const { return std::get<f64>(m_Value); }
      string &as_string() { return std::get<string>(m_Value); }
      const string &as_string() const { return std::get<string>(m_Value); }
      Array &as_array() { return std::get<Array>(m_Value); }
      const Array &as_array() const { return std::get<Array>(m_Value); }
      Object &as_object() { return std::get<Object>(m_Value); }
      const Object &as_object() const { return std::get<Object>(m_Value); }

    public:
      std::optional<bool> get_bool() const;
      std::optional<f64> get_number() const;
      std::optional<string> get_string() const;

    public:
      static ParseResult parse(std::string_view jsonText);

    public:
      string serialize(const SerializeOptions &opts = {}) const;
      bool serialize_to_file(const string &path, const SerializeOptions &opts = {}) const;

    private:
      static string make_indent(size_t depth, size_t width);

    private:
      string serialize_impl(const SerializeOptions &opts, size_t depth) const;
      static string serialize_string(const string &s);

    private:
      Variant m_Value;
  }; // class Value
  
  class Parser {
    public:
      Parser(std::string_view text) : m_Text(text), m_Pos(0) {}

      ParseResult parse();
      static ParseResult parse_from_file(const string &path);

    private:
      std::string_view m_Text;
      size_t m_Pos;

      ParseResult make_error(Error err) const;
      void skip_whitespace();
      char peek() const;
      char consume();
      bool expect(char c);

      ParseResult parse_value();
      ParseResult parse_object();
      ParseResult parse_array();
      ParseResult parse_string();
      std::optional<f64> parse_number();
  }; // class Parser
} // namespace json

inline std::ostream &operator<<(std::ostream &os, const json::Value &v) {
  os << v.serialize();
  return os;
}

inline json::Value &json::Object::operator[](std::string_view key) {
  return *get(key);
}

inline const json::Value &json::Object::operator[](std::string_view key) const {
  return *get(key);
}

#endif // JSON_HH
