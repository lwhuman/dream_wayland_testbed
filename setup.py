#!/usr/bin/env python3
import os
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT_DIR = Path(__file__).parent.resolve()
BUILD_DIR = PROJECT_ROOT_DIR / "build"
VIRT_ENV_DIR = BUILD_DIR / ".venv"
GLAD_PROJECT_DIR = PROJECT_ROOT_DIR / "external" / "glad"

GLAD_ARGS = [
    "--reproducible",
    "--out-path=.",
    "--api=gl:core=4.5",
    "c",
    "--header-only",
]


def log(msg):
    print(f"[setup.py] {msg}")


def run(cmd, cwd=None, env=None):
    log(f"Running: {' '.join(cmd)}")
    subprocess.check_call(cmd, cwd=cwd, env=env)


def pip_has_package(python, package):
    try:
        subprocess.check_call(
            [python, "-m", "pip", "show", package],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return True
    except subprocess.CalledProcessError:
        return False


def main():
    if not BUILD_DIR.exists():
        BUILD_DIR.mkdir(parents=True)
        log(f"Build dir created at: {BUILD_DIR}")

    if not VIRT_ENV_DIR.exists():
        log(f"Creating virtual environment at: {VIRT_ENV_DIR}")
        run([sys.executable, "-m", "venv", str(VIRT_ENV_DIR)])

    if os.name == "nt":
        V_PYTHON = VIRT_ENV_DIR / "Scripts" / "python.exe"
    else:
        V_PYTHON = VIRT_ENV_DIR / "bin" / "python"

    if pip_has_package(V_PYTHON, "glad2"):
        log("glad is already installed. skipping...")
    else:
        log(f"Installing glad from {GLAD_PROJECT_DIR} as editable")
        run(
            [str(V_PYTHON), "-m", "pip", "install", "-e", str(GLAD_PROJECT_DIR)]
        )

    log("Generating OpenGL loader using GLAD...")
    run([str(V_PYTHON), "-m", "glad", *GLAD_ARGS])

    log("Running cmake to generate compile_commands.json")
    run(["cmake", ".."], cwd=BUILD_DIR)

    log("Setup complete.")


if __name__ == "__main__":
    main()
