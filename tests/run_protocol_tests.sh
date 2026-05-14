#!/usr/bin/env bash
set -euo pipefail

CXX=${CXX:-g++}
OUT=${OUT:-/tmp/smart_desk_protocol_tests}

"$CXX" -std=c++17 -Wall -Wextra -Werror \
  -Itests/stubs \
  -I. \
  tests/protocol_decoder_test.cpp \
  components/smart_desk/rx_decoder.cpp \
  components/smart_desk/tx_verifier.cpp \
  components/smart_desk/constant.cpp \
  -o "$OUT"

"$OUT"
