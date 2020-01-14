#define BOOST_TEST_MODULE ax.armor tests
#include <boost/test/included/unit_test.hpp>

#include <unordered_map>

#include "ax.armor.hpp"


template <class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }


BOOST_AUTO_TEST_SUITE(test_suite_armor)

    BOOST_AUTO_TEST_CASE( case_test )
    {
        using namespace std;
    
        using namespace ax;
        using namespace ax::armor;
        
        {
            using namespace armor::utility;
            
            static_assert(!is_conversion_narrowing<int, int>::value, "");
            static_assert( is_conversion_narrowing<int64_t, int32_t>::value, "");
            static_assert(!is_conversion_narrowing<int32_t, int64_t>::value, "");
            
            static_assert( is_conversion_narrowing<int, char>::value, "");
            static_assert(!is_conversion_narrowing<char, int>::value, "");
            
            static_assert( is_conversion_narrowing<unsigned, int>::value, "");
            static_assert( is_conversion_narrowing<int, unsigned>::value, "");
            
            static_assert( is_conversion_narrowing<long long, unsigned short>::value, "");
            static_assert(!is_conversion_narrowing<unsigned short, long long>::value, "");
            
            static_assert(!is_conversion_narrowing<int&, int&>::value, "");
            static_assert( is_conversion_narrowing<int&, double&>::value, "");
            
            static_assert( is_conversion_narrowing<int&, char&>::value, "");
            static_assert(!is_conversion_narrowing<char&, int&>::value, "");
            
            // Not even constructible, checking
            static_assert(!is_conversion_narrowing<double, std::string>::value, "");
            static_assert(!is_conversion_narrowing<std::string, double>::value, "");
        }
        
        {
            using namespace armor::utility;
            
            enum My_enum { value };
            
            struct My_struct { int a; std::string b; };
            
            struct No_create { No_create() = delete; };
            
            struct No_default { explicit No_default(No_create) {} };
            
            named_type_default_test<
                std::array<int, 42>,
                std::vector<int>,
                
                std::string,
                std::array<std::string, 42>,
                std::vector<std::string>,
                
                My_enum,
                My_struct,
                No_create,
                No_default,
                std::unique_ptr<int>,
                std::unique_ptr<std::string>
            >();
        }
        
        {
            using Int = named_type<int, struct Int_tag>;
            using Uint = named_type<unsigned, struct Uint_tag>;
            
            using IntRef = named_type<int&, struct IntRef_tag>;
            using IntCRef = named_type<int const&, struct IntRef_tag>;
            
            static_assert(is_constructible<Int, Int>::value, "");
            
            static_assert(is_constructible<Int, int>::value, "");
            static_assert(is_constructible<Int, short>::value, "");
            static_assert(is_constructible<Int, signed char>::value, "");
            static_assert(is_constructible<Int, unsigned char>::value, "");
            
            static_assert(is_assignable<Int&, int>::value, "");
            static_assert(is_assignable<Int&, short>::value, "");
            static_assert(is_assignable<Int&, signed char>::value, "");
            static_assert(is_assignable<Int&, unsigned char>::value, "");
            
            static_assert(!is_constructible<Int, unsigned>::value, "");
            static_assert(!is_constructible<Int, unsigned long long>::value, "");
            
            static_assert(!is_assignable<Int&, unsigned>::value, "");
            static_assert(!is_assignable<Int&, unsigned long long>::value, "");
            
            static_assert(!is_constructible<Int, Uint&>::value, "");
            static_assert(!is_constructible<Uint, Int&>::value, "");
            static_assert(!is_assignable<Int&, Uint&>::value, "");
            static_assert(!is_assignable<Uint&, Int&>::value, "");
            
            static_assert( is_constructible<IntRef, int&>::value, "");
            static_assert(!is_constructible<IntRef, int >::value, "");
            
            static_assert( is_constructible<IntCRef, int&>::value, "");
            static_assert( is_constructible<IntCRef, int >::value, "");
            
            static_assert(!is_constructible<IntRef, float&>::value, "");
            static_assert(!is_constructible<IntRef, float >::value, "");
            
            static_assert(!is_constructible<IntCRef, float&>::value, "");
            static_assert(!is_constructible<IntCRef, float >::value, "");
        }
        
        {
            using namespace armor::utility;
            
            using Int = named_type<int, struct Int_tag>;
            using Int_conv = named_type<int, struct Int_conv_tag, mixins::ImplicitConvertible>;
            using Int_cons = named_type<int, struct Int_cons_tag, mixins::AllowImplicitConstructors>;
            using Int_narr = named_type<int, struct Int_narr_tag, mixins::AllowNarrowingConversions>;
            
            static_assert( is_constructible<Int, Int_conv&>::value, "");
            static_assert(!is_constructible<Int, Int_cons&>::value, "");
            
            static_assert( is_convertible<int, Int_cons>::value, "");
            static_assert(!is_convertible<int, Int_conv>::value, "");
            
            static_assert( is_convertible<Int_conv, int>::value, "");
            static_assert(!is_convertible<Int_cons, int>::value, "");
            
            static_assert( is_constructible<int, Int_conv&>::value, "");
            static_assert(!is_constructible<int, Int_cons&>::value, "");
            
            static_assert( is_constructible<Int_narr, unsigned>::value, "");
            static_assert( is_constructible<Int_narr, unsigned long>::value, "");
            static_assert( is_constructible<Int_narr, Int_conv>::value, "");
            
            constexpr Int_conv A{1}, B{2};
            constexpr int c = A + B;
            constexpr Int_cons C = c;
            
            static_assert(c == 3 && C.value() == 3, "");
            
            
            using Int2 = named_type<int, struct Int2_tag>;
            
            static_assert(!is_constructible<Int, Int2>::value, "");
            static_assert(!is_constructible<Int, Int2>::value, "");
            static_assert(!is_constructible<Int2, Int>::value, "");
            static_assert(!is_constructible<Int2, Int>::value, "");
        }
        
        {
            using String = named_type<std::string, struct String_tag,
                mixins::OperatorAccess,
                mixins::ImplicitConvertible>;
            
            static_assert(std::is_assignable<String, std::string>::value, "");
            static_assert(std::is_assignable<std::string, String>::value, "");
            
            String s("Hello");
            s = *s + std::string(", world");
            s->append(String("!"));
            BOOST_REQUIRE_EQUAL(*s, "Hello, world!");
            
            s->assign("");
            BOOST_REQUIRE(s->empty());
            
            String s2(4, '#');
            BOOST_REQUIRE_EQUAL(*s2, "####");
        }
        
        {
            using Uptr = named_type<std::unique_ptr<std::string>, struct Uptr_tag,
                mixins::OperatorAccess,
                mixins::AllowImplicitConstructors>;
            
            Uptr p1 = make_unique<std::string>(4, '!');
            BOOST_REQUIRE_EQUAL(**p1, "!!!!");
            
            (*p1)->clear();
            BOOST_REQUIRE((*p1)->empty());
        }
        
        {
            using IntEH = named_type<int, struct IntEH_tag,
                mixins::OperatorAccess, mixins::EqualLessComparable, mixins::Hashable>;
            
            {
                std::map<IntEH, int> map1;
                std::map<int, int> map2;
                
                using v1 = decltype(map1)::value_type;
                using v2 = decltype(map2)::value_type;
                
                for(int i = 0; i < 1000; ++i) {
                    map1.emplace(IntEH(i), i);
                    map2.emplace(i, i);
                }
                
                bool const eq = std::equal(map1.begin(), map1.end(), map2.begin(),
                    [](v1 const& lh, v2 const& rh){ return (*lh.first == rh.first) && (lh.second == rh.second); });
                
                BOOST_REQUIRE(eq);
            }
            
            {
                std::unordered_map<IntEH, int, IntEH::Hash> map1;
                std::unordered_map<int, int> map2;
                
                using v1 = decltype(map1)::value_type;
                using v2 = decltype(map2)::value_type;
                
                for(int i = 0; i < 1000; ++i) {
                    map1.emplace(IntEH(i), i);
                    map2.emplace(i, i);
                }
                
                bool const eq = std::equal(map1.begin(), map1.end(), map2.begin(),
                    [](v1 const& lh, v2 const& rh){ return (*lh.first == rh.first) && (lh.second == rh.second); });
                
                BOOST_REQUIRE(eq);
            }
        }
        
        {
            using Int = named_type<int, struct Int_tag,
                mixins::NamedArgument, mixins::OperatorDereferenceAccess>;
            
            using IntRef = named_type<int&, struct IntRef_tag,
                mixins::NamedArgument, mixins::OperatorDereferenceAccess>;
            
            using IntCRef = named_type<int const&, struct IntRef_tag,
                mixins::NamedArgument, mixins::OperatorDereferenceAccess>;
            
            using Iptr = named_type<std::unique_ptr<int>, struct Iptr_tag,
                mixins::NamedArgument, mixins::OperatorDereferenceAccess>;
            
            auto const fun1 = [](Int x)         { return    *x; };
            auto const fun2 = [](IntRef ref)    { return  *ref; };
            auto const fun3 = [](IntCRef cref)  { return *cref; };
            auto const fun4 = [](Iptr px)       { return  **px; };
            
            int tmp = 23;
            
            BOOST_REQUIRE(fun1(Int{42}) == fun1(Int::arg = 42));
            
            BOOST_REQUIRE(fun2(IntRef{tmp}) == fun2(IntRef::arg = tmp));
            
            BOOST_REQUIRE(fun3(IntCRef{42}) == fun3(IntCRef::arg = 42));
            
            BOOST_REQUIRE(fun4(Iptr{make_unique<int>(42)}) == fun4(Iptr::arg = make_unique<int>(42)));
            
            
            using IIptr = named_type<Iptr, struct IIptr_tag,
                mixins::NamedArgument, mixins::OperatorDereferenceAccess>;
            
            static_assert(std::is_same<Iptr, IIptr::value_type>::value, "");
            static_assert(std::is_same<std::unique_ptr<int>, Iptr::value_type>::value, "");
            
            static_assert( std::is_constructible<IIptr, Iptr&&>::value, "");
            static_assert(!std::is_constructible<IIptr, Iptr const&>::value, "");
            
            static_assert( std::is_constructible<IIptr, std::unique_ptr<int>&&>::value, "");
            static_assert(!std::is_constructible<IIptr, std::unique_ptr<int> const&>::value, "");
            
            auto const fun5 = [](IIptr px) -> Iptr { return std::move(px.value()); };
            auto const fun6 = [](Iptr px) -> std::unique_ptr<int> { return std::move(px.value()); };
            
            Iptr i1{make_unique<int>(42)};
            auto i2 = fun5(IIptr::arg = std::move(i1));
            BOOST_REQUIRE(**i2 == 42);
            
            auto i3 = fun6(std::move(i2));
            BOOST_REQUIRE(*i3 == 42);
            
            BOOST_REQUIRE(!*i1 && !*i2);
        }
        
        {
            using Age = named_type<int, struct Age_tag, mixins::NamedArgument>;
            using Name = named_type<std::string, struct Name_tag, mixins::NamedArgument>;
            
            static const Age::argument_t My_age;
            static const Name::argument_t My_name;
            
            auto fun = ([](Age age, Name name){
                BOOST_REQUIRE(age.value() == 42);
                BOOST_REQUIRE(name.value() == "Ololosha");
            });
            
            fun(My_age = 42, My_name = "Ololosha");
        }
    }

BOOST_AUTO_TEST_SUITE_END()
