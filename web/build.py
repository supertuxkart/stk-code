#!/usr/bin/env python3
"""Build the actual C++ game for browsers; requires an activated Emscripten SDK.

All downloads and generated files stay in ignored build-web* directories.
Nothing is installed into macOS or /usr/local.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "build-web-tools"
PREFIX = TOOLS / "prefix"
FLAGS = ["-pthread", "-fwasm-exceptions"]
PORTS = ["-sUSE_SDL=2", "-sUSE_ZLIB=1", "-sUSE_LIBPNG=1", "-sUSE_LIBJPEG=1",
         "-sUSE_FREETYPE=1", "-sUSE_HARFBUZZ=1", "-sUSE_OGG=1", "-sUSE_VORBIS=1"]


def run(args, cwd=ROOT, env=None):
    print("+", " ".join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=cwd, env=env, check=True)


def checkout(name, url, tag):
    path = TOOLS / name
    if not path.exists():
        run(["git", "clone", "--depth", "1", "--branch", tag, url, path])
    return path


def dependencies(jobs):
    # Ask Emscripten to build matching threaded ports and install their headers
    # before CMake's FindPNG/FindJPEG checks inspect the SDK sysroot.
    TOOLS.mkdir(exist_ok=True)
    probe = TOOLS / "ports.c"
    probe.write_text("int main(void) { return 0; }\n")
    run(["emcc", probe, *FLAGS, *PORTS, "-o", TOOLS / "ports.js"])

    if not (PREFIX / "lib/libcrypto.a").exists():
        crypto = checkout("openssl", "https://github.com/openssl/openssl.git", "openssl-3.3.3")
        env = os.environ.copy()
        env.update(CC="emcc", AR="emar", RANLIB="emranlib",
                   CFLAGS="-O2 -pthread -fwasm-exceptions")
        # Only the game's crypto API needs this library. Browser HTTPS is not
        # implemented by OpenSSL, and native online features remain disabled.
        run(["perl", "Configure", "linux-generic32", "no-shared", "no-asm",
             "no-dso", "no-engine", "no-tests", "no-async", "no-afalgeng"], crypto, env)
        run(["make", f"-j{jobs}", "build_generated", "libcrypto.a"], crypto, env)
        (PREFIX / "lib").mkdir(parents=True, exist_ok=True)
        shutil.copytree(crypto / "include/openssl", PREFIX / "include/openssl", dirs_exist_ok=True)
        shutil.copy2(crypto / "libcrypto.a", PREFIX / "lib")

    if not (PREFIX / "lib/libcurl.a").exists():
        curl = checkout("curl", "https://github.com/curl/curl.git", "curl-8_12_1")
        build = TOOLS / "curl-build"
        run(["emcmake", "cmake", "-S", curl, "-B", build, "-G", "Ninja",
             f"-DCMAKE_INSTALL_PREFIX={PREFIX}", "-DCMAKE_INSTALL_LIBDIR=lib",
             "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_C_FLAGS=-pthread -fwasm-exceptions",
             "-DBUILD_SHARED_LIBS=OFF", "-DBUILD_CURL_EXE=OFF", "-DBUILD_TESTING=OFF",
             "-DHTTP_ONLY=ON", "-DCURL_USE_OPENSSL=OFF", "-DCURL_USE_LIBPSL=OFF",
             "-DCURL_USE_LIBSSH2=OFF", "-DCURL_USE_GSSAPI=OFF", "-DCURL_ZLIB=OFF",
             "-DENABLE_THREADED_RESOLVER=OFF", "-DENABLE_IPV6=OFF"])
        run(["cmake", "--build", build, "--parallel", jobs])
        run(["cmake", "--install", build])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=min(os.cpu_count() or 2, 6))
    parser.add_argument("--assets", type=Path, default=TOOLS / "data")
    parser.add_argument("--deps-only", action="store_true")
    parser.add_argument("--sdk", type=Path, default=TOOLS / "emsdk",
                        help="SDK directory; avoids needing to source emsdk_env.sh")
    args = parser.parse_args()
    if (args.sdk / "upstream/emscripten/emcc").is_file():
        os.environ["PATH"] = str(args.sdk.resolve() / "upstream/emscripten") + os.pathsep + os.environ["PATH"]
        os.environ["EMSDK_PYTHON"] = sys.executable
    for tool in ("emcc", "emcmake", "em-config", "cmake", "ninja", "git", "perl", "make"):
        if not shutil.which(tool):
            parser.error(f"Missing {tool}. Activate Emscripten and install the documented build tools.")
    dependencies(args.jobs)
    if args.deps_only:
        return
    if not (args.assets / "supertuxkart.git").exists():
        parser.error("Run python3 web/prepare_assets.py PATH_TO_ASSETS first.")
    build = ROOT / "build-web"
    run(["emcmake", "cmake", "-S", ROOT, "-B", build, "-G", "Ninja",
         "-DCMAKE_BUILD_TYPE=Release", f"-DWEB_PREFIX={PREFIX}",
         f"-DWEB_ASSETS={args.assets.resolve()}"])
    run(["cmake", "--build", build, "--parallel", args.jobs])
    print(f"Serve with: python3 web/serve.py --directory '{build / 'web'}'")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        sys.exit(error.returncode)
