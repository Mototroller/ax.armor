#ifndef AX_ARMOR_HPP
#define AX_ARMOR_HPP

#include <memory>
#include <type_traits>
#include <utility>

#define LOG_HEAD "[armor]: "


#if __cplusplus < 201402L
    // Prior C++14 'constexpr' specifier for member functions implicitly implies 'const',
    //  so 'constexpr' declaration will cause redefinition error for non-const functions
    #define AX_ARMOR_CONSTEXPR_MUTABLE
#else
    #define AX_ARMOR_CONSTEXPR_MUTABLE constexpr
#endif


namespace ax { namespace armor {
    
    namespace details {
        
        template <bool Cond, class U, class V>
        using conditional_t = typename std::conditional<Cond, U, V>::type;
        
        template <class T>
        using decay_t = typename std::decay<T>::type;
        
        template <bool Cond, class T>
        using enable_if_t = typename std::enable_if<Cond, T>::type;
        
        
        template <class From, class To>
        auto probe_fundamental_conversion_is_narrowing(int) ->
            decltype(To{std::declval<From>()}, std::false_type());
        
        template <class From, class To>
        auto probe_fundamental_conversion_is_narrowing(...) -> std::true_type;
        
        struct fundamental_tag {};
        
        template <class From, class To>
        auto probe_conversion_is_narrowing(fundamental_tag) ->
            decltype(probe_fundamental_conversion_is_narrowing<From, To>(0));
        
        template <class From, class To>
        auto probe_conversion_is_narrowing(...) -> std::false_type;
        
        /// Checks if T{std::declval<From>()} yields narrowing conversion of fundamental types.
        /// Yields 'std::false_type' if types aren't fundamental ('constructible' trait isn't checked).
        /// NOTE: discards references, so check 'char& -> int&' yelds the same result as for 'char -> int' (false).
        template <class From, class To>
        struct is_conversion_narrowing {
        private:
            using From_ = typename std::remove_reference<From>::type;
            using To_   = typename std::remove_reference<To  >::type;
            
            enum : bool { types_are_fundamental =
                std::is_fundamental<From_>::value && std::is_fundamental<To_>::value };
            
            using tag_type = conditional_t<types_are_fundamental, fundamental_tag, int>;
            
        public:
            enum : bool { value = decltype(probe_conversion_is_narrowing<From_, To_>(tag_type{}))::value };
        };
        
        
        struct AllowNarrowingConversions_ {};
        
        struct AllowImplicitConstructors_ {};
        
        
        /**
         * Strong typedef wrapper.
         *  Grants strong typing and conversion control
         *  even for types which underlying native type are same.
         */
        template <class T, class Tag, template <class, class> class... Mixins>
        struct named_type : public Mixins<T, named_type<T, Tag, Mixins...>>... {
        private:
            T value_;
            
            
            /// NOTE: must be argument dependent to avoid 'incomplete type' errors
            template <class Dummy_>
            constexpr static bool is_constructor_implicit() {
                return std::is_base_of<AllowImplicitConstructors_, named_type>::value; }
            
            /// 'Variadic concept': checks if generic constructor from provided arguments
            /// must be enabled for overloading resolution by `std::enable_if`.
            template <class Head, class... Tail>
            constexpr static bool is_construction_allowed_from() {
                return
                    // If attempt to instantiate c-tor will cause error:
                    (!std::is_constructible<T, Head, Tail...>::value) ? false :
                    
                    // If this is forwarding c-tor of underlying type:
                    (sizeof...(Tail) > 0) ? true :
                    
                    // Narrowing conversions are disallowed (can be allowed explicitly):
                    (is_conversion_narrowing<Head, T>::value && !std::is_base_of<AllowNarrowingConversions_, named_type>::value) ? false :
                    
                    true;
            }
            
            /// 'Concept': checks if generic assignment operator from provided argument
            /// must be enabled for overloading resolution by `std::enable_if`.
            template <class Head>
            constexpr static bool is_assignment_allowed_from() {
                return
                    // If attempt to instantiate 'operator=' will cause error:
                    (!std::is_assignable<T&, Head>::value) ? false :
                    
                    (is_conversion_narrowing<Head, T>::value && !std::is_base_of<AllowNarrowingConversions_, named_type>::value) ? false :
                    
                    true;
            }
            
        public:
            using value_type = T;
            
            named_type() = default;
            
            named_type(named_type const&)             = default;
            named_type& operator=(named_type const&)  = default;
            
            named_type(named_type&&)              = default;
            named_type& operator=(named_type&&)   = default;
            
            ~named_type() = default;
            
            
            // TODO: f*ck std::initializer_list!
            // template <class U,
            //     class = std::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            // constexpr named_type(std::initializer_list<U> list)
            //     noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>>::value):
            //     value_(list) {}
            
