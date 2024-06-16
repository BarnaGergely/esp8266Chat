# https://community.platformio.org/t/run-script-before-after-image-upload/653
import shutil
Import("env")

used_theme = "astro"

print("script start")

def before_buildfs(source, target, env):
    # copy files from "astro" to "data" folder
    # overwrite files "data" folder with files in "astro" folder
    shutil.rmtree("data")
    shutil.copytree("themes/"+used_theme, "data")
    print("Copied "+used_theme+" to upload (data) folder")

env.AddPreAction("buildfs", before_buildfs)