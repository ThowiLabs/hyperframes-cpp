#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f "$script_dir/../.claude/skills/hyperframes-cpp/SKILL.md" ]]; then
  source_dir="$script_dir/../.claude/skills/hyperframes-cpp"
elif [[ -f "$script_dir/skills/hyperframes-cpp/SKILL.md" ]]; then
  source_dir="$script_dir/skills/hyperframes-cpp"
else
  echo "Cannot locate a packaged HyperFrames skill beside this installer" >&2
  exit 1
fi
if [[ ! -f "$source_dir/SKILL.md" ]]; then
  echo "Cannot locate SKILL.md in $source_dir" >&2
  exit 1
fi
destinations=()
if [[ $# -eq 0 ]]; then set -- --claude --agents; fi
for flag in "$@"; do
  case "$flag" in
    --claude) destinations+=("$HOME/.claude/skills/hyperframes-cpp");;
    --agents) destinations+=("$HOME/.agents/skills/hyperframes-cpp");;
    *) echo "Usage: install-skill.sh [--claude] [--agents]" >&2; exit 2;;
  esac
done
for dst in "${destinations[@]}"; do
  mkdir -p "$dst"
  cp -R "$source_dir/." "$dst/"
  echo "Installed skill to $dst"
done
