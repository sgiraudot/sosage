import os
import sys
import subprocess
import time

# Local config
data_folder = "/home/gee/art/superflu-pnc-data"
linuxdeploy = "/home/gee/download/linuxdeploy-x86_64.AppImage"
windows_sdl_folder = "/home/gee/local/i686-w64-mingw32/include/SDL2"
libyaml_souce_path = "/home/gee/local/sources/libyaml-master"

gamename = "superflu-riteurnz"
version = "v0.2.0-alpha2"

linux_buildir = "TMP_build_linux"
appimg_buildir = "TMP_build_appimg"
windows_buildir = "TMP_build_windows"
emscripten_buildir = "TMP_build_emscripten"
output_dir = "distrib-" + version
appname = gamename + "-" + version

verbose = (len(sys.argv) == 2 and sys.argv[1] == "-v")

def run_cmd(cmd):
    if verbose:
        subprocess.run(' '.join(cmd), shell=True, check=True)
    else:
        subprocess.run(' '.join(cmd), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True, check=True)

run_cmd(["rm", "-rf", output_dir])
run_cmd(["mkdir", output_dir])

print("(1/5) BUILD LINUX DEB/RPM")
try:
    begin = time.perf_counter()
    run_cmd(["rm", "-rf", linux_buildir])
    run_cmd(["mkdir", linux_buildir])
    os.chdir(linux_buildir)
    run_cmd(["cmake", "-DCMAKE_BUILD_TYPE=Release",
             "-DSOSAGE_DATA_FOLDER=" + data_folder, ".."])
    run_cmd(["make", "-j", "6"])
    run_cmd(["cpack"])
#    run_cmd(["cp", "*.deb", "*.rpm", "../" + output_dir])
    run_cmd(["cp", "*.deb", "../" + output_dir + "/" + appname + ".deb"])
    run_cmd(["cp", "*.rpm", "../" + output_dir + "/" + appname + ".rpm"])
    os.chdir("..")
    run_cmd(["rm", "-rf", linux_buildir])
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s")
except:
    run_cmd(["rm", "-rf", linux_buildir])
    print("  -> ERROR, something went wrong")

print("(2/5) BUILD LINUX APPIMAGE")
try:
    begin = time.perf_counter()
    run_cmd(["rm", "-rf", appimg_buildir])
    run_cmd(["mkdir", appimg_buildir])
    os.chdir(appimg_buildir)
    run_cmd(["mkdir", "appimg"])
    os.chdir("appimg")
    run_cmd(["cp", "../../platform/appimage/create_appimage.sh", "."])
    run_cmd(["bash", "create_appimage.sh", linuxdeploy, data_folder])
    run_cmd(["cp", "*.AppImage", "../../" + output_dir + "/" + appname + ".AppImage"])
    os.chdir("../..")
    run_cmd(["rm", "-rf", appimg_buildir])
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s")
except:
    run_cmd(["rm", "-rf", appimg_buildir])
    print("  -> ERROR, something went wrong")

print("(3/5) BUILD ANDROID")
try:
    begin = time.perf_counter()
    os.chdir("platform/android")
    run_cmd(["./gradlew", "clean"])
    run_cmd(["./gradlew", "assemble"])
    run_cmd(["cp", "app/build/outputs/apk/debug/app-debug.apk", "../../" + output_dir + "/" + appname + ".apk"])
    os.chdir("../..")
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s")
except:
    print("  -> ERROR, something went wrong")

print("(4/5) BUILD WINDOWS")
try:
    begin = time.perf_counter()
    run_cmd(["rm", "-rf", windows_buildir])
    run_cmd(["mkdir", windows_buildir])
    os.chdir(windows_buildir)
    run_cmd(["cmake", "-DCMAKE_BUILD_TYPE=Release",
             "-DSOSAGE_DATA_FOLDER=" + data_folder,
             "-DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake",
             "-DSDL2_INCLUDE_DIR=" + windows_sdl_folder,
             ".."])
    run_cmd(["make", "-j", "6"])
    run_cmd(["cpack"])
    run_cmd(["cp", "*-win32.exe", "../" + output_dir + "/" + appname + ".exe"])
    os.chdir("..")
    run_cmd(["rm", "-rf", windows_buildir])
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s")
except:
    run_cmd(["rm", "-rf", windows_buildir])
    print("  -> ERROR, something went wrong")

print("(5/5) BUILD EMSCRIPTEN")
try:
    begin = time.perf_counter()
    run_cmd(["rm", "-rf", emscripten_buildir])
    run_cmd(["mkdir", emscripten_buildir])
    os.chdir(emscripten_buildir)
    run_cmd(["mkdir", "emscripten"])
    os.chdir("emscripten")
    run_cmd(["cp", "../../platform/emscripten/CMakeLists.txt", "."])
    run_cmd(["ln", "-s", libyaml_souce_path, "libyaml"])
    run_cmd(["mkdir", "build"])
    os.chdir("build")
    run_cmd(["ln", "-s", data_folder, "data"])
    run_cmd(["emcmake", "cmake", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", ".."])
    run_cmd(["make", "-j", "6"])
    run_cmd(["mkdir", "../../../" + output_dir + "/" + appname + "-web/"])
    run_cmd(["cp", "-r", "data", "../../../" + output_dir + "/" + appname + "-web/"])
    run_cmd(["cp", "*.html", "*.js", "*.wasm", "../../../" + output_dir + "/" + appname + "-web/"])
    os.chdir("../../../")
    run_cmd(["rm", "-rf", emscripten_buildir])
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s")
except:
    run_cmd(["rm", "-rf", emscripten_buildir])
    print("  -> ERROR, something went wrong")