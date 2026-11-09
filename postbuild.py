import argparse
import shutil
import time
from pathlib import Path


def rm_path(file_name, retries=3) -> bool:
    path = Path(file_name)
    if path.is_symlink() or path.is_file():
        path.unlink()
        return True

    if path.is_dir():
        for i in range(retries):
            try:
                shutil.rmtree(file_name)
                return True
            except OSError:
                if i == retries - 1:
                    return False
                time.sleep(1)
        return False

    return False


def copy_path(src_path, dst_path) -> bool:
    src = Path(src_path)
    dst = Path(dst_path)

    if not src.exists():
        return True

    dst.parent.mkdir(parents=True, exist_ok=True)

    if src.is_dir():
        if dst.exists():
            if dst.is_dir():
                for p in src.iterdir():
                    if not copy_path(p, dst / p.name):
                        return False
                return True
            return False

        shutil.copytree(src, dst)
        return True

    shutil.copy2(src, dst)
    return True


class PostbuildClass:
    __args = None

    __include_dir = None
    __bin_dir = None
    __dst_dir = None

    def main(self) -> None:
        self.parse_args()

        # remove symlinks
        for p in self.__bin_dir.iterdir():
            if p.is_symlink():
                rm_path(p)

        rm_path(self.__dst_dir)
        copy_path(self.__include_dir, self.__dst_dir / "include")
        copy_path(self.__bin_dir, self.__dst_dir / "lib")

    def parse_args(self) -> None:
        parser = argparse.ArgumentParser(description="arg description")

        parser.add_argument("--os_name", required=True)
        parser.add_argument("--os_arch", required=True)
        parser.add_argument("--bin_dir", required=True)
        parser.add_argument("--include_dir", required=True)
        parser.add_argument("--lib_base_dir", required=True)
        parser.add_argument("--project_name", required=True)
        parser.add_argument("--project_version", required=True)

        self.__args = parser.parse_args()

        self.__include_dir = Path(self.__args.include_dir)
        self.__bin_dir = Path(self.__args.bin_dir)
        self.__dst_dir = (
            Path(self.__args.lib_base_dir)
            / self.__args.project_name
            / self.__args.project_version
            / self.__args.os_name
            / self.__args.os_arch
        )


if __name__ == "__main__":
    h = PostbuildClass()
    h.main()
