#!/bin/sh
set -eu
command -v gitleaks >/dev/null 2>&1 || { echo 'Install gitleaks first.' >&2; exit 1; }
git rev-parse --git-dir >/dev/null
git config --local core.hooksPath .githooks
echo 'Local secret scanning hooks enabled for this clone.'
