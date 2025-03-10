// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <functional>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?

namespace argpppp
{

namespace
{

// TODO: here too: document that options must not go out of scope?
// TODO: do we want to export and unit test this?
// TODO: we could stick this into the option partition thing, no?
std::vector<argp_option> to_argp_options(const std::vector<option>& options)
{
    std::vector<argp_option> argp_options;
    argp_options.reserve(options.size() + 1);
    std::transform(options.begin(), options.end(), back_inserter(argp_options), to_argp_option);
    argp_options.push_back({});
    return argp_options;
}

// TODO: temporarily make parser noncopyable to ensure we did not mess up anythin
// TODO: consider using a pointer rather than a parser reference. Might be less confusing
class argpppp_context final
{
public:
    // Nothing really bad will happen if we allow copying, but we only need to
    // route a context pointer through argp_parse, so we forbid copying.
    argpppp_context(const argpppp_context&) = delete;
    argpppp_context& operator=(const argpppp_context&) = delete;

    argpppp_context(parser& p) : m_parser(p) {}

    parser& get_parser() { return m_parser; }

    void set_exception(std::exception_ptr e) { m_exception = e; }

    void rethrow_exception_if_any()
    {
        if (m_exception)
        {
            std::rethrow_exception(m_exception);
        }
    }

private:
    parser& m_parser;
    std::exception_ptr m_exception;
};

}

void parser::add_option(const option& o, const std::function<void()>& c)
{
    m_options.push_back(o);
    m_callbacks[o.key()] = c;
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

    argpppp_context context(*this);
    argp_parse(&argp, argc, argv, 0, nullptr, &context);

    context.rethrow_exception_if_any();
}

error_t parser::parse_option(int key, char* /*arg*/, argp_state* /*state*/)
{
    // TODO: actually do something useful here:
    //       * find a registered handler for our key. If there is one, invoke it and handle its result
    //       * if there is none, handle key ourselves.
    auto callback = m_callbacks.find(key);
    if (callback != m_callbacks.end())
    {
        // TODO: should actually process return value of callback, but callbacks don't have a return value yet
        //       Speaking of callbacks: have an alias for these, will you please?
        callback->second();
    }

    return ARGP_ERR_UNKNOWN;
}

error_t parser::parse_option_static(int key, char* arg, argp_state* state)
{
    argpppp_context* context = static_cast<argpppp_context*>(state->input);
    try
    {
        return context->get_parser().parse_option(key, arg, state);
    }
    catch (...)
    {
        // Do not let exceptions escape into argp, which is written in C.
        // Instead, pass exception to calling C++ code through argpppp_context instance.
        context->set_exception(std::current_exception());
        return EINVAL;
    }
}

}
