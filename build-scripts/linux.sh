#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"

build_dir="${repo_root}/build"
unittest_build_dir="${repo_root}/build/unittest"

mkdir -p "${build_dir}"
cd "${build_dir}"
cmake -G Ninja "${repo_root}"
ninja

mkdir -p "${unittest_build_dir}"
cd "${unittest_build_dir}"
cmake -G Ninja "${repo_root}/test/unittest"
ninja
./unabto_unittest
