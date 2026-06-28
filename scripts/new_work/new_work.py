from __future__ import annotations

import os
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Tuple


PROBLEMS = "ABCDEFGHI"
USAGE = "usage: nw [-n NAME|--name NAME] [-t TEMPLATE|--template TEMPLATE]"


class NewWorkError(Exception):
    pass


@dataclass(frozen=True)
class Options:
    template_id: str
    work_name: Optional[str]


def project_root() -> Path:
    if os.environ.get("ICPC_KIT"):
        return Path(os.environ["ICPC_KIT"])
    return Path(__file__).resolve().parents[2]


def parse_args(argv: List[str]) -> Options:
    template_id = "default"
    work_name: Optional[str] = None
    index = 0

    while index < len(argv):
        arg = argv[index]
        if arg in ("-n", "--name"):
            index += 1
            if index >= len(argv) or argv[index] == "":
                raise NewWorkError("--name requires a value")
            work_name = argv[index]
        elif arg in ("-t", "--template"):
            index += 1
            if index >= len(argv) or argv[index] == "":
                raise NewWorkError("--template requires a value")
            template_id = argv[index]
        elif arg in ("-h", "--help"):
            print(USAGE)
            raise SystemExit(0)
        else:
            raise NewWorkError(f"unknown argument: {arg}")
        index += 1

    return Options(template_id=template_id, work_name=work_name)


def template_has_required_files(template_dir: Path) -> bool:
    return (
        template_dir.is_dir()
        and (template_dir / "main.cpp").is_file()
        and (template_dir / "main.py").is_file()
    )


def resolve_template(root: Path, template_id: str) -> Tuple[str, Path]:
    if template_id in ("", ".", "..") or "/" in template_id:
        template_id = "default"

    template_dir = root / "template" / template_id
    if not template_has_required_files(template_dir):
        template_id = "default"
        template_dir = root / "template" / "default"

    if not template_has_required_files(template_dir):
        raise NewWorkError("template/default is missing")

    return template_id, template_dir


def resolve_template_directory(root: Path, template_dir: Path, dirname: str) -> Path:
    candidate = template_dir / dirname
    if candidate.is_dir():
        return candidate

    default_candidate = root / "template" / "default" / dirname
    if default_candidate.is_dir():
        return default_candidate

    raise NewWorkError(f"template/default/{dirname} is missing")


def validate_work_name(work_name: str) -> None:
    if work_name in (".", "..") or "/" in work_name or work_name.startswith("-"):
        raise NewWorkError(f"invalid name: {work_name}")


def next_work_dir(workspace_root: Path) -> Path:
    max_work = 0
    for path in workspace_root.glob("work*"):
        if not path.is_dir():
            continue
        suffix = path.name[4:]
        if suffix.isdigit():
            max_work = max(max_work, int(suffix))
    return workspace_root / f"work{max_work + 1}"


def create_work(root: Path, options: Options) -> Tuple[Path, str]:
    template_id, template_dir = resolve_template(root, options.template_id)
    test_dir = resolve_template_directory(root, template_dir, "test")
    random_test_dir = resolve_template_directory(root, template_dir, "randomTest")
    workspace_root = root / "ICPC"

    if options.work_name is not None:
        validate_work_name(options.work_name)
        work_dir = workspace_root / options.work_name
    else:
        work_dir = next_work_dir(workspace_root)

    if work_dir.exists():
        raise NewWorkError(f"{work_dir} already exists")

    work_dir.mkdir(parents=True)
    for problem in PROBLEMS:
        problem_dir = work_dir / problem
        problem_dir.mkdir()
        shutil.copy2(template_dir / "main.cpp", problem_dir / "main.cpp")
        shutil.copy2(template_dir / "main.py", problem_dir / "main.py")
        shutil.copytree(test_dir, problem_dir / "test")
        shutil.copytree(random_test_dir, problem_dir / "randomTest")

    return work_dir, template_id


def main(argv: List[str]) -> int:
    try:
        options = parse_args(argv)
        work_dir, template_id = create_work(project_root(), options)
    except NewWorkError as error:
        print(f"nw: {error}", file=sys.stderr)
        if "requires a value" in str(error) or "unknown argument" in str(error):
            print(USAGE, file=sys.stderr)
        return 1

    print(f"created {work_dir} using template/{template_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
