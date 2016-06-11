
// Test public function subcontracting via virtual functions.

#include "contracts.hpp"
#include <boost/preprocessor/control/iif.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <sstream>

int main() {
    std::ostringstream ok;
    
    a aa;
    c& ca = aa; // Test polymorphic virtual call (via reference to base c).
    s_type s; s.value = "A";
    out.str("");
    result_type& r = ca.f(s);

    ok.str(""); ok
        #ifndef BOOST_CONTRACT_NO_ENTRY_INVARIANTS
            << "d::static_inv" << std::endl
            << "d::inv" << std::endl
            << "e::static_inv" << std::endl
            << "e::inv" << std::endl
            << "c::static_inv" << std::endl
            << "c::inv" << std::endl
            << "a::static_inv" << std::endl
            << "a::inv" << std::endl
        #endif
        #ifndef BOOST_CONTRACT_NO_PRECONDITIONS
            << "d::f::pre" << std::endl
            << "e::f::pre" << std::endl
            << "c::f::pre" << std::endl
            << "a::f::pre" << std::endl
        #endif
        #ifndef BOOST_CONTRACT_NO_POSTCONDITIONS
            << "d::f::old" << std::endl
            << "e::f::old" << std::endl
            << "c::f::old" << std::endl
            << "a::f::old" << std::endl
        #endif
        << "a::f::body" << std::endl
        #ifndef BOOST_CONTRACT_NO_EXIT_INVARIANTS
            << "d::static_inv" << std::endl
            << "d::inv" << std::endl
            << "e::static_inv" << std::endl
            << "e::inv" << std::endl
            << "c::static_inv" << std::endl
            << "c::inv" << std::endl
            << "a::static_inv" << std::endl
            << "a::inv" << std::endl
        #endif
        #ifndef BOOST_CONTRACT_NO_POSTCONDITIONS
            << "d::f::old" << std::endl
            << "d::f::post" << std::endl
            << "e::f::old" << std::endl
            << "e::f::post" << std::endl
            << "c::f::old" << std::endl
            << "c::f::post" << std::endl
            // No old call here because not a base object.
            << "a::f::post" << std::endl
        #endif
    ;
    BOOST_TEST(out.eq(ok.str()));

    #ifdef BOOST_CONTRACT_NO_POSTCONDITIONS
        #define BOOST_CONTRACT_TEST_post 0
    #else
        #define BOOST_CONTRACT_TEST_post 1
    #endif
    
    BOOST_TEST_EQ(r.value, "A");
    BOOST_TEST_EQ(s.value, "acde");
    BOOST_TEST_EQ(s.copies(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 4u, 0u));
    BOOST_TEST_EQ(s.evals(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 4u, 0u));
    BOOST_TEST_EQ(s.ctors(), s.dtors() + 1); // 1 local var.

    // Cannot access x via ca, but only via aa.
    BOOST_TEST_EQ(aa.x.value, "aA");
    BOOST_TEST_EQ(aa.x.copies(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(aa.x.evals(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(aa.x.ctors(), aa.x.dtors() + 1); // 1 data member.

    BOOST_TEST_EQ(ca.y.value, "cA");
    BOOST_TEST_EQ(ca.y.copies(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.y.evals(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.y.ctors(), ca.y.dtors() + 1); // 1 data member.
    
    BOOST_TEST_EQ(ca.t<'d'>::z.value, "dA");
    BOOST_TEST_EQ(ca.t<'d'>::z.copies(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.t<'d'>::z.evals(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.t<'d'>::z.ctors(), ca.t<'d'>::z.dtors() + 1); // 1 member.

    BOOST_TEST_EQ(ca.t<'e'>::z.value, "eA");
    BOOST_TEST_EQ(ca.t<'e'>::z.copies(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.t<'e'>::z.evals(),
            BOOST_PP_IIF(BOOST_CONTRACT_TEST_post, 1u, 0u));
    BOOST_TEST_EQ(ca.t<'e'>::z.ctors(), ca.t<'e'>::z.dtors() + 1); // 1 member.

    #undef BOOST_CONTRACT_TEST_post
    return boost::report_errors();
}

