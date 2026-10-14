# HyperFrames C++ v1.0.0 — release binaries

Versioned distribution files only; engine source remains on the `main` branch. There are no video projects or audiovisual assets in this branch.

- `hyperframes-cpp-v1.0.0-linux-x64.tar.gz` — Linux x64 executable, documentation, notices and Agent Skill.
- `hyperframes-cpp-v1.0.0-windows-x64.zip` — Windows x64 executable, documentation, notices and Agent Skill. Cross-compiled; requires real Windows runtime testing.
- `hyperframes-cpp-v1.0.0-agent-skill.zip` — portable SKILL.md and companion reference.
- `SHA256SUMS.txt` — hashes of the three packages.

Verify the files with `sha256sum -c SHA256SUMS.txt` or `Get-FileHash` on Windows. Browser mode requires installed Chrome/Chromium/Edge; native output does not use FFmpeg.

The GitHub integration presently supports Git pushes and release creation, but not uploading binary release attachments. Therefore the v1.0.0 release page links to the permanent downloadable files here. These are **Git-branch-hosted distributions, not assets attached to the release**.
