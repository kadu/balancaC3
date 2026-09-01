"""Generates include/build_info.h before every build.

The header is rewritten on each run so the timestamp always reflects the build
that actually produced the binary — that is the whole point of showing it in the
web UI. Only the files that include it get recompiled, so the cost is one extra
translation unit plus the link.

The file is generated, not committed (see .gitignore).
"""

import datetime
import os
import subprocess

Import("env")  # noqa: F821 — injected by PlatformIO/SCons


def sh(cmd, default=""):
    try:
        out = subprocess.check_output(cmd, shell=True, stderr=subprocess.DEVNULL)
        return out.decode("utf-8", "replace").strip()
    except Exception:
        return default


git_hash = sh("git rev-parse --short HEAD", "sem-git")
# Mark builds made on top of uncommitted changes — otherwise the hash would
# claim a state the binary does not actually correspond to.
if git_hash != "sem-git" and sh("git status --porcelain"):
    git_hash += "+"

stamp = datetime.datetime.now().strftime("%d/%m/%Y %H:%M")

header = (
    "// GERADO por scripts/build_info.py a cada build — nao editar, nao commitar.\n"
    "#pragma once\n"
    '#define BUILD_STAMP    "%s"\n'
    '#define BUILD_GIT_HASH "%s"\n'
) % (stamp, git_hash)

path = os.path.join(env["PROJECT_INCLUDE_DIR"], "build_info.h")  # noqa: F821
with open(path, "w", encoding="utf-8") as f:
    f.write(header)

print("build_info.h: %s | %s" % (stamp, git_hash))