            template <class Head, class... Tail,
                enable_if_t<is_constructor_implicit<Head>(), bool> = true,
                enable_if_t<is_construction_allowed_from<Head, Tail...>(), bool> = true>
            constexpr named_type(Head&& h, Tail&&... t)
                noexcept(std::is_nothrow_constructible<T, Head, Tail...>::value):
                value_(std::forward<Head>(h), std::forward<Tail>(t)...) {}
            
            template <class Head, class... Tail,
                enable_if_t<!is_constructor_implicit<Head>(), bool> = true,
                enable_if_t<is_construction_allowed_from<Head, Tail...>(), bool> = true>
            explicit constexpr named_type(Head&& h, Tail&&... t)
                noexcept(std::is_nothrow_constructible<T, Head, Tail...>::value):
                value_(std::forward<Head>(h), std::forward<Tail>(t)...) {}
            
            
            template <class U,
                enable_if_t<is_assignment_allowed_from<U>(), bool> = true>
            AX_ARMOR_CONSTEXPR_MUTABLE named_type& operator=(U&& value)
                noexcept(std::is_nothrow_assignable<T&, U>::value)
            { value_ = std::forward<U>(value); return *this; }
            
            
            AX_ARMOR_CONSTEXPR_MUTABLE T&  value() &  noexcept { return value_; }
            AX_ARMOR_CONSTEXPR_MUTABLE T&& value() && noexcept { return std::move(value_); }
            
            constexpr T const&  value() const &  noexcept { return value_; }
            constexpr T const&& value() const && noexcept { return std::move(value_); }
        };
        
        
        template <class T>
        struct sizeof_of : std::integral_constant<std::size_t, sizeof(T)> {};
        
        template <class T>
        struct alignof_of : std::integral_constant<std::size_t, alignof(T)> {};
        
        template <template <class...> class Fun, class... Args>
        struct bind_tail { template <class T> using type = Fun<T, Args...>; };
        
        // TODO: is it possible?
        // template <template <class...> class Trait, class... Args>
        // using bind_trait_t = typename bind_tail<Trait, Args...>::template type;
        
        template <class, class>
        constexpr bool check_traits_same() { return true; }
        
        template <class U, class V,
            template <class...> class Head,
            template <class...> class... Tail>
        constexpr bool check_traits_same() {
            static_assert(Head<U>::value == Head<V>::value, LOG_HEAD "Traits aren't equal");
            return check_traits_same<U, V, Tail...>();
        }
        
        
        template <class>
        struct dummy_tag_ {};
        
        template <class U>
        constexpr bool named_type_default_test() {
            using namespace std;
            using V = named_type<U, dummy_tag_<U>>;
            
            // NOTE: tests are too strict and will fail
            //  if mixins will add fields or change semantics.
            return check_traits_same<U, V,
                sizeof_of,
                alignof_of,
                
                is_trivial,
                is_standard_layout,
                is_trivially_copyable,
                
                is_default_constructible,
                is_nothrow_default_constructible,
                is_trivially_default_constructible,
                
                is_destructible,
                is_nothrow_destructible,
                is_trivially_destructible,
                
                is_copy_constructible,
                is_nothrow_copy_constructible,
                is_trivially_copy_constructible,
                
                is_move_constructible,
                is_nothrow_move_constructible,
                is_trivially_move_constructible,
                
                bind_tail<is_constructible, U>::template type,
                bind_tail<is_nothrow_constructible, U>::template type,
                
                bind_tail<is_constructible, U&>::template type,
                bind_tail<is_nothrow_constructible, U&>::template type,
                
                bind_tail<is_constructible, U&&>::template type,
                bind_tail<is_nothrow_constructible, U&&>::template type,
                
                is_copy_assignable,
                is_nothrow_copy_assignable,
                
                is_move_assignable,
                is_nothrow_move_assignable
            
            >() && check_traits_same<U&, V&,
                bind_tail<is_assignable, U>::template type,
                bind_tail<is_nothrow_assignable, U>::template type,
                
                bind_tail<is_assignable, U&>::template type,
                bind_tail<is_nothrow_assignable, U&>::template type,
                
                bind_tail<is_assignable, U&&>::template type,
                bind_tail<is_nothrow_assignable, U&&>::template type>();
        }
        
        template <class H1, class H2, class... Tail>
        constexpr bool named_type_default_test() {
            return named_type_default_test<H1>() && named_type_default_test<H2, Tail...>(); }
        
        /// Calls basic 'static_assert' tests over several built-in types
        enum class named_type_default_test_suite : bool { success =
            named_type_default_test<bool, int, double, void*, int(*)(int), named_type<int, struct Int_tag_>>() };
        
    } // details
    
    
    namespace utility {
        
        using armor::details::is_conversion_narrowing;
        
        using armor::details::bind_tail;
        
        using armor::details::check_traits_same;
        
        using armor::details::named_type_default_test;
        
    } // utility
    
    
    namespace mixins {
        
        /// Enables constructors and assignment operators
        /// which can cause narrowing conversion to underlying type
        template <class, class>
        struct AllowNarrowingConversions : armor::details::AllowNarrowingConversions_ {};
        
