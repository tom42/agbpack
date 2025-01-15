<!--
SPDX-FileCopyrightText: 2025 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# Important notice

To avoid ODR violations caused by specializations of `Catch::StringMaker`,
specializations of that class should only be provided by `agbpack_unit_testkit`.

Moreover, all translation units of `agbpack_unit_test` should import
`agbpack_unit_testkit`.

See https://brevzin.github.io/c++/2023/01/19/debug-fmt-catch/
