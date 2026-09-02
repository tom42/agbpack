// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <format>
#include <functional> // Required by g++ 15.2
#include <ranges>

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

using argpppp::command_line_parser;
using argpppp::error;
using argpppp::of;
using argpppp::ok;
using argpppp::options;
using argpppp::pf;
using argpppp::value;
using std::format;
using std::string;
using std::string_view;

namespace
{

command_line_parser make_parser(bool is_unit_test)
{
    command_line_parser parser;

    if (is_unit_test)
    {
        parser.flags(pf::no_errs | pf::no_exit);
    }

    return parser;
}

string list_compression_methods()
{
    using namespace std::ranges;
    using namespace std::views;

    return all_compression_methods()
        | transform([](auto& method) { return string_view(method.name); })
        | join_with(string_view(", "))
        | to<string>();
}

const char* to_string(compression_method method)
{
    return find_compression_method(method)->name;
}

}

parse_command_line_result parse_command_line(int argc, char* argv[], bool is_unit_test)
{
    parse_command_line_result result;

    auto parse_compression_method = [&](auto&& opt)
    {
        result.mode = program_mode::compress;

        if (opt.c_arg())
        {
            auto method_info = find_compression_method(opt.c_arg());
            if (method_info)
            {
                result.method = method_info->method;
            }
            else
            {
                return error(opt, "unknown compression method");
            }
        }

        return ok();
    };

    options command_line_options;
    command_line_options
        .doc(format("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack\n\nIf neither of -c or -d is given, data is compressed using method '{}'.", to_string(result.method)))
        .args_doc("FILE")
        .num_args(1)
        .add({ 'c', "compress", format("Compress the input file using the specified compression method. Compression method defaults to '{}' if not given. Valid compression methods are: {}", to_string(result.method), list_compression_methods()), "METHOD", of::arg_optional }, parse_compression_method)
        .add({ 'd', "decompress", "Decompress the input file" }, [&] { result.mode = program_mode::decompress; return ok(); })
        .add({ 'o', "output-file", "Output file name. If not given, input file is overwritten", "FILE" }, value(result.output_file))
        .add({ {}, "vram-safe", "Use VRAM safe version of compression method if available" }, value(result.vram_safe));

    auto parser = make_parser(is_unit_test);
    auto parse_result = parser.parse(argc, argv, command_line_options);

    result.success = parse_result.errnum == 0;

    if (result.success)
    {
        result.input_file = parse_result.args.at(0);
        if (result.output_file.empty())
        {
            result.output_file = result.input_file;
        }
    }

    return result;
}

}
