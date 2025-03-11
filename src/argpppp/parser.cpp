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

error_t handle_option_callback_result(bool option_callback_result)
{
    // TODO: process return value of callback. Basically there are three possible outcomes
    //       * Callback returns 'success': return 0 for success
    //       * Callback returns 'failed' + nothing: print generic error message and return EINVAL. See ARGP_PARSE manual (https://www.gnu.org/software/libc/manual/html_node/Argp-Helper-Functions.html)
    //       * Callback returns 'failed' + error message: print callback's error message and return EINVAL. See ARGP_PARSE manual (https://www.gnu.org/software/libc/manual/html_node/Argp-Helper-Functions.html)
    return option_callback_result ? 0 : EINVAL;
}

}

void parser::add_option(const option& o, const option_callback& c)
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

    argpppp_context context(this);
    argp_parse(&argp, argc, argv, 0, nullptr, &context);

    context.rethrow_exception_if_any();
}

error_t parser::parse_option(int key, char* /*arg*/, argp_state* /*state*/)
{
    // TODO: mrmpf: we currently have the problem that it is possible to register options that have key=0 and a callback, or maybe they don't even need a callback
    //              either way, since key=0 is actually ARGP_KEY_ARG, if we do register such options, then we eventually attempt to invoke them. Bad, particularly if there is no callback
    //              so, what do we do?
    //              => In add_option, only add a callback to the callback map if
    //                 * key != 0
    //                 * So we have two cases:
    //                   * key == 0
    //                   * key != 0
    //                 * In both cases, callback can be given or empty => another two combinations
    //                   key    callback      action
    //                   ---------------------------------------------------------------------------
    //                   zero      empty      OK: do not add callback to map
    //                   zero      nonempty   ERROR: an option with key=0 should not have a callback, since it is never going to be called (alternatively, silently ignore this, but this feels ugly)
    //                   nonzero   empty      ERROR: an option with key!=0 should have a callback
    //                   nonzero   nonempty   OK: add callback
    //              => Need to think about this, but as it is, supplying an argument to afbpacker results in an empty std::function being called, which throws then std::bad_function_call
    auto callback = m_callbacks.find(key);
    if (callback != m_callbacks.end())
    {
        return handle_option_callback_result(callback->second());
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
