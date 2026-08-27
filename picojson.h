/*
 * Copyright 2009-2010 Cybozu Labs, Inc.
 * Copyright 2011-2014 Kazuho Oku
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef picojson_h
#define picojson_h

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

// for isnan/isinf
#if __cplusplus >= 201103L
#include <cmath>
#else
extern "C" {
#ifdef _MSC_VER
#include <float.h>
#elif defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif
}
#endif

#ifndef PICOJSON_USE_RVALUE_REFERENCE
#if (defined(__cpp_rvalue_references) && __cpp_rvalue_references >= 200610) || (defined(_MSC_VER) && _MSC_VER >= 1600)
#define PICOJSON_USE_RVALUE_REFERENCE 1
#else
#define PICOJSON_USE_RVALUE_REFERENCE 0
#endif
#endif // PICOJSON_USE_RVALUE_REFERENCE

#ifndef PICOJSON_NOEXCEPT
#if PICOJSON_USE_RVALUE_REFERENCE
#define PICOJSON_NOEXCEPT noexcept
#else
#define PICOJSON_NOEXCEPT throw()
#endif
#endif

// experimental support for int64_t (see README.mkdn for detail)
#ifdef PICOJSON_USE_INT64
#define __STDC_FORMAT_MACROS
#include <cerrno>
#if __cplusplus >= 201103L
#include <cinttypes>
#else
extern "C" {
#include <inttypes.h>
}
#endif
#endif

// to disable the use of localeconv(3), set PICOJSON_USE_LOCALE to 0
#ifndef PICOJSON_USE_LOCALE
#define PICOJSON_USE_LOCALE 1
#endif
#if PICOJSON_USE_LOCALE
extern "C" {
#include <locale.h>
}
#endif

#ifndef PICOJSON_ASSERT
#define PICOJSON_ASSERT(e)                                                                                                         \
  do {                                                                                                                             \
    if (!(e))                                                                                                                      \
      throw std::runtime_error(#e);                                                                                                \
  } while (0)
#endif

#ifdef _MSC_VER
#define SNPRINTF _snprintf_s
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from int to char
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4702) // unreachable code
#pragma warning(disable : 4706) // assignment within conditional expression
#else
#define SNPRINTF snprintf
#endif

namespace picojson {

enum {
  null_type,
  boolean_type,
  number_type,
  string_type,
  array_type,
  object_type
#ifdef PICOJSON_USE_INT64
  ,
  int64_type
#endif
};

enum { INDENT_WIDTH = 2, DEFAULT_MAX_DEPTHS = 100 };

struct null {};

class value {
public:
  typedef std::vector<value> array;
  typedef std::map<std::string, value> object;
  union _storage {
    bool boolean_;
    double number_;
#ifdef PICOJSON_USE_INT64
    int64_t int64_;
#endif
    std::string *string_;
    array *array_;
    object *object_;
  };

protected:
  int type_;
  _storage u_;

public:
  value();
  value(int type, bool);
  explicit value(bool b);
#ifdef PICOJSON_USE_INT64
  explicit value(int64_t i);
#endif
  explicit value(double n);
  explicit value(const std::string &s);
  explicit value(const array &a);
  explicit value(const object &o);
#if PICOJSON_USE_RVALUE_REFERENCE
  explicit value(std::string &&s);
  explicit value(array &&a);
  explicit value(object &&o);
#endif
  explicit value(const char *s);
  value(const char *s, size_t len);
  ~value();
  value(const value &x);
  value &operator=(const value &x);
#if PICOJSON_USE_RVALUE_REFERENCE
  value(value &&x) PICOJSON_NOEXCEPT;
  value &operator=(value &&x) PICOJSON_NOEXCEPT;
#endif
  void swap(value &x) PICOJSON_NOEXCEPT;
  template <typename T> bool is() const;
  template <typename T> const T &get() const;
  template <typename T> T &get();
  template <typename T> void set(const T &);
#if PICOJSON_USE_RVALUE_REFERENCE
  template <typename T> void set(T &&);
#endif
  bool evaluate_as_boolean() const;
  const value &get(const size_t idx) const;
  const value &get(const std::string &key) const;
  value &get(const size_t idx);
  value &get(const std::string &key);

  bool contains(const size_t idx) const;
  bool contains(const std::string &key) const;
  std::string to_str() const;
  template <typename Iter> void serialize(Iter os, bool prettify = false) const;
  std::string serialize(bool prettify = false) const;

private:
  template <typename T> value(const T *); // intentionally defined to block implicit conversion of pointer to bool
  template <typename Iter> static void _indent(Iter os, int indent);
  template <typename Iter> void _serialize(Iter os, int indent) const;
  std::string _serialize(int indent) const;
  void clear();
};

typedef value::array array;
typedef value::object object;

inline value::value() : type_(null_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(int type, bool) : type_(type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(bool b) : type_(boolean_type), u_() { __builtin_trap() /* STUB: not implemented */; }

