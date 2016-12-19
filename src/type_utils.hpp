#ifndef TYPE_UTILS_HPP
#define TYPE_UTILS_HPP


namespace arr {

  /// Lets us define a compile time mapping from type to a unique
  /// number representing that type.
  template<typename T>
  struct TypeNumber {
    static const unsigned n = 0; // unsigned LLL ???
  };

  /// associated macro in order to write one liner mappings.
#define TYPE_NB(T, n_p) template <> struct TypeNumber<T> {    \
    static const unsigned n = n_p;                            \
  };

  /// Lets us define a compile time mapping from type to string.
  template<typename T>
  struct TypeName {
    constexpr static const char* s = "unknown";
  };

  /// associated macro in order to write one liner mappings.
#define TYPE_NAME(T, n_p) template <> struct TypeName<T> {    \
    constexpr static const char* s = n_p;                     \
  };


}

#endif
