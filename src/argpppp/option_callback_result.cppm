// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <string>

export module argpppp:option_callback_result;
import :optional_string;

namespace argpppp
{

// TODO: wondering whether we're overengineering things:
//       * We're trying to overload (kind of) return value.
//       * Otoh: we have the implicit conversion problem anyway, no?
//       * So maybe live with it?
//       * We could make option_callback_result completely opaque and hide it, no? It would not even be exported?
//       * And we could ssupport implicit conversion from bool
//       * And anything else is reported using factory functions like error below? (which we should probably rename to argument_error?)
export class option_callback_result final
{
public:
    // Allow implicit conversion from bool.
    constexpr option_callback_result(bool success) : m_success(success) {}

    explicit option_callback_result(const std::string& error_message) : m_error_message(error_message) {}

    bool is_successful() const { return m_success; }

    const optional_string& error_message() const { return m_error_message; }

private:
    bool m_success = false;
    optional_string m_error_message;
};

// TODO: unit test?
export inline option_callback_result error(const std::string& error_message)
{
    return option_callback_result(error_message);
}

}
