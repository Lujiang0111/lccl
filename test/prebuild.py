import os
import shutil


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
        os.makedirs(os.path.join(dep_base, "include", lib_name), exist_ok=True)
        copy_dir(
            os.path.join(lib_path, "include"),
            os.path.join(dep_base, "include", lib_name),
        )
        copy_dir(os.path.join(lib_path, "bin"), os.path.join(dep_base, "lib"))
    elif os.path.exists(trd_lib_path):
        os.makedirs(os.path.join(dep_base, "include", lib_name), exist_ok=True)
        copy_dir(
            os.path.join(trd_lib_path, "include"),
            os.path.join(dep_base, "include", lib_name),
        )
        copy_dir(os.path.join(trd_lib_path, "lib"), os.path.join(dep_base, "lib"))
    else:
        print(
            f"Please put https://github.com/Lujiang0111/{lib_name} source in {lib_path} or lib in {trd_lib_path}"
        )
        exit(0)


if __name__ == "__main__":
    self_path = os.path.split(os.path.realpath(__file__))[0]
    root_base = os.path.join(self_path, "..", "..")
    bin_base = os.path.join(self_path, "bin")
    trd_base = os.path.join(self_path, "3rd")
    dep_base = os.path.join(self_path, "dep")

    remove_dir(dep_base)

    os.makedirs(os.path.join(dep_base, "include"), exist_ok=True)
    os.makedirs(os.path.join(dep_base, "lib"), exist_ok=True)

    dep_libs = ["lccl"]
    for dep_lib in dep_libs:
        copy_dep(dep_lib)
