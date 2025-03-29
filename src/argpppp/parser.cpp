// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <cstddef>
#include <exception>
#include <iterator>
#include <ranges>
#include <variant>
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
    argpppp_context(const argpppp_context&) = delete;
    argpppp_context& operator=(const argpppp_context&) = delete;

    argpppp_context(const parser& p, parse_result& r)
        : this_parser(p)
        , result(r)
    {}

    const parser& this_parser;
    parse_result& result;
    std::exception_ptr exception;
};

argpppp_context* get_context(argp_state* state)
{
    return static_cast<argpppp_context*>(state->input);
}

void rethrow_exception_if_any(const argpppp_context& context)
{
    if (context.exception)
    {
        std::rethrow_exception(context.exception);
    }
}

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

void parser::set_args_doc(const optional_string& s)
{
    m_args_doc = s;
}

void parser::set_doc(const optional_string& s)
{
    m_doc = s;
}

void parser::set_failure_callback(const failure_callback& c)
{
    m_failure_callback = c;
}

void parser::set_nargs(std::size_t nargs)
{
    set_nargs(nargs, nargs);
}

void parser::set_nargs(std::size_t min_args, std::size_t max_args)
{
    if (min_args > max_args)
    {
        throw std::logic_error("min_args must be <= max_args");
    }

    m_min_args = min_args;
    m_max_args = max_args;
}

parse_result parser::parse(int argc, char** argv, pf flags) const
{
    constexpr const argp_child* children = nullptr;
    constexpr const auto help_filter = nullptr;
    constexpr const char* argp_domain = nullptr;
    const auto argp_options = to_argp_options(m_options);
    const argp argp { argp_options.data(), parse_option_static, c_str(m_args_doc), c_str(m_doc), children, help_filter, argp_domain };

    parse_result result;
    argpppp_context context(*this, result);
    result.errnum = argp_parse(&argp, argc, argv, to_uint(flags), nullptr, &context);

    rethrow_exception_if_any(context);
    return result;
}

error_t parser::parse_option(int key, char* arg, argp_state* state) const
{
    auto callback = m_callbacks.find(key);
    if (callback != m_callbacks.end())
    {
        const auto callback_result = callback->second(arg);
        return handle_option_callback_result(callback_result, key, arg, state);
    }

    switch (key)
    {
        case ARGP_KEY_ARG:
            return handle_key_arg(arg, state);
        case ARGP_KEY_END:
            return handle_key_end(state);
        default:
            return ARGP_ERR_UNKNOWN;
    }
}

error_t parser::parse_option_static(int key, char* arg, argp_state* state)
{
    auto context = get_context(state);
    try
    {
        return context->this_parser.parse_option(key, arg, state);
    }
    catch (...)
    {
        // Do not let exception escape into argp, which is written in C.
        // Instead, pass exception to calling C++ code through argpppp_context instance.
        context->exception = std::current_exception();
        return EINVAL;
    }
}

error_t parser::handle_option_callback_result(const option_callback_result& result, int key, char* arg, const argp_state* state) const
{
    return std::visit([&](const auto& r) { return handle_option_callback_result_for_type(r, key, arg, state); }, result);
}

error_t parser::handle_option_callback_result_for_type(bool result, int key, char* arg, const argp_state* state) const
{
    if (result)
    {
        return 0;
    }
    else
    {
        auto opt = find_option_or_throw(m_options, key);
        auto error_message = get_default_error_message(*opt, arg);
        report_failure(state, EXIT_FAILURE, 0, error_message);
        return EINVAL;
    }
}

error_t parser::handle_option_callback_result_for_type(const arg_error& error, int, char*, const argp_state* state) const
{
    report_failure(state, EXIT_FAILURE, 0, error.message());
    return EINVAL;
}

error_t parser::handle_key_arg(char* arg, argp_state* state) const
{
    get_context(state)->result.args.push_back(arg);
    return 0;
}

error_t parser::handle_key_end(argp_state* state) const
{
    auto context = get_context(state);

    if (context->result.args.size() < m_min_args)
    {
        report_failure(state, EXIT_FAILURE, 0, "too few arguments");
        return EINVAL;
    }

    if (context->result.args.size() > m_max_args)
    {
        report_failure(state, EXIT_FAILURE, 0, "too many arguments");
        return EINVAL;
    }

    return 0;
}

void parser::report_failure(const argp_state* state, int status, int errnum, const std::string& message) const
{
    if (m_failure_callback)
    {
        m_failure_callback(errnum, message);
    }
    else
    {
        argp_failure(state, status, errnum, "%s", message.c_str());
    }
}

}