#ifdef PICOJSON_USE_INT64
inline value::value(int64_t i) : type_(int64_type), u_() { __builtin_trap() /* STUB: not implemented */; }
#endif

inline value::value(double n) : type_(number_type), u_() {
  if (
#ifdef _MSC_VER
      !_finite(n)
#elif __cplusplus >= 201103L
      std::isnan(n) || std::isinf(n)
#else
      isnan(n) || isinf(n)
#endif
          ) {
    throw std::overflow_error("");
  }
  u_.number_ = n;
}

inline value::value(const std::string &s) : type_(string_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(const array &a) : type_(array_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(const object &o) : type_(object_type), u_() { __builtin_trap() /* STUB: not implemented */; }

#if PICOJSON_USE_RVALUE_REFERENCE
inline value::value(std::string &&s) : type_(string_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(array &&a) : type_(array_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(object &&o) : type_(object_type), u_() { __builtin_trap() /* STUB: not implemented */; }
#endif

inline value::value(const char *s) : type_(string_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(const char *s, size_t len) : type_(string_type), u_() { __builtin_trap() /* STUB: not implemented */; }

inline void value::clear() { __builtin_trap() /* STUB: not implemented */; }

inline value::~value() { __builtin_trap() /* STUB: not implemented */; }

inline value::value(const value &x) : type_(x.type_), u_() { __builtin_trap() /* STUB: not implemented */; }

inline value &value::operator=(const value &x) { __builtin_trap() /* STUB: not implemented */; }

#if PICOJSON_USE_RVALUE_REFERENCE
inline value::value(value &&x) PICOJSON_NOEXCEPT : type_(null_type), u_() { __builtin_trap() /* STUB: not implemented */; }
inline value &value::operator=(value &&x) PICOJSON_NOEXCEPT { __builtin_trap() /* STUB: not implemented */; }
#endif
inline void value::swap(value &x) PICOJSON_NOEXCEPT { __builtin_trap() /* STUB: not implemented */; }

#define IS(ctype, jtype)                                                                                                           \
  template <> inline bool value::is<ctype>() const {                                                                               \
    return type_ == jtype##_type;                                                                                                  \
  }
IS(null, null)
IS(bool, boolean)
#ifdef PICOJSON_USE_INT64
IS(int64_t, int64)
#endif
IS(std::string, string)
IS(array, array)
IS(object, object)
#undef IS
template <> inline bool value::is<double>() const { __builtin_trap() /* STUB: not implemented */; }

#define GET(ctype, var)                                                                                                            \
  template <> inline const ctype &value::get<ctype>() const {                                                                      \
    PICOJSON_ASSERT("type mismatch! call is<type>() before get<type>()" && is<ctype>());                                           \
    return var;                                                                                                                    \
  }                                                                                                                                \
  template <> inline ctype &value::get<ctype>() {                                                                                  \
    PICOJSON_ASSERT("type mismatch! call is<type>() before get<type>()" && is<ctype>());                                           \
    return var;                                                                                                                    \
  }
GET(bool, u_.boolean_)
GET(std::string, *u_.string_)
GET(array, *u_.array_)
GET(object, *u_.object_)
#ifdef PICOJSON_USE_INT64
GET(double,
    (type_ == int64_type && (const_cast<value *>(this)->type_ = number_type, (const_cast<value *>(this)->u_.number_ = u_.int64_)),
     u_.number_))
GET(int64_t, u_.int64_)
#else
GET(double, u_.number_)
#endif
#undef GET

#define SET(ctype, jtype, setter)                                                                                                  \
  template <> inline void value::set<ctype>(const ctype &_val) {                                                                   \
    clear();                                                                                                                       \
    type_ = jtype##_type;                                                                                                          \
    setter                                                                                                                         \
  }
SET(bool, boolean, u_.boolean_ = _val;)
SET(std::string, string, u_.string_ = new std::string(_val);)
SET(array, array, u_.array_ = new array(_val);)
SET(object, object, u_.object_ = new object(_val);)
SET(double, number, u_.number_ = _val;)
#ifdef PICOJSON_USE_INT64
SET(int64_t, int64, u_.int64_ = _val;)
#endif
#undef SET

#if PICOJSON_USE_RVALUE_REFERENCE
#define MOVESET(ctype, jtype, setter)                                                                                              \
  template <> inline void value::set<ctype>(ctype && _val) {                                                                       \
    clear();                                                                                                                       \
    type_ = jtype##_type;                                                                                                          \
    setter                                                                                                                         \
  }
MOVESET(std::string, string, u_.string_ = new std::string(std::move(_val));)
MOVESET(array, array, u_.array_ = new array(std::move(_val));)
MOVESET(object, object, u_.object_ = new object(std::move(_val));)
#undef MOVESET
#endif

inline bool value::evaluate_as_boolean() const { __builtin_trap() /* STUB: not implemented */; }

inline const value &value::get(const size_t idx) const { __builtin_trap() /* STUB: not implemented */; }

inline value &value::get(const size_t idx) { __builtin_trap() /* STUB: not implemented */; }

inline const value &value::get(const std::string &key) const { __builtin_trap() /* STUB: not implemented */; }

inline value &value::get(const std::string &key) { __builtin_trap() /* STUB: not implemented */; }

inline bool value::contains(const size_t idx) const { __builtin_trap() /* STUB: not implemented */; }

inline bool value::contains(const std::string &key) const { __builtin_trap() /* STUB: not implemented */; }

inline std::string value::to_str() const { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> void copy(const std::string &s, Iter oi) { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> struct serialize_str_char {
  Iter oi;
  void operator()(char c) { __builtin_trap() /* STUB: not implemented */; }
};

template <typename Iter> void serialize_str(const std::string &s, Iter oi) { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> void value::serialize(Iter oi, bool prettify) const { __builtin_trap() /* STUB: not implemented */; }

inline std::string value::serialize(bool prettify) const { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> void value::_indent(Iter oi, int indent) { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> void value::_serialize(Iter oi, int indent) const { __builtin_trap() /* STUB: not implemented */; }

inline std::string value::_serialize(int indent) const { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> class input {
protected:
  Iter cur_, end_;
  bool consumed_;
  int line_;

public:
  input(const Iter &first, const Iter &last) : cur_(first), end_(last), consumed_(false), line_(1) { __builtin_trap() /* STUB: not implemented */; }
  int getc() { __builtin_trap() /* STUB: not implemented */; }
  void ungetc() { __builtin_trap() /* STUB: not implemented */; }
  Iter cur() const { __builtin_trap() /* STUB: not implemented */; }
  int line() const { __builtin_trap() /* STUB: not implemented */; }
  void skip_ws() { __builtin_trap() /* STUB: not implemented */; }
  bool expect(const int expected) { __builtin_trap() /* STUB: not implemented */; }
  bool match(const std::string &pattern) { __builtin_trap() /* STUB: not implemented */; }
};

template <typename Iter> inline int _parse_quadhex(input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename String, typename Iter> inline bool _parse_codepoint(String &out, input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename String, typename Iter> inline bool _parse_string(String &out, input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename Context, typename Iter> inline bool _parse_array(Context &ctx, input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename Context, typename Iter> inline bool _parse_object(Context &ctx, input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> inline std::string _parse_number(input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

template <typename Context, typename Iter> inline bool _parse(Context &ctx, input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }

class deny_parse_context {
public:
  bool set_null() { __builtin_trap() /* STUB: not implemented */; }
  bool set_bool(bool) { __builtin_trap() /* STUB: not implemented */; }
#ifdef PICOJSON_USE_INT64
  bool set_int64(int64_t) { __builtin_trap() /* STUB: not implemented */; }
#endif
  bool set_number(double) { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_string(input<Iter> &) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_array_item(input<Iter> &, size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_stop(size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_object_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_object_item(input<Iter> &, const std::string &) { __builtin_trap() /* STUB: not implemented */; }
};

class default_parse_context {
protected:
  value *out_;
  size_t depths_;

public:
  default_parse_context(value *out, size_t depths = DEFAULT_MAX_DEPTHS) : out_(out), depths_(depths) { __builtin_trap() /* STUB: not implemented */; }
  bool set_null() { __builtin_trap() /* STUB: not implemented */; }
  bool set_bool(bool b) { __builtin_trap() /* STUB: not implemented */; }
#ifdef PICOJSON_USE_INT64
  bool set_int64(int64_t i) { __builtin_trap() /* STUB: not implemented */; }
#endif
  bool set_number(double f) { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_string(input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_array_item(input<Iter> &in, size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_stop(size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_object_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_object_item(input<Iter> &in, const std::string &key) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_object_stop() { __builtin_trap() /* STUB: not implemented */; }

private:
  default_parse_context(const default_parse_context &);
  default_parse_context &operator=(const default_parse_context &);
};

class null_parse_context {
protected:
  size_t depths_;

public:
  struct dummy_str {
    void push_back(int) { __builtin_trap() /* STUB: not implemented */; }
  };

public:
  null_parse_context(size_t depths = DEFAULT_MAX_DEPTHS) : depths_(depths) { __builtin_trap() /* STUB: not implemented */; }
  bool set_null() { __builtin_trap() /* STUB: not implemented */; }
  bool set_bool(bool) { __builtin_trap() /* STUB: not implemented */; }
#ifdef PICOJSON_USE_INT64
  bool set_int64(int64_t) { __builtin_trap() /* STUB: not implemented */; }
#endif
  bool set_number(double) { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_string(input<Iter> &in) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_array_item(input<Iter> &in, size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_array_stop(size_t) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_object_start() { __builtin_trap() /* STUB: not implemented */; }
  template <typename Iter> bool parse_object_item(input<Iter> &in, const std::string &) { __builtin_trap() /* STUB: not implemented */; }
  bool parse_object_stop() { __builtin_trap() /* STUB: not implemented */; }

private:
  null_parse_context(const null_parse_context &);
  null_parse_context &operator=(const null_parse_context &);
};

// obsolete, use the version below
template <typename Iter> inline std::string parse(value &out, Iter &pos, const Iter &last) { __builtin_trap() /* STUB: not implemented */; }

template <typename Context, typename Iter> inline Iter _parse(Context &ctx, const Iter &first, const Iter &last, std::string *err) { __builtin_trap() /* STUB: not implemented */; }

template <typename Iter> inline Iter parse(value &out, const Iter &first, const Iter &last, std::string *err) { __builtin_trap() /* STUB: not implemented */; }

inline std::string parse(value &out, const std::string &s) { __builtin_trap() /* STUB: not implemented */; }

inline std::string parse(value &out, std::istream &is) { __builtin_trap() /* STUB: not implemented */; }

template <typename T> struct last_error_t { static std::string s; };
template <typename T> std::string last_error_t<T>::s;

inline void set_last_error(const std::string &s) { __builtin_trap() /* STUB: not implemented */; }

inline const std::string &get_last_error() { __builtin_trap() /* STUB: not implemented */; }

inline bool operator==(const value &x, const value &y) { __builtin_trap() /* STUB: not implemented */; }

inline bool operator!=(const value &x, const value &y) { __builtin_trap() /* STUB: not implemented */; }
}

#if !PICOJSON_USE_RVALUE_REFERENCE
namespace std {
template <> inline void swap(picojson::value &x, picojson::value &y) { __builtin_trap() /* STUB: not implemented */; }
}
#endif

inline std::istream &operator>>(std::istream &is, picojson::value &x) { __builtin_trap() /* STUB: not implemented */; }

inline std::ostream &operator<<(std::ostream &os, const picojson::value &x) { __builtin_trap() /* STUB: not implemented */; }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
