#!/usr/bin/env bash
# Builds the application (and its .mo catalogs) and the i18n test out of tree,
# then runs the test with LANGUAGE=pt_BR, LANGUAGE=en and the C locale.
#   tests/run-i18n-tests.sh [build-dir]     (default: a new temporary directory)
set -euo pipefail

src=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
build=${1:-$(mktemp -d)}
qmake=${QMAKE:-$(command -v qmake6 || command -v qmake)}
mkdir -p "$build/app" "$build/test"

(cd "$build/app" && "$qmake" "$src/PainelControle.pro" >/dev/null && make -s -j"$(nproc)")
(cd "$build/test" && "$qmake" "$src/tests/tst_i18n.pro" >/dev/null && make -s -j"$(nproc)")

run() {  # expected-language, then environment assignments
  local expected=$1; shift
  echo "== $* (expect $expected)"
  env -u LC_ALL -u LC_MESSAGES QT_QPA_PLATFORM=offscreen \
    PAINEL_CONTROLE_LOCALEDIR="$build/app/locale" PAINEL_CONTROLE_EXPECTED_LANGUAGE="$expected" \
    "$@" "$build/test/tst_i18n"
}

run pt_BR LANG=en_US.UTF-8 LANGUAGE=pt_BR
run en    LANG=en_US.UTF-8 LANGUAGE=en
run en    LANG=C LC_ALL=C LANGUAGE=
