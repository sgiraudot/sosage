import os
import sys
import subprocess
import time

# Local config
raw_data_folder = "/home/gee/art/superflu_riteurnz"
data_folder = "/home/gee/art/superflu_riteurnz/release"
linuxdeploy = "/home/gee/download/linuxdeploy-x86_64.AppImage"
windows_sdl_folder = "/home/gee/local/i686-w64-mingw32/include/SDL2"

sdl2_source_path = "/home/gee/local/sources/SDL2-2.0.12"
sdl2_image_source_path = "/home/gee/local/sources/SDL2_image-2.0.5"
sdl2_mixer_source_path = "/home/gee/local/sources/SDL2_mixer-2.0.4"
sdl2_ttf_source_path = "/home/gee/local/sources/SDL2_ttf-2.0.15"
libyaml_source_path = "/home/gee/local/sources/libyaml-master"
lz4_source_path = "/home/gee/local/sources/lz4"

gamename = "superflu-riteurnz"
version = "v1.1.0-modern-ui"

data_dir = "TMP_data"
linux_buildir = "TMP_build_linux"
appimg_buildir = "TMP_build_appimg"
windows_buildir = "TMP_build_windows"
emscripten_buildir = "TMP_build_emscripten"
output_dir = "distrib-" + version
appname = gamename + "-" + version

verbose = (len(sys.argv) == 2 and sys.argv[1] == "-v")
simulate = (len(sys.argv) == 2 and sys.argv[1] == "-s")

def run_cmd(cmd):
    if simulate:
        print(cmd)
    else:
        try:
            if verbose:
                out = subprocess.run(cmd, shell=True, check=True)
            else:
                out = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True, check=True)
        except:
            print("The following command raised an exception:")
            print("  " + cmd)
            exit()
        else:
            if out.returncode != 0:
                print("The following command returned error code " + str(out.returncode))
                print("  " + cmd)
                exit()

def chdir(folder):
    if simulate:
        print("cd " + folder)
    else:
        os.chdir(folder)
    
all_begin = time.perf_counter()

print("### CLEANING")
begin = time.perf_counter()
run_cmd("rm -rf " + output_dir)
run_cmd("mkdir " + output_dir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### COMPILING DATA PACKAGER")
begin = time.perf_counter()
run_cmd("rm -rf " + linux_buildir)
run_cmd("mkdir " + linux_buildir)
chdir(linux_buildir)
run_cmd("cmake -DCMAKE_BUILD_TYPE=Release -DSOSAGE_COMPILE_SCAP:BOOL=True -DSOSAGE_DATA_FOLDER=" + raw_data_folder + " ..")
run_cmd("make -j 6 SCAP")
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### PACKAGING DATA")
begin = time.perf_counter()
run_cmd("rm -rf " + data_folder + "/*")
run_cmd("./SCAP " + raw_data_folder + " " + data_folder)
chdir("..")
run_cmd("rm -rf " + linux_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING DATA ZIP")
begin = time.perf_counter()
run_cmd("rm -rf " + data_dir)
run_cmd("mkdir " + data_dir)
chdir(data_dir)
copy_data_dir = appname + "-data"
run_cmd("cp -r " + data_folder + " " + copy_data_dir)
run_cmd("zip -r ../" + output_dir + "/" + appname + "-data.zip " + copy_data_dir)
chdir("..")
run_cmd("rm -rf " + data_dir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING LINUX DEB/RPM")
begin = time.perf_counter()
run_cmd("rm -rf " + linux_buildir)
run_cmd("mkdir " + linux_buildir)
chdir(linux_buildir)
run_cmd("cmake -DCMAKE_BUILD_TYPE=Release -DSOSAGE_CONFIG_ANDROID:BOOL=True -DSOSAGE_DATA_FOLDER=" + data_folder
        + " -DSDL2_SOURCE_PATH:PATH=" + sdl2_source_path
        + " -DSDL2_IMAGE_SOURCE_PATH:PATH=" + sdl2_image_source_path
        + " -DSDL2_MIXER_SOURCE_PATH:PATH=" + sdl2_mixer_source_path
        + " -DSDL2_TTF_SOURCE_PATH:PATH=" + sdl2_ttf_source_path
        + " -DYAML_SOURCE_PATH:PATH=" + libyaml_source_path
        + " -DLZ4_SOURCE_PATH:PATH=" + lz4_source_path + " ..")
run_cmd("make -j 6")
run_cmd("cpack")
run_cmd("cp *.deb ../" + output_dir + "/" + appname + "-gnunux.deb")
run_cmd("cp *.rpm ../" + output_dir + "/" + appname + "-gnunux.rpm")
chdir("..")
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING LINUX APPIMAGE")
begin = time.perf_counter()
run_cmd("rm -rf " + appimg_buildir)
run_cmd("mkdir " + appimg_buildir)
chdir(appimg_buildir)
run_cmd("mkdir appimg")
chdir("appimg")
run_cmd("cp ../../platform/appimage/create_appimage.sh .")
run_cmd("bash create_appimage.sh " + linuxdeploy + " " + data_folder)
run_cmd("cp *.AppImage ../../" + output_dir + "/" + appname + "-gnunux.AppImage")
chdir("../..")
run_cmd("rm -rf " + appimg_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING ANDROID")
begin = time.perf_counter()
chdir(linux_buildir + "/android")
run_cmd("./gradlew assembleDebug --parallel --max-workers=6")
run_cmd("cp app/build/outputs/apk/debug/app-debug.apk ../../" + output_dir + "/" + appname + "-android.apk")
chdir("../..")
run_cmd("rm -rf " + linux_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING WINDOWS")
begin = time.perf_counter()
run_cmd("rm -rf " + windows_buildir)
run_cmd("mkdir " + windows_buildir)
chdir(windows_buildir)
run_cmd("cmake -DCMAKE_BUILD_TYPE=Release -DSOSAGE_DATA_FOLDER=" + data_folder
        + " -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake "
        + "-DSDL2_INCLUDE_DIR=" + windows_sdl_folder + " ..")
run_cmd("make -j 6")
run_cmd("cpack")
run_cmd("cp *-win32.exe ../" + output_dir + "/" + appname + "-windows.exe")
chdir("..")
run_cmd("rm -rf " + windows_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### BUILDING EMSCRIPTEN")
begin = time.perf_counter()
run_cmd("rm -rf " + emscripten_buildir)
run_cmd("mkdir " + emscripten_buildir)
chdir(emscripten_buildir)
run_cmd("emcmake cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOSAGE_DATA_FOLDER=" + data_folder
        + " -DYAML_SOURCE_PATH:PATH=" + libyaml_source_path
        + " -DLZ4_SOURCE_PATH:PATH=" + lz4_source_path + " ..")
run_cmd("make -j 6")
run_cmd("mkdir ../" + output_dir + "/" + appname + "-web/")
run_cmd("cp -r data ../" + output_dir + "/" + appname + "-web/")
run_cmd("cp *.data *.js *.wasm ../" + output_dir + "/" + appname + "-web/")
chdir("..")
run_cmd("rm -rf " + emscripten_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

all_end = time.perf_counter()
print(" ==> all done in " + str(int(all_end - all_begin)) + "s")
