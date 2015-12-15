
// Test throw from free function body.

#include "../aux_/oteststream.hpp"
#include <boost/contract/function.hpp>
#include <boost/contract/guard.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <sstream>

boost::contract::aux::test::oteststream out;

struct err {};

void f() {
    boost::contract::guard c = boost::contract::function()
        .precondition([&] { out << "f::pre" << std::endl; })
        .old([&] { out << "f::old" << std::endl; })
        .postcondition([&] { out << "f::post" << std::endl; })
    ;
    out << "f::body" << std::endl;
    throw err(); // Test body throws.
}

int main() {
    std::ostringstream ok;

    try {
        out.str("");
        f();
        BOOST_TEST(false);
    } catch(err const&) {
        ok.str(""); ok
            << "f::pre" << std::endl
            << "f::old" << std::endl
            << "f::body" << std::endl // Test this threw.
        ;
        BOOST_TEST(out.eq(ok.str()));
    } catch(...) { BOOST_TEST(false); }

    return boost::report_errors();
}

