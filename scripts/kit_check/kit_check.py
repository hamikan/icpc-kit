from __future__ import annotations

import contextlib
import importlib.util
import io
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def load_random_test_module():
    spec = importlib.util.spec_from_file_location(
        "random_test",
        ROOT / "scripts" / "random_test" / "random_test.py",
    )
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def load_library_insert_module():
    spec = importlib.util.spec_from_file_location(
        "library_insert",
        ROOT / "scripts" / "library_insert" / "library_insert.py",
    )
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def load_secret_test_module():
    spec = importlib.util.spec_from_file_location(
        "secret_test",
        ROOT / "scripts" / "secret_test" / "secret_test.py",
    )
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class SampleSlotTests(unittest.TestCase):
    def setUp(self) -> None:
        self.random_test = load_random_test_module()
        self.tmp = tempfile.TemporaryDirectory()
        self.test_dir = Path(self.tmp.name)

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def test_empty_existing_pair_is_reused(self) -> None:
        (self.test_dir / "sample-1.in").touch()
        (self.test_dir / "sample-1.out").touch()

        slot = self.random_test.save_sample(self.test_dir, b"input\n", b"answer\n")

        self.assertEqual(slot, 1)
        self.assertEqual((self.test_dir / "sample-1.in").read_bytes(), b"input\n")
        self.assertEqual((self.test_dir / "sample-1.out").read_bytes(), b"answer\n")

    def test_nonempty_input_marks_pair_used(self) -> None:
        (self.test_dir / "sample-1.in").write_bytes(b"used\n")
        (self.test_dir / "sample-1.out").touch()

        slot = self.random_test.save_sample(self.test_dir, b"input\n", b"answer\n")

        self.assertEqual(slot, 2)

    def test_nonempty_output_marks_pair_used(self) -> None:
        (self.test_dir / "sample-1.in").touch()
        (self.test_dir / "sample-1.out").write_bytes(b"used\n")

        slot = self.random_test.save_sample(self.test_dir, b"input\n", b"answer\n")

        self.assertEqual(slot, 2)


class CompilerSelectionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.random_test = load_random_test_module()
        self.previous_sanitizer_available = getattr(self.random_test, "_SANITIZER_AVAILABLE", None)

    def tearDown(self) -> None:
        self.random_test._SANITIZER_AVAILABLE = self.previous_sanitizer_available

    def test_random_test_defaults_to_plain_gplusplus_when_cxx_is_unset(self) -> None:
        previous_cxx = os.environ.pop("CXX", None)
        try:
            self.assertEqual(self.random_test.compiler_command(), ["g++"])
        finally:
            if previous_cxx is not None:
                os.environ["CXX"] = previous_cxx

    def test_random_test_uses_cxx_when_set(self) -> None:
        previous_cxx = os.environ.get("CXX")
        os.environ["CXX"] = "ccache g++-14"
        try:
            self.assertEqual(self.random_test.compiler_command(), ["ccache", "g++-14"])
        finally:
            if previous_cxx is None:
                os.environ.pop("CXX", None)
            else:
                os.environ["CXX"] = previous_cxx

    def test_compile_retries_without_sanitizer_silently_when_asan_is_missing(self) -> None:
        calls: list[list[str]] = []
        original_run = self.random_test.subprocess.run
        self.random_test._SANITIZER_AVAILABLE = None
        stderr = io.StringIO()
        stdout = io.StringIO()

        def fake_run(command, stdout, stderr):
            calls.append(command)
            if len(calls) == 1:
                return subprocess.CompletedProcess(command, 1, b"", b"ld: library 'asan' not found\n")
            return subprocess.CompletedProcess(command, 0, b"", b"")

        self.random_test.subprocess.run = fake_run
        try:
            with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
                self.random_test.compile_cpp(Path("main.cpp"), Path("main"))
        finally:
            self.random_test.subprocess.run = original_run

        self.assertEqual(len(calls), 2)
        self.assertIn("-fsanitize=undefined,address", calls[0])
        self.assertNotIn("-fsanitize=undefined,address", calls[1])
        self.assertEqual(stderr.getvalue(), "")

    def test_compile_skips_when_binary_is_newer_than_source(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            source = tmp_path / "main.cpp"
            output = tmp_path / "main"
            source.write_text("int main(){return 0;}\n", encoding="utf-8")
            output.write_text("compiled\n", encoding="utf-8")
            os.utime(source, ns=(1_000_000_000, 1_000_000_000))
            os.utime(output, ns=(2_000_000_000, 2_000_000_000))

            original_run = self.random_test.subprocess.run
            calls: list[list[str]] = []

            def fake_run(command, stdout, stderr):
                calls.append(command)
                return subprocess.CompletedProcess(command, 0, b"", b"")

            self.random_test.subprocess.run = fake_run
            try:
                self.random_test.compile_cpp(source, output)
            finally:
                self.random_test.subprocess.run = original_run

            self.assertEqual(calls, [])

    def test_compile_runs_when_source_is_newer_than_binary(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            source = tmp_path / "main.cpp"
            output = tmp_path / "main"
            source.write_text("int main(){return 0;}\n", encoding="utf-8")
            output.write_text("compiled\n", encoding="utf-8")
            os.utime(output, ns=(1_000_000_000, 1_000_000_000))
            os.utime(source, ns=(2_000_000_000, 2_000_000_000))

            original_run = self.random_test.subprocess.run
            calls: list[list[str]] = []

            def fake_run(command, stdout, stderr):
                calls.append(command)
                return subprocess.CompletedProcess(command, 0, b"", b"")

            self.random_test.subprocess.run = fake_run
            try:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.random_test.compile_cpp(source, output)
            finally:
                self.random_test.subprocess.run = original_run

            self.assertEqual(len(calls), 1)

    def test_compile_prints_success_message(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            source = tmp_path / "main.cpp"
            output = tmp_path / "main"
            source.write_text("int main(){return 0;}\n", encoding="utf-8")
            stdout = io.StringIO()

            original_run = self.random_test.subprocess.run

            def fake_run(command, stdout, stderr):
                return subprocess.CompletedProcess(command, 0, b"", b"")

            self.random_test.subprocess.run = fake_run
            try:
                with contextlib.redirect_stdout(stdout):
                    self.random_test.compile_cpp(source, output)
            finally:
                self.random_test.subprocess.run = original_run

            self.assertIn(f"Compiled {source} -> {output}", stdout.getvalue())

    def test_run_binary_passes_timeout_to_subprocess(self) -> None:
        original_run = self.random_test.subprocess.run
        calls: list[float | None] = []

        def fake_run(command, input, stdout, stderr, timeout):
            calls.append(timeout)
            return subprocess.CompletedProcess(command, 0, b"ok\n", b"")

        self.random_test.subprocess.run = fake_run
        try:
            result = self.random_test.run_binary(["main"], b"input\n", timeout=1.5)
        finally:
            self.random_test.subprocess.run = original_run

        self.assertEqual(calls, [1.5])
        self.assertEqual(result.stdout, b"ok\n")
        self.assertEqual(result.returncode, 0)

    def test_run_binary_reports_timeout_as_failure(self) -> None:
        original_run = self.random_test.subprocess.run

        def fake_run(command, input, stdout, stderr, timeout):
            raise subprocess.TimeoutExpired(command, timeout, output=b"partial\n", stderr=b"")

        self.random_test.subprocess.run = fake_run
        try:
            result = self.random_test.run_binary(["main"], b"input\n", timeout=2.0)
        finally:
            self.random_test.subprocess.run = original_run

        self.assertEqual(result.stdout, b"partial\n")
        self.assertIn(b"timeout after 2 seconds", result.stderr)
        self.assertEqual(result.returncode, 124)

    def test_display_path_uses_relative_path_inside_current_directory(self) -> None:
        self.assertEqual(
            self.random_test.display_path(Path.cwd() / "randomTest" / ".rt" / "main"),
            "randomTest/.rt/main",
        )

    def test_print_failure_uses_problem_output_sections(self) -> None:
        stdout = io.StringIO()

        with contextlib.redirect_stdout(stdout):
            self.random_test.print_failure(
                2,
                b"input\n",
                b"wrong\n",
                b"right\n",
            )

        output = stdout.getvalue()
        self.assertEqual(
            output,
            "3回目\n"
            "=== Test Case ===\n"
            "input\n"
            "\n"
            "=== Expected Output (naive) ===\n"
            "right\n"
            "\n"
            "=== Your Output (main) ===\n"
            "wrong\n"
            "\n",
        )

    def test_run_random_tests_saves_failing_case_when_requested(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            (tmp_path / "main.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest").mkdir()
            (tmp_path / "randomTest" / "gen.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest" / "naive.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")

            original_cwd = Path.cwd()
            original_compile_cpp = self.random_test.compile_cpp
            original_run_binary = self.random_test.run_binary

            calls = iter(
                [
                    self.random_test.RunResult(b"input\n", b"", 0),
                    self.random_test.RunResult(b"wrong\n", b"", 0),
                    self.random_test.RunResult(b"right\n", b"", 0),
                ]
            )
            stdout = io.StringIO()

            self.random_test.compile_cpp = lambda source, output: None
            self.random_test.run_binary = lambda command, input_data=None, timeout=None: next(calls)
            try:
                os.chdir(tmp_path)
                with contextlib.redirect_stdout(stdout):
                    exit_code = self.random_test.run_random_tests(1, save=True)
            finally:
                os.chdir(original_cwd)
                self.random_test.compile_cpp = original_compile_cpp
                self.random_test.run_binary = original_run_binary

            self.assertEqual(exit_code, 1)
            self.assertEqual((tmp_path / "test" / "sample-1.in").read_bytes(), b"input\n")
            self.assertEqual((tmp_path / "test" / "sample-1.out").read_bytes(), b"right\n")

    def test_run_random_tests_runs_generator_without_command_line_arguments(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            (tmp_path / "main.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest").mkdir()
            (tmp_path / "randomTest" / "gen.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest" / "naive.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")

            original_cwd = Path.cwd()
            original_compile_cpp = self.random_test.compile_cpp
            original_run_binary = self.random_test.run_binary

            commands: list[list[str]] = []
            calls = iter(
                [
                    self.random_test.RunResult(b"input\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                ]
            )

            def fake_run_binary(command, input_data=None, timeout=None):
                commands.append(command)
                return next(calls)

            self.random_test.compile_cpp = lambda source, output: None
            self.random_test.run_binary = fake_run_binary
            try:
                os.chdir(tmp_path)
                with contextlib.redirect_stdout(io.StringIO()):
                    exit_code = self.random_test.run_random_tests(1, save=False)
            finally:
                os.chdir(original_cwd)
                self.random_test.compile_cpp = original_compile_cpp
                self.random_test.run_binary = original_run_binary

            self.assertEqual(exit_code, 0)
            self.assertEqual(len(commands[0]), 1)
            self.assertEqual(
                Path(commands[0][0]).resolve(),
                (tmp_path / "randomTest" / ".rt" / "gen").resolve(),
            )

    def test_run_random_tests_uses_default_two_second_timeout(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            (tmp_path / "main.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest").mkdir()
            (tmp_path / "randomTest" / "gen.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest" / "naive.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")

            original_cwd = Path.cwd()
            original_compile_cpp = self.random_test.compile_cpp
            original_run_binary = self.random_test.run_binary

            timeouts: list[float | None] = []
            calls = iter(
                [
                    self.random_test.RunResult(b"input\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                ]
            )

            def fake_run_binary(command, input_data=None, timeout=None):
                timeouts.append(timeout)
                return next(calls)

            self.random_test.compile_cpp = lambda source, output: None
            self.random_test.run_binary = fake_run_binary
            try:
                os.chdir(tmp_path)
                with contextlib.redirect_stdout(io.StringIO()):
                    exit_code = self.random_test.run_random_tests(1, save=False)
            finally:
                os.chdir(original_cwd)
                self.random_test.compile_cpp = original_compile_cpp
                self.random_test.run_binary = original_run_binary

            self.assertEqual(exit_code, 0)
            self.assertEqual(timeouts, [2.0, 2.0, 2.0])

    def test_run_random_tests_passes_custom_timeout(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            (tmp_path / "main.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest").mkdir()
            (tmp_path / "randomTest" / "gen.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")
            (tmp_path / "randomTest" / "naive.cpp").write_text("int main(){return 0;}\n", encoding="utf-8")

            original_cwd = Path.cwd()
            original_compile_cpp = self.random_test.compile_cpp
            original_run_binary = self.random_test.run_binary

            timeouts: list[float | None] = []
            calls = iter(
                [
                    self.random_test.RunResult(b"input\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                    self.random_test.RunResult(b"answer\n", b"", 0),
                ]
            )

            def fake_run_binary(command, input_data=None, timeout=None):
                timeouts.append(timeout)
                return next(calls)

            self.random_test.compile_cpp = lambda source, output: None
            self.random_test.run_binary = fake_run_binary
            try:
                os.chdir(tmp_path)
                with contextlib.redirect_stdout(io.StringIO()):
                    exit_code = self.random_test.run_random_tests(1, save=False, timeout=4.5)
            finally:
                os.chdir(original_cwd)
                self.random_test.compile_cpp = original_compile_cpp
                self.random_test.run_binary = original_run_binary

            self.assertEqual(exit_code, 0)
            self.assertEqual(timeouts, [4.5, 4.5, 4.5])


class InitCompilerSelectionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.fake_bin = Path(self.tmp.name)
        fake_compiler = self.fake_bin / "g++-15"
        fake_compiler.write_text("#!/usr/bin/env bash\nprintf 'fake g++-15\\n'\n", encoding="utf-8")
        fake_compiler.chmod(0o755)

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def run_bash(self, command: str, env: dict[str, str]) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            ["bash", "-lc", command],
            cwd=ROOT,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_init_selects_first_available_versioned_gplusplus(self) -> None:
        env = {**os.environ, "PATH": f"{self.fake_bin}:{os.environ['PATH']}"}
        env.pop("CXX", None)

        result = self.run_bash("source init.sh && printf '%s' \"$CXX\"", env)

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "g++-15")

    def test_init_preserves_existing_cxx(self) -> None:
        env = {**os.environ, "CXX": "custom-cxx"}

        result = self.run_bash("source init.sh && printf '%s' \"$CXX\"", env)

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "custom-cxx")


class CheckCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.fake_bin = self.root / "fake-bin"
        self.fake_bin.mkdir()
        (self.root / "bin").mkdir()
        shutil.copy2(ROOT / "bin" / "check", self.root / "bin" / "check")

        for command in ["python3", "g++", "oj", "nw", "rt", "ace", "lib", "st"]:
            path = self.fake_bin / command
            path.write_text("#!/usr/bin/env bash\nexit 0\n", encoding="utf-8")
            path.chmod(0o755)

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def test_check_reports_commands_without_template_file_checks(self) -> None:
        env = {
            **os.environ,
            "CXX": "true",
            "ICPC_KIT": str(self.root),
            "PATH": f"{self.root / 'bin'}:{self.fake_bin}:{os.environ['PATH']}",
        }

        result = subprocess.run(
            ["bash", "-lc", "check"],
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("OK   nw", result.stdout)
        self.assertIn("OK   lib", result.stdout)
        self.assertIn("OK   st", result.stdout)
        self.assertIn("[INFO] compiler feature checks", result.stdout)
        self.assertNotIn("template/", result.stdout)


class RtCommandTests(unittest.TestCase):
    def test_rt_help_runs_through_command_entrypoint(self) -> None:
        env = {**os.environ, "ICPC_KIT": str(ROOT)}

        result = subprocess.run(
            [str(ROOT / "bin" / "rt"), "--help"],
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("number of tests to run", result.stdout)


class SecretTestUnitTests(unittest.TestCase):
    def setUp(self) -> None:
        self.secret_test = load_secret_test_module()
        self.tmp = tempfile.TemporaryDirectory()
        self.problem_dir = Path(self.tmp.name)
        self.secret_dir = self.problem_dir / "secret"
        self.secret_dir.mkdir()

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def test_pairs_cases_by_extension_name(self) -> None:
        (self.secret_dir / "sample.in").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "sample.out").write_text("1\n", encoding="utf-8")
        (self.secret_dir / ".DS_Store").write_text("ignored\n", encoding="utf-8")

        pairs = self.secret_test.collect_case_pairs(self.secret_dir, "in", "out")

        self.assertEqual(len(pairs), 1)
        self.assertEqual(pairs[0].name, "sample")

    def test_pairs_cases_with_missing_input_extension(self) -> None:
        (self.secret_dir / "sample").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "sample.out").write_text("1\n", encoding="utf-8")

        pairs = self.secret_test.collect_case_pairs(self.secret_dir, "_", "out")

        self.assertEqual(len(pairs), 1)
        self.assertEqual(pairs[0].input_path.name, "sample")
        self.assertEqual(pairs[0].expected_path.name, "sample.out")

    def test_rejects_dot_prefixed_extension(self) -> None:
        with self.assertRaisesRegex(ValueError, "must not start with"):
            self.secret_test.normalize_extension(".in")

    def test_rejects_same_extensions(self) -> None:
        with self.assertRaisesRegex(ValueError, "must be different"):
            self.secret_test.validate_extensions("in", "in")

    def test_formats_penalty_results(self) -> None:
        self.assertEqual(self.secret_test.format_verdict("AC", 0, 8, 8), "AC 8 / 8")
        self.assertEqual(self.secret_test.format_verdict("AC", 2, 14, 14), "AC(2) 14 / 14")
        self.assertEqual(self.secret_test.format_verdict("WA", 3, 10, 14), "WA(3) 10 / 14")

    def test_run_cases_keeps_running_and_prefers_highest_priority_status(self) -> None:
        for name in ["ac", "wa", "re", "tle"]:
            (self.secret_dir / f"{name}.in").write_text(name, encoding="utf-8")
            (self.secret_dir / f"{name}.out").write_text("ok", encoding="utf-8")
        pairs = self.secret_test.collect_case_pairs(self.secret_dir, "in", "out")
        calls: list[str] = []
        original_run_binary = self.secret_test.run_binary

        def fake_run_binary(problem_dir: Path, input_path: Path, timeout: float):
            name = input_path.read_text(encoding="utf-8")
            calls.append(name)
            if name == "ac":
                return self.secret_test.RunResult(status="AC", stdout=b"ok\n", returncode=0)
            if name == "wa":
                return self.secret_test.RunResult(status="AC", stdout=b"wrong\n", returncode=0)
            if name == "re":
                return self.secret_test.RunResult(status="RE", stdout=b"", returncode=1)
            return self.secret_test.RunResult(status="TLE", stdout=b"", returncode=124)

        self.secret_test.run_binary = fake_run_binary
        try:
            result = self.secret_test.run_cases(self.problem_dir, pairs, timeout=2.0)
        finally:
            self.secret_test.run_binary = original_run_binary

        self.assertEqual(result, ("TLE", 1, 4))
        self.assertEqual(calls, ["ac", "ac", "re", "tle", "tle", "wa"])

    def test_tle_is_retried_until_a_later_run_finishes_before_judging(self) -> None:
        (self.secret_dir / "sample.in").write_text("sample", encoding="utf-8")
        (self.secret_dir / "sample.out").write_text("ok", encoding="utf-8")
        pairs = self.secret_test.collect_case_pairs(self.secret_dir, "in", "out")
        calls: list[str] = []
        original_run_binary = self.secret_test.run_binary

        def fake_run_binary(problem_dir: Path, input_path: Path, timeout: float):
            calls.append(input_path.read_text(encoding="utf-8"))
            if len(calls) == 2:
                return self.secret_test.RunResult(status="TLE", stdout=b"", returncode=124)
            return self.secret_test.RunResult(status="AC", stdout=b"ok\n", returncode=0)

        self.secret_test.run_binary = fake_run_binary
        try:
            result = self.secret_test.run_cases(self.problem_dir, pairs, timeout=2.0)
        finally:
            self.secret_test.run_binary = original_run_binary

        self.assertEqual(result, ("AC", 1, 1))
        self.assertEqual(calls, ["sample", "sample", "sample"])

    def test_run_binary_reads_stdin_from_input_file(self) -> None:
        binary = self.problem_dir / "a.out"
        input_path = self.problem_dir / "input.txt"
        binary.write_text("#!/bin/sh\ncat\n", encoding="utf-8")
        binary.chmod(0o755)
        input_path.write_text("hello\n", encoding="utf-8")

        result = self.secret_test.run_binary(self.problem_dir, input_path, timeout=2.0)

        self.assertEqual(result.status, "AC")
        self.assertEqual(result.stdout, b"hello\n")


class SecretTestCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.problem_dir = Path(self.tmp.name)
        self.secret_dir = self.problem_dir / "secret"
        self.secret_dir.mkdir()
        self.main_cpp = self.problem_dir / "main.cpp"

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def run_st(self, *args: str) -> subprocess.CompletedProcess[str]:
        env = {**os.environ, "ICPC_KIT": str(ROOT), "CXX": os.environ.get("CXX", "g++")}
        return subprocess.run(
            [str(ROOT / "bin" / "st"), *args],
            cwd=self.problem_dir,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def write_echo_program(self) -> None:
        self.main_cpp.write_text(
            "#include <iostream>\n"
            "using namespace std;\n"
            "int main(){ long long x; if(cin >> x) cout << x << '\\n'; }\n",
            encoding="utf-8",
        )

    def test_secret_test_reports_ac_without_penalty(self) -> None:
        self.write_echo_program()
        (self.secret_dir / "one.in").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "one.out").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "two.in").write_text("2\n", encoding="utf-8")
        (self.secret_dir / "two.out").write_text("2\n", encoding="utf-8")

        result = self.run_st("in", "out")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "AC 2 / 2\n")

    def test_secret_test_runs_all_cases_and_keeps_penalty(self) -> None:
        self.write_echo_program()
        (self.secret_dir / "one.in").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "one.out").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "two.in").write_text("2\n", encoding="utf-8")
        (self.secret_dir / "two.out").write_text("wrong\n", encoding="utf-8")

        first = self.run_st("in", "out")
        (self.secret_dir / "two.out").write_text("2\n", encoding="utf-8")
        second = self.run_st("in", "out")

        self.assertNotEqual(first.returncode, 0)
        self.assertEqual(first.stdout, "WA(1) 1 / 2\n")
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertEqual(second.stdout, "AC(1) 2 / 2\n")

    def test_secret_test_supports_missing_input_extension(self) -> None:
        self.write_echo_program()
        (self.secret_dir / "one").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "one.out").write_text("1\n", encoding="utf-8")

        result = self.run_st("_", "out")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "AC 1 / 1\n")

    def test_compile_error_does_not_increment_penalty(self) -> None:
        self.main_cpp.write_text("int main( {", encoding="utf-8")
        (self.secret_dir / "one.in").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "one.out").write_text("1\n", encoding="utf-8")

        ce = self.run_st("in", "out")
        self.write_echo_program()
        ac = self.run_st("in", "out")

        self.assertNotEqual(ce.returncode, 0)
        self.assertEqual(ce.stdout, "CE\n")
        self.assertEqual(ac.returncode, 0, ac.stderr)
        self.assertEqual(ac.stdout, "AC 1 / 1\n")

    def test_reset_clears_penalty(self) -> None:
        self.write_echo_program()
        (self.secret_dir / "one.in").write_text("1\n", encoding="utf-8")
        (self.secret_dir / "one.out").write_text("wrong\n", encoding="utf-8")

        first = self.run_st("in", "out")
        reset = self.run_st("--reset")
        (self.secret_dir / "one.out").write_text("1\n", encoding="utf-8")
        ac = self.run_st("in", "out")

        self.assertEqual(first.stdout, "WA(1) 0 / 1\n")
        self.assertEqual(reset.returncode, 0, reset.stderr)
        self.assertEqual(reset.stdout, "penalty reset\n")
        self.assertEqual(ac.stdout, "AC 1 / 1\n")


class LibraryInsertUnitTests(unittest.TestCase):
    def setUp(self) -> None:
        self.library_insert = load_library_insert_module()

    def test_duplicate_removal_only_targets_prologue_lines(self) -> None:
        body = (
            "using ll = long long;\n"
            "\n"
            "template<typename T>\n"
            "struct Box {};\n"
        )
        source = (
            "using ll = long long;\n"
            "\n"
            "template<typename T>\n"
            "struct Existing {};\n"
        )

        result = self.library_insert.remove_duplicate_prologue_lines(body, source)

        self.assertNotIn("using ll = long long;", result)
        self.assertIn("template<typename T>", result)
        self.assertIn("struct Box", result)

    def test_duplicate_non_prologue_line_at_top_is_kept(self) -> None:
        body = (
            "template<typename T>\n"
            "struct Box {};\n"
        )

        result = self.library_insert.remove_duplicate_prologue_lines(body, body)

        self.assertEqual(result, body)

    def test_using_alias_duplicate_ignores_atcoder_namespace_for_comparison_only(self) -> None:
        body = (
            "using mint = atcoder::modint998244353;\n"
            "\n"
            "struct Box {};\n"
        )
        source = "using mint = modint998244353;\n"

        result = self.library_insert.remove_duplicate_prologue_lines(body, source)

        self.assertNotIn("using mint", result)
        self.assertIn("struct Box", result)

    def test_atcoder_namespace_is_not_removed_from_inserted_code(self) -> None:
        body = (
            "using mint = atcoder::modint998244353;\n"
            "\n"
            "struct Box {};\n"
        )

        result = self.library_insert.remove_duplicate_prologue_lines(body, "")

        self.assertIn("using mint = atcoder::modint998244353;", result)


class LibraryCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.problem_dir = Path(self.tmp.name)
        self.main_cpp = self.problem_dir / "main.cpp"
        self.main_cpp.write_text(
            "#include <bits/stdc++.h>\n"
            "using namespace std;\n"
            "\n"
            "#define rep(i, n) for (int i=0; i<n; i++)\n"
            "\n"
            "int main() {\n"
            "    return 0;\n"
            "}\n",
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def run_lib(self, *args: str) -> subprocess.CompletedProcess[str]:
        env = {**os.environ, "ICPC_KIT": str(ROOT)}
        return subprocess.run(
            [str(ROOT / "bin" / "lib"), *args],
            cwd=self.problem_dir,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_inserts_library_before_main(self) -> None:
        result = self.run_lib("seg")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertLess(source.index("struct SegTree"), source.index("int main()"))
        self.assertEqual(source.count("#include <bits/stdc++.h>"), 1)
        self.assertIn("inserted: seg", result.stdout)

    def test_keeps_include_using_define_group_together(self) -> None:
        self.main_cpp.write_text(
            "#include <bits/stdc++.h>\n"
            "using namespace std;\n"
            "\n"
            "int main() {\n"
            "    return 0;\n"
            "}\n",
            encoding="utf-8",
        )

        result = self.run_lib("bit")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(
            "#include <bits/stdc++.h>\n"
            "using namespace std;\n"
            "using ll = long long;\n"
            "\n"
            "struct Ops",
            source,
        )

    def test_skips_duplicate_prologue_lines(self) -> None:
        self.main_cpp.write_text((ROOT / "template" / "2" / "main.cpp").read_text(encoding="utf-8"), encoding="utf-8")

        result = self.run_lib("bit")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(source.count("using ll = long long;"), 1)
        self.assertLess(source.index("struct Ops"), source.index("int main()"))
        self.assertIn("inserted: bit", result.stdout)

    def test_accepts_uppercase_name(self) -> None:
        result = self.run_lib("UF")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertLess(source.index("struct UnionFind"), source.index("int main()"))
        self.assertIn("inserted: uf", result.stdout)

    def test_rejects_hyphen_prefixed_library_name(self) -> None:
        result = self.run_lib("-uf")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("must not start with '-'", result.stderr)
        self.assertNotIn("struct UnionFind", source)

    def test_rejects_double_hyphen_library_name(self) -> None:
        result = self.run_lib("--bit")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("must not start with '-'", result.stderr)
        self.assertNotIn("FenwickTree", source)

    def test_does_not_insert_same_library_twice(self) -> None:
        first = self.run_lib("seg")
        second = self.run_lib("seg")

        source = self.main_cpp.read_text(encoding="utf-8")
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertEqual(source.count("struct SegTree"), 1)
        self.assertIn("already inserted: seg", second.stdout)

    def test_library_insert_uses_in_place_write(self) -> None:
        source = (ROOT / "scripts" / "library_insert" / "library_insert.py").read_text(encoding="utf-8")

        self.assertIn('path.open("r+", encoding="utf-8")', source)
        self.assertNotIn("main_cpp.write_text", source)

    def test_list_shows_available_libraries(self) -> None:
        result = self.run_lib("--list")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("seg", result.stdout)
        self.assertIn("uf", result.stdout)


class NewWorkCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        shutil.copytree(ROOT / "bin", self.root / "bin")
        shutil.copytree(ROOT / "scripts" / "new_work", self.root / "scripts" / "new_work")
        shutil.copytree(ROOT / "template", self.root / "template")

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def run_nw(self, *args: str) -> subprocess.CompletedProcess[str]:
        env = {**os.environ, "ICPC_KIT": str(self.root)}
        return subprocess.run(
            [str(self.root / "bin" / "nw"), *args],
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_nw_uses_python_implementation(self) -> None:
        wrapper = (ROOT / "bin" / "nw").read_text(encoding="utf-8")

        self.assertIn("scripts/new_work/new_work.py", wrapper)
        self.assertTrue((ROOT / "scripts" / "new_work" / "new_work.py").is_file())

    def test_creates_next_work_with_all_problem_directories(self) -> None:
        for index in range(1, 4):
            (self.root / "ICPC" / f"work{index}").mkdir(parents=True)

        result = self.run_nw("--template", "2")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("template/2", result.stdout)
        for problem in "ABCDEFGHI":
            problem_dir = self.root / "ICPC" / "work4" / problem
            self.assertTrue((problem_dir / "main.cpp").is_file())
            self.assertTrue((problem_dir / "main.py").is_file())
            self.assertTrue((problem_dir / "test" / "sample-1.in").is_file())
            self.assertTrue((problem_dir / "randomTest" / "gen.cpp").is_file())
            self.assertTrue((problem_dir / "secret").is_dir())
            self.assertEqual(list((problem_dir / "secret").iterdir()), [])

    def test_default_template_contains_shared_directories(self) -> None:
        self.assertTrue((ROOT / "template" / "default" / "test" / "sample-1.in").is_file())
        self.assertTrue((ROOT / "template" / "default" / "randomTest" / "gen.cpp").is_file())
        self.assertTrue((ROOT / "template" / "default" / "randomTest" / "naive.cpp").is_file())

    def test_missing_test_directories_fall_back_to_default_template(self) -> None:
        (self.root / "template" / "default" / "randomTest" / "gen.cpp").write_text("default random\n", encoding="utf-8")
        (self.root / "template" / "mikan").mkdir()
        shutil.copy2(self.root / "template" / "2" / "main.cpp", self.root / "template" / "mikan" / "main.cpp")
        shutil.copy2(self.root / "template" / "2" / "main.py", self.root / "template" / "mikan" / "main.py")

        result = self.run_nw("--template", "mikan", "--name", "mikan")

        problem_dir = self.root / "ICPC" / "mikan" / "A"
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((problem_dir / "randomTest" / "gen.cpp").read_text(encoding="utf-8"), "default random\n")
        self.assertTrue((problem_dir / "test" / "sample-1.in").is_file())

    def test_template_specific_random_test_directory_is_used_when_present(self) -> None:
        template_dir = self.root / "template" / "mikan"
        random_test_dir = template_dir / "randomTest"
        template_dir.mkdir()
        random_test_dir.mkdir()
        shutil.copy2(self.root / "template" / "2" / "main.cpp", template_dir / "main.cpp")
        shutil.copy2(self.root / "template" / "2" / "main.py", template_dir / "main.py")
        (random_test_dir / "gen.cpp").write_text("mikan random\n", encoding="utf-8")

        result = self.run_nw("--template", "mikan", "--name", "mikan")

        problem_dir = self.root / "ICPC" / "mikan" / "A"
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((problem_dir / "randomTest" / "gen.cpp").read_text(encoding="utf-8"), "mikan random\n")
        self.assertTrue((problem_dir / "test" / "sample-1.in").is_file())

    def test_creates_named_work_directory(self) -> None:
        result = self.run_nw("--name", "2026")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("created", result.stdout)
        for problem in "ABCDEFGHI":
            self.assertTrue((self.root / "ICPC" / "2026" / problem / "main.cpp").is_file())

    def test_invalid_template_falls_back_to_default_template(self) -> None:
        result = self.run_nw("--template", "abc")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("template/default", result.stdout)

    def test_positional_template_argument_is_rejected(self) -> None:
        result = self.run_nw("2")

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("unknown argument", result.stderr)

    def test_invalid_name_is_rejected(self) -> None:
        result = self.run_nw("--name", "../contest")

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("invalid name", result.stderr)


if __name__ == "__main__":
    unittest.main()
