#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shlex
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


BASE_COMPILE_FLAGS = [
    "-std=gnu++20",
    "-O2",
    "-Wall",
    "-Wextra",
    "-Wshadow",
    "-DLOCAL",
]

SANITIZER_FLAGS = [
    "-fsanitize=undefined,address",
    "-fno-sanitize-recover=all",
    "-fno-omit-frame-pointer",
    "-g",
]

COMPILE_FLAGS = [*BASE_COMPILE_FLAGS, *SANITIZER_FLAGS]
_SANITIZER_AVAILABLE: bool | None = None
DEFAULT_TIMEOUT_SECONDS = 2.0
TIMEOUT_RETURN_CODE = 124


@dataclass(frozen=True)
class RunResult:
    stdout: bytes
    stderr: bytes
    returncode: int


def parse_count(value: str) -> int:
    try:
        count = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("count must be an integer") from exc
    if count < 0:
        raise argparse.ArgumentTypeError("count must be non-negative")
    return count


def parse_timeout(value: str) -> float:
    try:
        timeout = float(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("timeout must be a number") from exc
    if timeout <= 0:
        raise argparse.ArgumentTypeError("timeout must be positive")
    return timeout


def decode(data: bytes) -> str:
    return data.decode("utf-8", errors="replace")


def ensure_bytes(data: bytes | str | None) -> bytes:
    if data is None:
        return b""
    if isinstance(data, bytes):
        return data
    return data.encode("utf-8", errors="replace")


def format_seconds(seconds: float) -> str:
    return f"{seconds:g}"


def compiler_command() -> list[str]:
    cxx = os.environ.get("CXX")
    if cxx:
        return shlex.split(cxx)
    return ["g++"]


def sanitizer_link_error(result: subprocess.CompletedProcess[bytes]) -> bool:
    message = decode(result.stdout + result.stderr).lower()
    patterns = [
        "library 'asan' not found",
        "cannot find -lasan",
        "library not found for -lasan",
    ]
    return any(pattern in message for pattern in patterns)


def print_compile_failure(command: list[str], result: subprocess.CompletedProcess[bytes]) -> None:
    print(f"compile failed: {' '.join(command)}", file=sys.stderr)
    if result.stdout:
        print(decode(result.stdout), file=sys.stderr, end="")
    if result.stderr:
        print(decode(result.stderr), file=sys.stderr, end="")


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(Path.cwd().resolve()))
    except ValueError:
        return str(path)


def binary_is_fresh(source: Path, output: Path) -> bool:
    return output.exists() and output.stat().st_mtime_ns >= source.stat().st_mtime_ns


