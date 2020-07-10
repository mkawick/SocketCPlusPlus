
// TypeRegistry.h

//https://stackoverflow.com/questions/4790721/c-type-registration-at-compile-time-trick
// The maximum number of types that can be registered with the same tag.
enum { kMaxRegisteredTypes = 10 };

template <int N>
struct Rank : Rank<N - 1> {};

template <>
struct Rank<0> {};

// Poor man's MPL vector.
template <class... Ts>
struct TypeList {
    static const int size = sizeof...(Ts);
};

template <class List, class T>
struct Append;

template <class... Ts, class T>
struct Append<TypeList<Ts...>, T> {
    typedef TypeList<Ts..., T> type;
};

template <class Tag>
TypeList<> GetTypes(Tag*, Rank<0>) { return {}; }

// Evaluates to TypeList of all types previously registered with
// REGISTER_TYPE macro with the same tag.
#define GET_REGISTERED_TYPES(Tag) \
  decltype(GetTypes(static_cast<Tag*>(nullptr), Rank<kMaxRegisteredTypes>()))

// Appends Type to GET_REGISTERED_TYPES(Tag).
#define REGISTER_TYPE(Tag, Type)                              \
  inline Append<GET_REGISTERED_TYPES(Tag), Type>::type        \
  GetTypes(Tag*, Rank<GET_REGISTERED_TYPES(Tag)::size + 1>) { \
    return {};                                                \
  }                                                           \
  static_assert(true, "")

/* usage

struct IntegralTypes;
struct FloatingPointTypes;

// Initially both type lists are empty.
static_assert(std::is_same<GET_REGISTERED_TYPES(IntegralTypes), TypeList<>>::value, "");
static_assert(std::is_same<GET_REGISTERED_TYPES(FloatingPointTypes), TypeList<>>::value, "");

// Add something to both lists.
REGISTER_TYPE(IntegralTypes, int);
REGISTER_TYPE(FloatingPointTypes, float);
static_assert(std::is_same<GET_REGISTERED_TYPES(IntegralTypes), TypeList<int>>::value, "");
static_assert(std::is_same<GET_REGISTERED_TYPES(FloatingPointTypes), TypeList<float>>::value, "");

// Add more types.
REGISTER_TYPE(IntegralTypes, long);
REGISTER_TYPE(FloatingPointTypes, double);
static_assert(std::is_same<GET_REGISTERED_TYPES(IntegralTypes), TypeList<int, long>>::value, "");
static_assert(std::is_same<GET_REGISTERED_TYPES(FloatingPointTypes), TypeList<float, double>>::value, "");
*/