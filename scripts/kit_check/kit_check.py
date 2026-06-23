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

        for command in ["python3", "g++", "oj", "nw", "rt", "ace"]:
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


class NewWorkCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        shutil.copytree(ROOT / "bin", self.root / "bin")
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

    def test_creates_next_work_with_all_problem_directories(self) -> None:
        for index in range(1, 4):
            (self.root / f"work{index}").mkdir()

        result = self.run_nw("2")

        self.assertEqual(result.returncode, 0, result.stderr)
        for problem in "ABCDEFGHI":
            problem_dir = self.root / "work4" / problem
            self.assertTrue((problem_dir / "main.cpp").is_file())
            self.assertTrue((problem_dir / "main.py").is_file())
            self.assertTrue((problem_dir / "test" / "sample-1.in").is_file())
            self.assertTrue((problem_dir / "randomTest" / "gen.cpp").is_file())

    def test_invalid_template_falls_back_to_template_1(self) -> None:
        result = self.run_nw("abc")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("template/1", result.stdout)


if __name__ == "__main__":
    unittest.main()
