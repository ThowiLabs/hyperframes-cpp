# Security Policy

## Supported versions

HyperFrames C++ is an experimental prototype. There are **no supported stable releases or security-update guarantees** at present.

## Reporting a vulnerability

Please **do not** post detailed proof-of-concept exploits or malicious HTML in a public issue. Use GitHub's private vulnerability reporting feature if enabled for this repository, or contact the maintainers through an appropriate private channel.

Helpful details include affected source revision, minimum reproduction input, impact, platform and any suggested mitigation.

## Current security boundaries

The parser intentionally handles a limited subset of HTML without executing embedded JavaScript. Input limits exist, but arbitrary untrusted HTML and WAV files have **not** undergone a formal security audit. Do not use this prototype to process untrusted media in a privileged environment.
