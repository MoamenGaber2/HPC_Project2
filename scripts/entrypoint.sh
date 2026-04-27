#!/usr/bin/env bash
set -euo pipefail

mkdir -p /run/sshd
/usr/sbin/sshd

if [ "$#" -gt 0 ]; then
  exec "$@"
fi

exec sleep infinity
