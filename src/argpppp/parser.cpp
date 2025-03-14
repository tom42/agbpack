// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <exception>
#include <iterator>
#include <stdexcept>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?

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

// TODO: is this testworthy?
error_t handle_option_callback_result(bool result)
{
    // TODO: print error message using argp_error (or argp_failure) if return value is false.
    return result ? 0 : EINVAL;
}

}

void parser::add_option(const option& o, const option_callback& c)
{
    if (o.key() == 0)
    {
        if (c)
        {
            throw std::logic_error("add_option: special options with key = 0 must not have callbacks");
        }
    }
    else
    {
        if (!c)
        {
            throw std::logic_error("add_option: option must have a callback");
        }
        m_options.push_back(o);
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

void parser::parse(int argc, char** argv)
{
    constexpr const argp_child* children = nullptr;
    constexpr const auto help_filter = nullptr;
    constexpr const char* argp_domain = nullptr;
    const auto argp_options = to_argp_options(m_options);
    const argp argp { argp_options.data(), parse_option_static, c_str(m_args_doc), c_str(m_doc), children, help_filter, argp_domain };

    constexpr auto flags = 0;
    argpppp_context context(this);
    argp_parse(&argp, argc, argv, flags, nullptr, &context);

    context.rethrow_exception_if_any();
}

error_t parser::parse_option(int key, char* arg, argp_state* /*state*/)
{
    auto callback = m_callbacks.find(key);
    if (callback != m_callbacks.end())
    {
        return handle_option_callback_result(callback->second(arg));
    }

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