def compile_cpp(source: Path, output: Path) -> None:
    global _SANITIZER_AVAILABLE

    if binary_is_fresh(source, output):
        return

    flags = COMPILE_FLAGS if _SANITIZER_AVAILABLE is not False else BASE_COMPILE_FLAGS
    command = [*compiler_command(), *flags, str(source), "-o", str(output)]
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        print(f"Compiled {display_path(source)} -> {display_path(output)}")
        return

    if _SANITIZER_AVAILABLE is not False and sanitizer_link_error(result):
        _SANITIZER_AVAILABLE = False
        fallback_flags = BASE_COMPILE_FLAGS
        fallback_command = [*compiler_command(), *fallback_flags, str(source), "-o", str(output)]
        fallback_result = subprocess.run(fallback_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if fallback_result.returncode == 0:
            print(f"Compiled {display_path(source)} -> {display_path(output)}")
            return
        print_compile_failure(fallback_command, fallback_result)
        raise SystemExit(fallback_result.returncode)

    print_compile_failure(command, result)
    raise SystemExit(result.returncode)


def run_binary(
    command: list[str],
    input_data: bytes | None = None,
    timeout: float = DEFAULT_TIMEOUT_SECONDS,
) -> RunResult:
    try:
        result = subprocess.run(
            command,
            input=input_data,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as exc:
        stderr = ensure_bytes(exc.stderr)
        stderr += f"timeout after {format_seconds(timeout)} seconds\n".encode("utf-8")
        return RunResult(
            stdout=ensure_bytes(exc.output),
            stderr=stderr,
            returncode=TIMEOUT_RETURN_CODE,
        )
    return RunResult(
        stdout=result.stdout,
        stderr=result.stderr,
        returncode=result.returncode,
    )


def sample_pair_is_used(input_path: Path, output_path: Path) -> bool:
    return (input_path.exists() and input_path.stat().st_size > 0) or (
        output_path.exists() and output_path.stat().st_size > 0
    )


def find_sample_slot(test_dir: Path) -> int:
    index = 1
    while True:
        input_path = test_dir / f"sample-{index}.in"
        output_path = test_dir / f"sample-{index}.out"
        if not sample_pair_is_used(input_path, output_path):
            return index
        index += 1


def save_sample(test_dir: Path, input_data: bytes, expected: bytes) -> int:
    test_dir.mkdir(parents=True, exist_ok=True)
    index = find_sample_slot(test_dir)
    (test_dir / f"sample-{index}.in").write_bytes(input_data)
    (test_dir / f"sample-{index}.out").write_bytes(expected)
    return index


def require_file(path: Path) -> None:
    if not path.is_file():
        print(f"rt: required file not found: {path}", file=sys.stderr)
        raise SystemExit(1)


def print_output_block(data: bytes) -> None:
    text = decode(data)
    if text:
        print(text, end="" if text.endswith("\n") else "\n")
    print()


def print_failure(index: int, input_data: bytes, output: bytes, expected: bytes) -> None:
    print(f"{index + 1}回目")
    print("=== Test Case ===")
    print_output_block(input_data)
    print("=== Expected Output (naive) ===")
    print_output_block(expected)
    print("=== Your Output (main) ===")
    print_output_block(output)


def run_random_tests(count: int, save: bool, timeout: float = DEFAULT_TIMEOUT_SECONDS) -> int:
    problem_dir = Path.cwd()
    main_cpp = problem_dir / "main.cpp"
    random_dir = problem_dir / "randomTest"
    gen_cpp = random_dir / "gen.cpp"
    naive_cpp = random_dir / "naive.cpp"
    test_dir = problem_dir / "test"
    rt_dir = random_dir / ".rt"

    require_file(main_cpp)
    require_file(gen_cpp)
    require_file(naive_cpp)

    rt_dir.mkdir(parents=True, exist_ok=True)
    main_bin = rt_dir / "main"
    gen_bin = rt_dir / "gen"
    naive_bin = rt_dir / "naive"

    compile_cpp(main_cpp, main_bin)
    compile_cpp(gen_cpp, gen_bin)
    compile_cpp(naive_cpp, naive_bin)

    for index in range(count):
        generated = run_binary([str(gen_bin)], timeout=timeout)
        if generated.returncode != 0:
            print(f"gen failed at {index + 1}回目", file=sys.stderr)
            print(decode(generated.stderr), file=sys.stderr, end="")
            return generated.returncode

        input_data = generated.stdout
        output = run_binary([str(main_bin)], input_data, timeout=timeout)
        expected = run_binary([str(naive_bin)], input_data, timeout=timeout)

        failed = output.returncode != 0 or expected.returncode != 0 or output.stdout != expected.stdout

        if failed:
            print_failure(index, input_data, output.stdout, expected.stdout)

            if output.returncode != 0:
                print(f"main exited with status {output.returncode}", file=sys.stderr)
                print(decode(output.stderr), file=sys.stderr, end="")
            if expected.returncode != 0:
                print(f"naive exited with status {expected.returncode}", file=sys.stderr)
                print(decode(expected.stderr), file=sys.stderr, end="")

            if save:
                save_sample(test_dir, input_data, expected.stdout)
            return 1

    print("OK")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Run random tests for an ICPC problem directory.")
    parser.add_argument("-s", "--save", action="store_true", help="save a failing case to test/sample-k.in/out")
    parser.add_argument(
        "--timeout",
        default=DEFAULT_TIMEOUT_SECONDS,
        type=parse_timeout,
        help=f"seconds before each run is killed (default: {format_seconds(DEFAULT_TIMEOUT_SECONDS)})",
    )
    parser.add_argument("count", nargs="?", default=100, type=parse_count, help="number of tests to run")
    args = parser.parse_args()
    return run_random_tests(args.count, args.save, args.timeout)


if __name__ == "__main__":
    raise SystemExit(main())