        /// Makes constructors from underlying type implicit
        template <class, class>
        struct AllowImplicitConstructors : armor::details::AllowImplicitConstructors_ {};
        
        
        /// Provides 'operator->()'
        template <class T, class Base>
        struct OperatorArrowAccess {
            static_assert(!std::is_reference<T>::value,
                LOG_HEAD "'operator->()' for references is illegal");
            
            AX_ARMOR_CONSTEXPR_MUTABLE T* operator->() noexcept {
                return std::addressof(static_cast<Base*>(this)->value()); }
            
            constexpr T const* operator->() const noexcept {
                return std::addressof(static_cast<Base const*>(this)->value()); }
        };
        
        /// Provides 'operator*()'
        template <class T, class Base>
        struct OperatorDereferenceAccess {
            AX_ARMOR_CONSTEXPR_MUTABLE T& operator*() & noexcept {
                return static_cast<Base*>(this)->value(); }
            
            constexpr T const& operator*() const & noexcept {
                return static_cast<Base const*>(this)->value(); }
            
            AX_ARMOR_CONSTEXPR_MUTABLE T&& operator*() && noexcept {
                return static_cast<Base*>(this)->value(); }
            
            constexpr T const&& operator*() const && noexcept {
                return static_cast<Base const*>(this)->value(); }
        };
        
        /// Provides both 'operator->()' and 'operator*()'
        template <class T, class Base>
        struct OperatorAccess:
            OperatorArrowAccess<T, Base>,
            OperatorDereferenceAccess<T, Base>
        {};
        
        
        /// Provides implicit 'operator T&()' and 'operator T const&()'
        template <class T, class Base>
        struct ImplicitConvertible {
            AX_ARMOR_CONSTEXPR_MUTABLE operator T&() noexcept {
                return static_cast<Base*>(this)->value(); }
            
            constexpr operator T const&() const noexcept {
                return static_cast<Base const*>(this)->value(); }
        };
        
        /// Provides explicit 'operator T&()' and 'operator T const&()'
        template <class T, class Base>
        struct ExplicitConvertible {
            AX_ARMOR_CONSTEXPR_MUTABLE operator T&() noexcept {
                return static_cast<Base*>(this)->value(); }
            
            explicit constexpr operator T const&() const noexcept {
                return static_cast<Base const*>(this)->value(); }
        };
        
        
        template <class T, class Base>
        struct EqualLessComparable {
            constexpr bool operator<(Base const& rh) const
                noexcept(noexcept(std::declval<T>() < std::declval<T>())) {
                    return static_cast<Base const*>(this)->value() < rh.value(); }
            
            constexpr bool operator==(Base const& rh) const
                noexcept(noexcept(std::declval<T>() == std::declval<T>())) {
                    return static_cast<Base const*>(this)->value() == rh.value(); }
            
            constexpr bool operator!=(Base const& rh) const
                noexcept(noexcept(std::declval<T>() == std::declval<T>())) {
                    return static_cast<Base const*>(this)->value() != rh.value(); }
        };
        
        template <class T, class Base>
        struct Hashable {
            struct Hash : private std::hash<T> {
                std::size_t operator()(Base const& obj) const
                    noexcept(noexcept(std::hash<T>{}(std::declval<T>())))
                {
                    return std::hash<T>::operator()(obj.value());
                }
            };
        };
        
        
        /**
         * Provides an ability to emulate named parameters (Python-like way):
         *  void foo(Name, Age) {}
         *  foo(Name::arg = "Mike Wazowski", Age::arg = 42);
         * 
         * Can be combined with another mixins such as:
         *  OperatorAccess, ImplicitConvertible etc.
         * 
         * Note lifetime extension rules:
         *  named_type<int const&> x{42}; x.value(); // UB
         */
        template <class T, class Base>
        struct NamedArgument {
        private:
            struct fwd_t {
                /// Works like 'named_type' factory,
                /// @returns exactly 'named_type<T, ...>'
                template <class U>
                constexpr Base operator=(U&& val) const & {
                    return Base{std::forward<U>(val)}; }
                
                fwd_t() = default;
                ~fwd_t() = default;
                
                fwd_t(fwd_t const&)             = delete;
                fwd_t& operator=(fwd_t const&)  = delete;
                
                fwd_t(fwd_t&&)              = delete;
                fwd_t& operator=(fwd_t&&)   = delete;
            };
            
        public:
            /// Typedef to declare own named arguments
            using argument_t = fwd_t;
            
            /// Endpoint to use forwarding assignment
            constexpr static argument_t arg{};
        };
        
        template <class T, class Base>
        constexpr typename NamedArgument<T, Base>::fwd_t NamedArgument<T, Base>::arg;
        
    } // mixins
    
    
    using details::named_type;
    
    
} // armor
} // ax


#undef LOG_HEAD

#endif // guard AX_ARMOR_HPP
