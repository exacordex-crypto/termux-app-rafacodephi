#!/usr/bin/env python3
"""
Validate canonical side-by-side contract for Termux RAFCODEΦ.

This is a fast static gate that catches identity drift across build/runtime/manifest
without requiring Android SDK installation.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CANONICAL_PACKAGE = "com.termux.rafacodephi"
LEGACY_CODE_PACKAGE = "com.termux"
REQUIRED_PAGE_SIZE = "16384"
PACKAGE_RE = re.compile(r"^[a-z][a-z0-9_]*(?:\.[a-z][a-z0-9_]*)+$")


def read_text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(pattern: str, text: str, message: str, *, multiline: bool = True) -> str | None:
    flags = re.MULTILINE if multiline else 0
    if not re.search(pattern, text, flags):
        return message
    return None


def gradle_default(name: str, text: str) -> str | None:
    pattern = re.compile(
        rf'{name}\s*=\s*System\.getenv\("[^"]+"\)\s*\?:\s*"([^"]+)"'
    )
    match = pattern.search(text)
    return match.group(1) if match else None


def require_gradle_default(name: str, expected: str, text: str, errors: list[str]) -> None:
    actual = gradle_default(name, text)
    if actual != expected:
        errors.append(f"app/build.gradle: {name} default must be {expected}, got {actual!r}")


def require_valid_package(label: str, package_name: str | None, errors: list[str]) -> None:
    if not package_name or not PACKAGE_RE.fullmatch(package_name):
        errors.append(f"app/build.gradle: {label} must be a concrete Android package name, got {package_name!r}")


def main() -> int:
    errors: list[str] = []

    build_gradle = read_text("app/build.gradle")
    require_gradle_default("appPackageName", CANONICAL_PACKAGE, build_gradle, errors)
    require_gradle_default("bootstrapMetadataPackageName", CANONICAL_PACKAGE, build_gradle, errors)
    require_gradle_default("bootstrapRequiredPageSize", REQUIRED_PAGE_SIZE, build_gradle, errors)

    app_package = gradle_default("appPackageName", build_gradle)
    bootstrap_package = gradle_default("bootstrapMetadataPackageName", build_gradle)
    require_valid_package("appPackageName", app_package, errors)
    require_valid_package("bootstrapMetadataPackageName", bootstrap_package, errors)
    if app_package and bootstrap_package and app_package != bootstrap_package:
        errors.append(
            "app/build.gradle: appPackageName and bootstrapMetadataPackageName must match "
            "for side-by-side bootstrap paths"
        )

    errors += filter(
        None,
        [
            require(
                r'manifestPlaceholders\.TERMUX_PACKAGE_NAME\s*=\s*project\.ext\.appPackageName',
                build_gradle,
                "app/build.gradle: manifest TERMUX_PACKAGE_NAME must be derived from appPackageName",
            ),
            require(
                r'--package-name",\s*project\.ext\.bootstrapMetadataPackageName',
                build_gradle,
                "app/build.gradle: bootstrap rewrite package-name must use bootstrapMetadataPackageName",
            ),
            require(
                r'--prefix",\s*"/data/data/\$\{project\.ext\.bootstrapMetadataPackageName\}/files/usr"',
                build_gradle,
                "app/build.gradle: bootstrap prefix must be package-derived, not hardcoded com.termux",
            ),
            require(
                r'--home",\s*"/data/data/\$\{project\.ext\.bootstrapMetadataPackageName\}/files/home"',
                build_gradle,
                "app/build.gradle: bootstrap home must be package-derived, not hardcoded com.termux",
            ),
            require(
                r'--page-size",\s*project\.ext\.bootstrapRequiredPageSize',
                build_gradle,
                "app/build.gradle: bootstrap rewrite page size must use bootstrapRequiredPageSize",
            ),
            require(
                r'hasReleaseTaskRequested\(\)\s*&&\s*bootstrapBaremetalStrictOverride\s*==\s*false',
                build_gradle,
                "app/build.gradle: release tasks must reject disabled baremetal bootstrap strict mode",
            ),
            require(
                r'BOOTSTRAP_INFO TERMUX_PACKAGE_NAME mismatch',
                build_gradle,
                "app/build.gradle: bootstrap metadata must fail on package mismatch",
            ),
            require(
                r'BOOTSTRAP_INFO TERMUX_PAGE_SIZE mismatch',
                build_gradle,
                "app/build.gradle: bootstrap metadata must fail on page-size mismatch",
            ),
        ],
    )

    constants_java = read_text("termux-shared/src/main/java/com/termux/shared/termux/TermuxConstants.java")
    errors += filter(
        None,
        [
            require(
                rf'TERMUX_PACKAGE_NAME\s*=\s*"{re.escape(CANONICAL_PACKAGE)}"',
                constants_java,
                "TermuxConstants.java: TERMUX_PACKAGE_NAME is not canonical com.termux.rafacodephi",
            ),
            require(
                rf'TERMUX_APP_CODE_PACKAGE_NAME\s*=\s*"{re.escape(LEGACY_CODE_PACKAGE)}"',
                constants_java,
                "TermuxConstants.java: TERMUX_APP_CODE_PACKAGE_NAME missing or changed unexpectedly",
            ),
            require(
                r'TERMUX_INTERNAL_PRIVATE_APP_DATA_DIR_PATH\s*=\s*"/data/data/" \+ TERMUX_PACKAGE_NAME[\s\S]*TERMUX_FILES_DIR_PATH\s*=\s*TERMUX_INTERNAL_PRIVATE_APP_DATA_DIR_PATH \+ "/files"[\s\S]*TERMUX_PREFIX_DIR_PATH\s*=\s*TERMUX_FILES_DIR_PATH \+ "/usr"',
                constants_java,
                "TermuxConstants.java: runtime prefix path must be derived from TERMUX_PACKAGE_NAME",
            ),
        ],
    )

    shortcuts = read_text("app/src/main/res/xml/shortcuts.xml")
    errors += filter(
        None,
        [
            require(
                rf'android:targetPackage="{re.escape(CANONICAL_PACKAGE)}"',
                shortcuts,
                "shortcuts.xml: targetPackage must point to canonical app id",
            ),
            require(
                rf'<extra android:name="{re.escape(CANONICAL_PACKAGE)}\.app\.failsafe_session"',
                shortcuts,
                "shortcuts.xml: failsafe extra key must use canonical package prefix",
            ),
        ],
    )

    manifest = read_text("app/src/main/AndroidManifest.xml")
    if 'android:name=".app.' in manifest or 'android:name=".shared.' in manifest or 'android:name=".filepicker.' in manifest:
        errors.append("AndroidManifest.xml: relative component names (.app/.shared/.filepicker) are forbidden in canonical mode")

    if errors:
        print("❌ Side-by-side contract validation failed:")
        for err in errors:
            print(f"  - {err}")
        return 1

    print("✅ Side-by-side contract validation passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
