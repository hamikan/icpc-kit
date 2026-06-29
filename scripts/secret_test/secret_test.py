from __future__ import annotations

import os
import shlex
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Tuple


DEFAULT_INPUT_EXTENSION = "in"
DEFAULT_OUTPUT_EXTENSION = "out"
DEFAULT_TIMEOUT_SECONDS = 2.0
TLE_CONFIRMATION_RUNS = 2
COMPILE_FLAGS = [
    "-std=gnu++20",
    "-O2",
    "-Wall",
    "-Wextra",
    "-Wshadow",
    "-DLOCAL",
]
STATUS_PRIORITY = {"AC": 0, "WA": 1, "RE": 2, "TLE": 3}
USAGE = "usage: st [-t SECONDS|--timeout SECONDS] [INPUT_EXT OUTPUT_EXT]\n       st --reset"


class SecretTestError(Exception):
    pass


@dataclass(frozen=True)
class Options:
    input_extension: str
    output_extension: str
    timeout: float
    reset: bool


@dataclass(frozen=True)
class CasePair:
    name: str
    input_path: Path
    expected_path: Path


@dataclass(frozen=True)
class RunResult:
    status: str
    stdout: bytes
    returncode: int


def project_root() -> Path:
    if os.environ.get("ICPC_KIT"):
        return Path(os.environ["ICPC_KIT"])
    return Path(__file__).resolve().parents[2]


def compiler_command() -> List[str]:
    cxx = os.environ.get("CXX")
    if cxx:
        return shlex.split(cxx)
    return ["g++"]


def parse_timeout(value: str) -> float:
    try:
        timeout = float(value)
    except ValueError as exc:
        raise SecretTestError("timeout must be a number") from exc
    if timeout <= 0:
        raise SecretTestError("timeout must be positive")
    return timeout


def normalize_extension(value: str) -> str:
    if value == "":
        raise ValueError("extension must not be empty")
    if value.startswith("."):
        raise ValueError("extension must not start with '.'")
    if "/" in value or "." in value:
        raise ValueError("extension must not contain '/' or '.'")
    return value


def validate_extensions(input_extension: str, output_extension: str) -> None:
    if input_extension == "_" and output_extension == "_":
        raise ValueError("'_' cannot be used for both extensions")
    if input_extension == output_extension:
        raise ValueError("input and output extensions must be different")


def parse_args(argv: List[str]) -> Options:
    timeout = DEFAULT_TIMEOUT_SECONDS
    reset = False
    positional: List[str] = []
    index = 0

    while index < len(argv):
        arg = argv[index]
        if arg in ("-t", "--timeout"):
            index += 1
            if index >= len(argv):
                raise SecretTestError("--timeout requires a value")
            timeout = parse_timeout(argv[index])
        elif arg == "--reset":
            reset = True
        elif arg in ("-h", "--help"):
            print(USAGE)
            raise SystemExit(0)
        elif arg.startswith("-"):
            raise SecretTestError(f"unknown option: {arg}")
        else:
            positional.append(arg)
        index += 1

    if reset and positional:
        raise SecretTestError("--reset does not take extensions")
    if len(positional) == 0:
        input_extension = DEFAULT_INPUT_EXTENSION
        output_extension = DEFAULT_OUTPUT_EXTENSION
    elif len(positional) == 2:
        try:
            input_extension = normalize_extension(positional[0])
            output_extension = normalize_extension(positional[1])
            validate_extensions(input_extension, output_extension)
        except ValueError as exc:
            raise SecretTestError(str(exc)) from exc
    else:
        raise SecretTestError("expected zero or two extensions")

    return Options(
        input_extension=input_extension,
        output_extension=output_extension,
        timeout=timeout,
        reset=reset,
    )


def case_name(path: Path, extension: str) -> Optional[str]:
    if path.name.startswith(".") or not path.is_file():
        return None
    if extension == "_":
        if "." in path.name:
            return None
        return path.name

    suffix = f".{extension}"
    if not path.name.endswith(suffix) or len(path.name) <= len(suffix):
        return None
    return path.name[: -len(suffix)]


