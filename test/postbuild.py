import os
import shutil
import sys


def remove_dir(dir) -> None:
    if not os.path.exists(dir):
        return

    if os.path.isdir(dir):
        shutil.rmtree(dir)
    else:
        os.remove(dir)


def copy_dir(src_dir, dst_dir) -> None:
    if not os.path.isdir(src_dir):
        return

    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)

    for item in os.listdir(src_dir):
        src_item = os.path.join(src_dir, item)
        dst_item = os.path.join(dst_dir, item)
        if os.path.isdir(src_item):
            copy_dir(src_item, dst_item)
        else:
            shutil.copy2(src_item, dst_item, follow_symlinks=False)


def copy_dep(lib_name) -> None:
    lib_path = os.path.join(root_base, lib_name)
    trd_lib_path = os.path.join(trd_base, lib_name)

    if os.path.exists(lib_path):
        copy_dir(os.path.join(lib_path, "bin"), dst_lib_dir)

        lib_third_path = os.path.join(lib_path, "3rd")
        if os.path.exists(lib_third_path):
            for d in os.listdir(lib_third_path):
                third_lib_dir = os.path.join(lib_third_path, d)
                if os.path.isdir(third_lib_dir):
                    copy_dir(os.path.join(third_lib_dir, "lib"), dst_lib_dir)

    elif os.path.exists(trd_lib_path):
        copy_dir(os.path.join(trd_lib_path, "lib"), dst_lib_dir)
    else:
        print(
            f"Please put https://github.com/Lujiang0111/{lib_name} source in {lib_path} or lib in {trd_lib_path}"
        )
        exit(0)


if __name__ == "__main__":
    param_cnt = len(sys.argv) - 1
    if param_cnt < 1:
        raise SystemExit("param cnt={} too less".format(param_cnt))
    bin_base = sys.argv[1]

    self_path = os.path.split(os.path.realpath(__file__))[0]
    root_base = os.path.join(self_path, "..", "..")
    trd_base = os.path.join(self_path, "3rd")

    if sys.platform.startswith("win"):
        dst_lib_dir = os.path.join(bin_base)
    elif sys.platform.startswith("linux"):
        dst_lib_dir = os.path.join(bin_base, "lib")
        remove_dir(dst_lib_dir)
        os.makedirs(dst_lib_dir, exist_ok=True)

    dep_libs = ["lccl"]
    for dep_lib in dep_libs:
        copy_dep(dep_lib)
