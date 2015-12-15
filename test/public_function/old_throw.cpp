
// Test from public function (derived and bases) .old().

#include "../aux_/oteststream.hpp"
#include <boost/contract/public_function.hpp>
#include <boost/contract/assert.hpp>
#include <boost/contract/base_types.hpp>
#include <boost/contract/override.hpp>
#include <boost/contract/guard.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <sstream>

boost::contract::aux::test::oteststream out;

struct c {
    static void static_invariant() { out << "c::static_inv" << std::endl; }
    void invariant() const { out << "c::inv" << std::endl; }
    
    struct err {};

    virtual void f(boost::contract::virtual_* v = 0) {
        boost::contract::guard c = boost::contract::public_function(v, this)
            .precondition([&] {
                out << "c::f::pre" << std::endl;
                BOOST_CONTRACT_ASSERT(false); // To check derived pre.
            })
            .old([&] {
                out << "c::f::old" << std::endl;
                throw c::err(); // Test .old() throws.
            })
            .postcondition([&] { out << "c::f::post" << std::endl; })
        ;
        out << "c::f::body" << std::endl;
    }
};

struct b
    #define BASES public c
    : BASES
{
    typedef BOOST_CONTRACT_BASE_TYPES(BASES) base_types;
    #undef BASES

    static void static_invariant() { out << "b::static_inv" << std::endl; }
    void invariant() const { out << "b::inv" << std::endl; }

    struct err {};

    virtual void f(boost::contract::virtual_* v = 0) /* override */ {
        boost::contract::guard c = boost::contract::public_function<override_f>(
                v, &b::f, this)
            .precondition([&] {
                out << "b::f::pre" << std::endl;
                BOOST_CONTRACT_ASSERT(false); // To check derived pre.
            })
            .old([&] {
                out << "b::f::old" << std::endl;
                throw b::err(); // Test .old() throws.
            })
            .postcondition([&] { out << "b::f::post" << std::endl; })
        ;
        out << "b::f::body" << std::endl;
    }
    BOOST_CONTRACT_OVERRIDE(f)
};

struct a
    #define BASES public b
    : BASES
{
    typedef BOOST_CONTRACT_BASE_TYPES(BASES) base_types;
    #undef BASES

    static void static_invariant() { out << "a::static_inv" << std::endl; }
    void invariant() const { out << "a::inv" << std::endl; }

    struct err {};

    void f(boost::contract::virtual_* v = 0) /* override */ {
        boost::contract::guard c = boost::contract::public_function<override_f>(
                v, &a::f, this)
            .precondition([&] { out << "a::f::pre" << std::endl; })
            .old([&] {
                out << "a::f::old" << std::endl;
                throw a::err(); // Test .old() throws.
            })
            .postcondition([&] { out << "a::f::post" << std::endl; })
        ;
        out << "a::f::body" << std::endl;
    }
    BOOST_CONTRACT_OVERRIDE(f)
};

int main() {
    std::ostringstream ok;

    boost::contract::set_postcondition_failed(
            [] (boost::contract::from) { throw; });
    
    a aa;
    b& ba = aa; // Test as virtual call via polymorphism.
    try {
        out.str("");
        ba.f();
        BOOST_TEST(false);
    } catch(c::err const&) {
        ok.str(""); ok
            << "c::static_inv" << std::endl
            << "c::inv" << std::endl
            << "b::static_inv" << std::endl
            << "b::inv" << std::endl
            << "a::static_inv" << std::endl
            << "a::inv" << std::endl

            << "c::f::pre" << std::endl
            << "b::f::pre" << std::endl
            << "a::f::pre" << std::endl
            
            << "c::f::old" << std::endl // Test this threw.
        ;
        BOOST_TEST(out.eq(ok.str()));
    } catch(...) { BOOST_TEST(false); }

    return boost::report_errors();
}

