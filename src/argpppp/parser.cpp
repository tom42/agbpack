// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <exception>
#include <iterator>
#include <ranges>
#include <vector>
#include <stdexcept>

module argpppp;
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

auto find_option_or_throw(const std::vector<option>& options, int key)
{
    auto opt = std::ranges::find_if(options, [=](const option& o) { return o.key() == key; });
    if (opt == options.end())
    {
        throw std::logic_error("find_option_or_throw: option not found");
    }
    return opt;
}

}

void parser::add_option(const option& o, const option_callback& c)
{
    m_options.push_back(o);

    if (o.key() == 0)
    {
        if (c)
        {
            throw std::invalid_argument("add_option: special options with key = 0 must not have callbacks");
        }
    }
    else
    {
        if (!c)
        {
            throw std::invalid_argument("add_option: option must have a callback");
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
        return handle_option_callback_result(callback_result, key, arg, state);
    }

    // TODO: apparently, state->name is the program name (read that up again! so, if we wanted we could probably set it here, and it'd always be correct, no?
    //       ugh...not sure...would it also be correct for builtin options such as --help or --version?
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

// TODO: is this testworthy? => Sure is
error_t parser::handle_option_callback_result(bool result, int key, char* arg, const argp_state * state)
{
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
        // TODO: format a default error message. For this we need:
        //       * The option
        //       * The argument
        //       * Unfortunately we have neither
        //       * Well we could make key and arg arguments of this function, and make this function a member function
        //         * Why a member function? Because we have then access to the options array where we can find the option by key
        // TODO: probably want to be able to replace this (this probably being argp_failure) with some sort of test double for unit testing

        // TODO: OK, part 2: using the option and the argument, format a default message and pass that to argp_failure
        //       Ugh: what if arg is missing because it is not required or optional?

        auto opt = find_option_or_throw(m_options, key);

        // TODO: number of problems here:
        //       * arg may be null
        //         * It may be optional and was not given (but is it then null or empty? Need to check with a real argp_parser I guess)
        //           * We may have a switch which never gets an argument. In this case we might want want to use a different message
        // TODO: we really ought to have some hook to capture argp_failure output
        // TODO: re %s: we protect us from error messages returning strings with percent signs:
        //              * Document?
        //              * It certainly is testworthy, no?
        argp_failure(state, EXIT_FAILURE, 0, "%s", get_default_error_message(*opt, arg).c_str());
        return EINVAL;
    }
}

}
