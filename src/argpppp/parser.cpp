// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <exception>
#include <iterator>
#include <stdexcept>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?
import :pf;

namespace argpppp
{

namespace
{

class argpppp_context final
{
public:
    // Nothing really bad will happen if we allow copying, but we only need to
    // route a context pointer through argp_parse, so we forbid copying.
    argpppp_context(const argpppp_context&) = delete;
    argpppp_context& operator=(const argpppp_context&) = delete;

    argpppp_context(parser* p) : m_parser(p) {}

    parser* get_parser() { return m_parser; }

    void set_exception(std::exception_ptr e) { m_exception = e; }

    void rethrow_exception_if_any()
    {
        if (m_exception)
        {
            std::rethrow_exception(m_exception);
        }
    }

private:
    parser* m_parser;
    std::exception_ptr m_exception;
};

// TODO: is this testworthy? => Sure is
error_t handle_option_callback_result(bool result, const argp_state* state)
{
    // TODO: print error message using argp_error/argp_failure if return value is false.
    // TODO: how should the error message look like?
    //       * well it should include the option name, and the invalid value.
    //       * Problem: the option short name may be missing. So we have to print the long name in this case
    // TODO: if the callback supplied an error rather than a boolean, then the error should be printed instead
    if (result)
    {
        return 0;
    }
    else
    {
        // TODO: format a default error message
        // TODO: probably want to be able to replace this with some sort of test double for unit testing
        argp_failure(state, EXIT_FAILURE, 0, "meh");
        return EINVAL;
    }
}

}

void parser::add_option(const option& o, const option_callback& c)
{
    m_options.push_back(o);

    if (o.key() == 0)
    {
        if (c)
        {
            throw std::logic_error("add_option: special options with key = 0 must not have callbacks");
        }
    }
    else
    {
        // TODO: verify that if the option has a non-printable key it has a long name? (This IS a requirement, isn't it?)
        if (!c)
        {
            throw std::logic_error("add_option: option must have a callback");
        }
        m_callbacks[o.key()] = c;
    }
}

void parser::args_doc(const optional_string& s)
{
    m_args_doc = s;
}

void parser::doc(const optional_string& s)
{
    m_doc = s;
}

void parser::parse(int argc, char** argv, pf flags)
{
    constexpr const argp_child* children = nullptr;
    constexpr const auto help_filter = nullptr;
    constexpr const char* argp_domain = nullptr;
    const auto argp_options = to_argp_options(m_options);
    const argp argp { argp_options.data(), parse_option_static, c_str(m_args_doc), c_str(m_doc), children, help_filter, argp_domain };

    argpppp_context context(this);
    argp_parse(&argp, argc, argv, to_uint(flags), nullptr, &context);

    context.rethrow_exception_if_any();
}

error_t parser::parse_option(int key, char* arg, argp_state* state)
{
    auto callback = m_callbacks.find(key);
    if (callback != m_callbacks.end())
    {
        const auto callback_result = callback->second(arg);
        return handle_option_callback_result(callback_result, state);
    }

    // TODO: apparently, state->name is the program name (read that up again! so, if we wanted we could probably set it here, and it'd always be correct, no?
    // TODO: there is no callback. So here goes now the idiomatic argp_parse switch on key.
    return ARGP_ERR_UNKNOWN;
}

error_t parser::parse_option_static(int key, char* arg, argp_state* state)
{
    argpppp_context* context = static_cast<argpppp_context*>(state->input);
    try
    {
        return context->get_parser()->parse_option(key, arg, state);
    }
    catch (...)
    {
        // Do not let exception escape into argp, which is written in C.
        // Instead, pass exception to calling C++ code through argpppp_context instance.
        context->set_exception(std::current_exception());
        return EINVAL;
    }
}

}
