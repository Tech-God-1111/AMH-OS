#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/third_party"
cd "$ROOT/third_party"
if [[ ! -d nuttx/.git ]]; then
  git clone --depth 1 https://github.com/apache/nuttx.git nuttx
else
  echo "NuttX checkout already exists; leaving it unchanged."
fi
if [[ ! -d apps/.git ]]; then
  git clone --depth 1 https://github.com/apache/nuttx-apps.git apps
else
  echo "NuttX apps checkout already exists; leaving it unchanged."
fi
echo "Upstream sources ready. Next: ./scripts/configure.sh"
