Import("env")
import os
import json
import shutil
import re

def extract_fw_version(env_):
    flags = env_.GetProjectOption("build_flags") or []
    if isinstance(flags, str):
        flags = [flags]
    joined = " ".join(flags)
    m = re.search(r'FW_VERSION=\\"([^"]+)\\"', joined)
    return m.group(1) if m else "unknown"

def after_build(source, target, env, **kwargs):
    project_dir = env["PROJECT_DIR"]
    fw_version = extract_fw_version(env)

    # target[0] should be the .bin
    built_path = str(target[0])
    if not os.path.exists(built_path):
        print(f"[post_build] Built binary not found at: {built_path}")
        return

    publish_dir = os.path.join(project_dir, "ota_publish")
    os.makedirs(publish_dir, exist_ok=True)

    firmware_dst = os.path.join(publish_dir, "firmware.bin")
    shutil.copyfile(built_path, firmware_dst)

    # Update these ONCE:
    BIN_URL = "https://github.com/0shuvo0/reef-tank-light/raw/refs/heads/main/ota_publish/firmware.bin"

    version_json_path = os.path.join(publish_dir, "version.json")
    with open(version_json_path, "w") as f:
        json.dump({"version": fw_version, "bin": BIN_URL}, f, indent=2)

    print("\n[post_build] ✅ Published OTA files:")
    print(f"  - {firmware_dst}")
    print(f"  - {version_json_path}\n")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)