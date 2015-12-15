
#ifndef BOOST_CONTRACT_EXCEPTION_HPP_
#define BOOST_CONTRACT_EXCEPTION_HPP_

/** @file */

/** @cond */
#include <boost/function.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/config.hpp>
#include <exception>
#include <sstream>
#include <string>
#include <iostream>
/** @endcond */

namespace boost { namespace contract {

// Placeholder base class to group all this lib exceptions.
// IMPORTANT: Must not inherit from std::exception as derived exceptions will.
class exception {};

// Rationale: boost::bad_any_cast exception does not print from/to type names,
// so throw custom exception.
class bad_virtual_result_cast : // Copyable (as string, etc.).
        public std::bad_cast, public boost::contract::exception {
public:
    explicit bad_virtual_result_cast(char const* from_type_name,
            char const* to_type_name) {
        std::ostringstream text;
        text
            << "incompatible contracted virtual function result type "
            << "conversion from '" << from_type_name << "' to '"
            << to_type_name  << "'"
        ;
        what_ = text.str();
    }

    virtual char const* what() const BOOST_NOEXCEPT { return what_.c_str(); }

private:
    std::string what_;
};

class assertion_failure : // Copyable (as string, etc.).
        public std::exception, public boost::contract::exception {
public:
    explicit assertion_failure(char const* const file = "",
            unsigned long const line = 0, char const* const code = "") :
        file_(file), line_(line), code_(code)
    { init(); }
    
    explicit assertion_failure(char const* const code) :
            file_(""), line_(0), code_(code)
    { init(); }

    virtual ~assertion_failure() {}

    // Return `assertion "XYZ" failed: file "ABC", line 123`.
    virtual char const* what() const BOOST_NOEXCEPT { return what_.c_str(); }

    char const* const file() const { return file_; }
    unsigned long line() const { return line_; }
    char const* const code() const { return code_; }

private:
    void init() {
        std::ostringstream text;
        text << "assertion";
        if(std::string(code_) != "") text << " \"" << code_ << "\"";
        text << " failed";
        if(std::string(file_) != "") {
            text << ": file \"" << file_ << "\"";
            if(line_ != 0) text << ", line " << line_;
        }
        what_ = text.str();
    }

    char const* const file_;
    unsigned long const line_;
    char const* const code_;
    std::string what_;
};

enum from {
    from_constructor,
    from_destructor,
    from_function
};

// TODO: Even if it might be confused with assertion_failure (which is an exception and not a function), all these function names are more readable as ..._failure instead of ..._failed (so they should be renamed that way).

// Use Boost.Function to handle also lambdas, binds, etc,
typedef boost::function<void (from)> assertion_failed_handler;

namespace exception_ {
    enum failed_key { pre_key, post_key, entry_inv_key, exit_inv_key };

    template<failed_key Key>
    void default_handler(from) {
        std::string s = "";
        switch(Key) {
            case pre_key: s = "precondition "; break;
            case post_key: s = "postcondition "; break;
            case entry_inv_key: s = "entry invariant "; break;
            case exit_inv_key: s = "exit invariant "; break;
            // No default (so compiler warning/error on missing enum case).
        }
        try {
            throw;
        } catch(boost::contract::assertion_failure const& error) {
            // what = "assertion '...' failed: ...".
            std::cerr << s << error.what() << std::endl;
        } catch(...) {
            std::cerr << s << "checking threw following exception:" << std::endl
                    << boost::current_exception_diagnostic_information();
        }
        std::terminate(); // Default handlers log and call terminate.
    }

    // TODO: These (and some of the related def code) should be moved in a .cpp.
    //boost::mutex pre_failed_mutex;
    assertion_failed_handler pre_failed_handler =
            &default_handler<pre_key>;
    
    //boost::mutex post_failed_mutex;
    assertion_failed_handler post_failed_handler =
            &default_handler<post_key>;
    
    //boost::mutex entry_inv_failed_mutex;
    assertion_failed_handler entry_inv_failed_handler =
            &default_handler<entry_inv_key>;
    
    //boost::mutex exit_inv_failed_mutex;
    assertion_failed_handler exit_inv_failed_handler =
            &default_handler<exit_inv_key>;
}

#define BOOST_CONTRACT_EXCEPTION_SET_HANDLER_(_mutex, handler, f) \
    /*boost::mutex::scoped_lock lock(exception_::_mutex);*/ \
    assertion_failed_handler result = exception_::handler; \
    exception_::handler = f; \
    return result;
    
#define BOOST_CONTRACT_EXCEPTION_GET_HANDLER_(_mutex, handler) \
    /*boost::mutex::scoped_lock lock(exception_::_mutex)*/; \
    return exception_::handler;
    
#define BOOST_CONTRACT_EXCEPTION_CALL_HANDLER_(_mutex, handler, where) \
    /*boost::mutex::scoped_lock lock(exception_::_mutex);*/ \
    exception_::handler(where);

assertion_failed_handler set_precondition_failed(
        assertion_failed_handler const& f) BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_SET_HANDLER_(pre_failed_mutex, pre_failed_handler,
            f)
}

assertion_failed_handler get_precondition_failed() BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_GET_HANDLER_(pre_failed_mutex, pre_failed_handler)
}

void precondition_failed(from where) /* can throw */ {
    BOOST_CONTRACT_EXCEPTION_CALL_HANDLER_(pre_failed_mutex, pre_failed_handler,
            where)
}

assertion_failed_handler set_postcondition_failed(
        assertion_failed_handler const& f) BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_SET_HANDLER_(post_failed_mutex,
            post_failed_handler, f)
}

assertion_failed_handler get_postcondition_failed() BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_GET_HANDLER_(post_failed_mutex,
            post_failed_handler)
}

void postcondition_failed(from where) /* can throw */ {
    BOOST_CONTRACT_EXCEPTION_CALL_HANDLER_(post_failed_mutex,
            post_failed_handler, where)
}

assertion_failed_handler set_entry_invariant_failed(
        assertion_failed_handler const& f) BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_SET_HANDLER_(entry_inv_failed_mutex,
            entry_inv_failed_handler, f)
}

assertion_failed_handler get_entry_invariant_failed()
        BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_GET_HANDLER_(entry_inv_failed_mutex,
            entry_inv_failed_handler)
}

void entry_invariant_failed(from where) /* can throw */ {
    BOOST_CONTRACT_EXCEPTION_CALL_HANDLER_(entry_inv_failed_mutex,
            entry_inv_failed_handler, where)
}

assertion_failed_handler set_exit_invariant_failed(
        assertion_failed_handler const& f) BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_SET_HANDLER_(exit_inv_failed_mutex,
            exit_inv_failed_handler, f)
}

assertion_failed_handler get_exit_invariant_failed() BOOST_NOEXCEPT_OR_NOTHROW {
    BOOST_CONTRACT_EXCEPTION_GET_HANDLER_(exit_inv_failed_mutex,
            exit_inv_failed_handler)
}

void exit_invariant_failed(from where) /* can throw */ {
    BOOST_CONTRACT_EXCEPTION_CALL_HANDLER_(exit_inv_failed_mutex,
            exit_inv_failed_handler, where)
}

void set_invariant_failed(
        assertion_failed_handler const& f) BOOST_NOEXCEPT_OR_NOTHROW {
    set_entry_invariant_failed(f);
    set_exit_invariant_failed(f);
}

} } // namespace

#endif // #include guard

