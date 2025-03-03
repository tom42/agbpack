// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

export module agbpack:version;

// TODO: looks like this is going to be painful, so we might just as well go for a configuration header. Ouch, oh well.
inline constexpr const char* version = AGBPACK_VERSION;
