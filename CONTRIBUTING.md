# Contributing

Thank you for considering a contribution to HyperFrames C++.

This project is a **small experimental renderer**, not a complete HTML browser. Please discuss major compatibility or architectural changes in an issue before investing significant work.

## Development workflow

1. Create a branch from `main` and keep changes focused.
2. Build on Linux using `./scripts/build-linux.sh`. The project targets C++17, without required dynamic runtime dependencies.
3. Run `./tests/smoke-linux.sh` and add a regression test for changed rendering, HTML parsing or video output.
4. For cross-platform changes, also build with `./scripts/build-windows.sh` where possible.
5. Open a pull request that explains the feature, motivation, testing and any visible output changes.

## Code guidelines

- Favor portable C++17 and narrowly scoped modules over heavy frameworks.
- Do not invoke FFmpeg or other external encoding tools from the runtime executable. The optional, explicitly selected browser renderer is an intentional exception for HTML/CSS/JavaScript fidelity.
- Keep runtime inputs validated. The browser mode executes JavaScript; accept only trusted local HTML and use least privilege.
- Do not commit video projects, examples using third-party imagery, audio/narration, generated AVI/MP4, binary executables, raw fonts or GSAP distributions. Synthetic technical fixtures in tests/ are allowed.
- Preserve upstream attribution for vendored libraries and fonts.
- Document compatibility limits accurately; do not advertise full HTML, CSS or GSAP support.

## Reports and security

For bug reports, include the command, OS, compiler version, relevant minimal HTML and expected/actual result. **Do not include secrets, personal data or copyrighted input media without permission.**

For suspected security vulnerabilities, use the security reporting instructions in [SECURITY.md](SECURITY.md), rather than publishing an exploit in an issue.

By participating, you agree to follow [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