def collect_case_pairs(secret_dir: Path, input_extension: str, output_extension: str) -> List[CasePair]:
    if not secret_dir.is_dir():
        raise SecretTestError("secret directory not found")

    inputs = {}
    outputs = {}
    for path in secret_dir.iterdir():
        input_name = case_name(path, input_extension)
        if input_name is not None:
            inputs[input_name] = path
        output_name = case_name(path, output_extension)
        if output_name is not None:
            outputs[output_name] = path

    missing = sorted(set(inputs) ^ set(outputs))
    if missing:
        raise SecretTestError(f"unmatched secret case: {missing[0]}")
    if not inputs:
        raise SecretTestError("no secret cases found")

    return [
        CasePair(name=name, input_path=inputs[name], expected_path=outputs[name])
        for name in sorted(inputs)
    ]


def compile_cpp(problem_dir: Path, root: Path) -> bool:
    source = problem_dir / "main.cpp"
    if not source.is_file():
        return False

    output = problem_dir / "a.out"
    include_flags = []
    ac_library = root / "ac-library"
    if ac_library.is_dir():
        include_flags = ["-I", str(ac_library)]
    command = [
        *compiler_command(),
        *COMPILE_FLAGS,
        *include_flags,
        str(source),
        "-o",
        str(output),
    ]
    result = subprocess.run(
        command,
        cwd=problem_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.returncode == 0


def run_binary(problem_dir: Path, input_path: Path, timeout: float) -> RunResult:
    try:
        with input_path.open("rb") as input_file, tempfile.TemporaryFile() as output_file:
            result = subprocess.run(
                ["./a.out"],
                cwd=problem_dir,
                stdin=input_file,
                stdout=output_file,
                stderr=subprocess.DEVNULL,
                timeout=timeout,
            )
            output_file.seek(0)
            stdout = output_file.read()
    except subprocess.TimeoutExpired as exc:
        return RunResult(status="TLE", stdout=b"", returncode=124)

    if result.returncode != 0:
        return RunResult(status="RE", stdout=stdout, returncode=result.returncode)
    return RunResult(status="AC", stdout=stdout, returncode=0)


def judge_output(output: bytes, expected: bytes) -> bool:
    return output.split() == expected.split()


def run_cases(problem_dir: Path, pairs: List[CasePair], timeout: float) -> Tuple[str, int, int]:
    status = "AC"
    accepted = 0
    run_binary(problem_dir, pairs[0].input_path, timeout)
    for pair in pairs:
        result = run_binary(problem_dir, pair.input_path, timeout)
        for _ in range(TLE_CONFIRMATION_RUNS - 1):
            if result.status != "TLE":
                break
            result = run_binary(problem_dir, pair.input_path, timeout)
        if result.status == "AC" and judge_output(result.stdout, pair.expected_path.read_bytes()):
            accepted += 1
            continue
        case_status = result.status if result.status != "AC" else "WA"
        if STATUS_PRIORITY[case_status] > STATUS_PRIORITY[status]:
            status = case_status
    return status, accepted, len(pairs)


def penalty_path(problem_dir: Path) -> Path:
    return problem_dir / ".st" / "penalty"


def read_penalty(problem_dir: Path) -> int:
    path = penalty_path(problem_dir)
    if not path.is_file():
        return 0
    try:
        return int(path.read_text(encoding="utf-8").strip())
    except ValueError:
        return 0


def write_penalty(problem_dir: Path, penalty: int) -> None:
    path = penalty_path(problem_dir)
    path.parent.mkdir(exist_ok=True)
    path.write_text(f"{penalty}\n", encoding="utf-8")


def reset_penalty(problem_dir: Path) -> None:
    write_penalty(problem_dir, 0)


def format_verdict(status: str, penalty: int, accepted: int, total: int) -> str:
    if status == "AC" and penalty == 0:
        return f"AC {accepted} / {total}"
    return f"{status}({penalty}) {accepted} / {total}"


def run_secret_tests(options: Options) -> int:
    problem_dir = Path.cwd()
    if options.reset:
        reset_penalty(problem_dir)
        print("penalty reset")
        return 0

    pairs = collect_case_pairs(problem_dir / "secret", options.input_extension, options.output_extension)
    if not compile_cpp(problem_dir, project_root()):
        print("CE")
        return 1

    status, accepted, total = run_cases(problem_dir, pairs, options.timeout)
    penalty = read_penalty(problem_dir)
    if status != "AC":
        penalty += 1
        write_penalty(problem_dir, penalty)

    print(format_verdict(status, penalty, accepted, total))
    return 0 if status == "AC" else 1


def main(argv: List[str]) -> int:
    try:
        options = parse_args(argv)
        return run_secret_tests(options)
    except SecretTestError as error:
        print(f"st: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
