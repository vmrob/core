#ifndef CORE_TYPE_TRAITS_HPP
#define CORE_TYPE_TRAITS_HPP

#include <type_traits>
#include <utility>

namespace core {
inline namespace v1 {

/* custom type traits */
/* Check if template class is given class */
template <class T, template <class...> class Class>
struct is_specialization_of : std::false_type { };

template <template <class...> class Class, class... Args>
struct is_specialization_of<Class<Args...>, Class> : std::true_type { };

/* extracts the class of a member function ponter */
template <class T> struct class_of { using type = T; };
template <class Signature, class Type>
struct class_of<Signature Type::*> { using type = Type; };

/* forward declaration */
template <class... Args> struct invokable;
template <class... Args> struct invoke_of;
template <class T> struct result_of; /* SFINAE result_of */

/* C++14 style aliases for standard traits */
template <class T>
using remove_volatile_t = typename std::remove_volatile<T>::type;

template <class T> using remove_const_t = typename std::remove_const<T>::type;
template <class T> using remove_cv_t = typename std::remove_cv<T>::type;

template <class T> using add_volatile_t = typename std::add_volatile<T>::type;
template <class T> using add_const_t = typename std::add_const<T>::type;
template <class T> using add_cv_t = typename std::add_cv<T>::type;

template <class T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

template <class T>
using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

template <class T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <class T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <class T> using add_pointer_t = typename std::add_pointer<T>::type;

template <class T>
using make_unsigned_t = typename std::make_unsigned<T>::type;
template <class T> using make_signed_t = typename std::make_signed<T>::type;

template <class T>
using remove_extent_t = typename std::remove_extent<T>::type;

template <class T>
using remove_all_extents_t = typename std::remove_all_extents<T>::type;

template <std::size_t Len, std::size_t Align>
using aligned_storage_t = typename std::aligned_storage<Len, Align>::type;

template <class T> using decay_t = typename std::decay<T>::type;

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

template <class T>
using underlying_type_t = typename std::underlying_type<T>::type;

template <class... T>
using common_type_t = typename std::common_type<T...>::type;

/* custom type trait specializations */
template <class... Args> using invoke_of_t = typename invoke_of<Args...>::type;
template <class T> using class_of_t = typename class_of<T>::type;

namespace impl {

struct undefined { undefined (...); };

/* Get the result of an attempt at the INVOKE expression */
/* fallback */
template <class... Args> auto invoke (undefined, Args&&...) -> undefined;

template <class Functor, class Object, class... Args>
auto invoke (Functor&& fun, Object&& obj, Args&&... args) -> enable_if_t<
  std::is_member_function_pointer<remove_reference_t<Functor>>::value and
  std::is_base_of<
    class_of_t<remove_reference_t<Functor>>,
    remove_reference_t<Object>
  >::value,
  decltype((std::forward<Object>(obj).*fun)(std::forward<Args>(args)...))
>;

template <class Functor, class Object, class... Args>
auto invoke (Functor&& fun, Object&& obj, Args&&... args) -> enable_if_t<
  std::is_member_function_pointer<remove_reference_t<Functor>>::value and
  not std::is_base_of<
    class_of_t<remove_reference_t<Functor>>,
    remove_reference_t<Object>
  >::value,
  decltype(((*std::forward<Object>(obj)).*fun)(std::forward<Args>(args)...))
>;

template <class Functor, class Object>
auto invoke (Functor&& functor, Object&& object) -> enable_if_t<
  std::is_member_object_pointer<remove_reference_t<Functor>>::value and
  std::is_base_of<
    class_of_t<remove_reference_t<Functor>>,
    remove_reference_t<Object>
  >::value,
  decltype(std::forward<Object>(object).*functor)
>;

template <class Functor, class Object>
auto invoke (Functor&& functor, Object&& object) -> enable_if_t<
  std::is_member_object_pointer<remove_reference_t<Functor>>::value and
  not std::is_base_of<
    class_of_t<remove_reference_t<Functor>>,
    remove_reference_t<Object>
  >::value,
  decltype((*std::forward<Object>(object)).*functor)
>;

template <class Functor, class... Args>
auto invoke (Functor&& functor, Args&&... args) ->
  decltype(std::forward<Functor>(functor)(std::forward<Args>(args)...));

template <bool, class... Args> struct invoke_of { };
template <class... Args>
struct invoke_of<true, Args...> {
  using type = decltype(invoke(std::declval<Args>()...));
};

} /* namespace impl */

template <class... Args> struct invokable : std::integral_constant<
  bool,
  not std::is_same<
    decltype(impl::invoke(std::declval<Args>()...)),
    impl::undefined
  >::value
> { };

template <class... Args> struct invoke_of :
  impl::invoke_of<invokable<Args...>::value, Args...>
{ };

template <class F, class... Args>
struct result_of<F(Args...)> : invoke_of<F, Args...> { };

template <class T> using result_of_t = typename result_of<T>::type;

}} /* namespace core::v1 */

#endif /* CORE_TYPE_TRAITS_HPP */