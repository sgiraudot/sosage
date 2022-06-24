import os
import sys
import subprocess
import time
import yaml

if len(sys.argv) != 2:
    print("Usage: " + sys.argv[0] + "[build.yaml]")

yaml_file = open(sys.argv[1], 'r')
data = yaml.safe_load(yaml_file)
    
gamename = ''
for line in open(data["folder"] + '/config.cmake').readlines():
    if 'SOSAGE_EXE_NAME "' in line:
        gamename = line.split('SOSAGE_EXE_NAME "')[1].split('"')[0]
        break
if gamename == '':
    print("Couldn't find SOSAGE_EXE_NAME in " + data["folder"] + "/config.cmake")
    exit()

version = ''
for line in open(data["folder"] + '/data/data/init.yaml').readlines():
    if 'version: ' in line:
        version = line.split('version: ')[1][:-1]
        break
if version == '':
    print("Couldn't find version in " + data["folder"] + "/data/data/init.yaml")
    exit()

id = gamename + '-v' + version
print("## BUILDING " + id + "\n")

raw_data_folder = data["folder"]
data_folder = data["folder"] + "/compressed_data"
data_dir = "TMP_data"
linux_buildir = "TMP_build_linux"
appimg_buildir = "TMP_build_appimg"
android_buildir = "TMP_build_android"
mac_buildir = "TMP_build_mac"
windows_buildir = "TMP_build_windows"
emscripten_buildir = "TMP_build_emscripten"
output_dir = data["output"] + "/release-v" + version
steam_dir = output_dir + "/steam"
appname = gamename + "-" + version
cmake_cmd = "cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_CFG_DISPLAY_DEBUG_INFO:BOOL=" + str(data["debug"]) + " -DSOSAGE_DATA_FOLDER=" + data_folder

def todo(key):
    return len(packages_to_do) == 0 or key in packages_to_do

def run_cmd(cmd):
    if data["simulate"]:
        print(cmd)
    else:
        try:
            if data["verbose"]:
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
    if data["simulate"]:
        print("cd " + folder)
    else:
        os.chdir(folder)

all_begin = time.perf_counter()

print("### INIT")
begin = time.perf_counter()
if not os.path.isdir(output_dir):
    run_cmd("mkdir " + output_dir)
    run_cmd("mkdir " + steam_dir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### COMPILING DATA PACKAGER")
begin = time.perf_counter()
run_cmd("rm -rf " + linux_buildir)
run_cmd("mkdir " + linux_buildir)
chdir(linux_buildir)
run_cmd("cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_COMPILE_SCAP:BOOL=True -DSOSAGE_DATA_FOLDER=" + raw_data_folder + " ..")
run_cmd("make -j " + str(data["threads"]) + " SCAP")
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

print("### PACKAGING DATA")
begin = time.perf_counter()
run_cmd("rm -rf " + data_folder)
run_cmd("mkdir " + data_folder)
run_cmd("./SCAP " + raw_data_folder + " " + data_folder)
chdir("..")
run_cmd("rm -rf " + linux_buildir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

if data["data"]:
    print("### BUILDING DATA ZIP")
    begin = time.perf_counter()
    run_cmd("rm -rf " + data_dir)
    run_cmd("mkdir " + data_dir)
    chdir(data_dir)
    copy_data_dir = appname + "-data"
    run_cmd("cp -r " + data_folder + " " + copy_data_dir)
    run_cmd("zip -r " + output_dir + "/" + appname + "-data.zip " + copy_data_dir)
    chdir("..")
    run_cmd("rm -rf " + data_dir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["linux"]:
    print("### BUILDING LINUX DEB/RPM")
    begin = time.perf_counter()
    run_cmd("rm -rf " + linux_buildir)
    run_cmd("mkdir " + linux_buildir)
    chdir(linux_buildir)
    run_cmd(cmake_cmd + " ..")
    run_cmd("make -j " + str(data["threads"]))
    run_cmd("cpack")
    run_cmd("cp *.deb " + output_dir + "/" + appname + "-gnunux.deb")
    run_cmd("cp *.rpm " + output_dir + "/" + appname + "-gnunux.rpm")
    chdir("..")
    run_cmd("rm -rf " + linux_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["steam"]:
    print("### BUILDING LINUX/STEAMOS")
    begin = time.perf_counter()
    run_cmd("rm -rf " + linux_buildir)
    run_cmd("mkdir " + linux_buildir)
    chdir(linux_buildir)
    run_cmd("mkdir install")
    run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "CC=/usr/bin/gcc-9 CXX=/usr/bin/g++-9 ' + cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/ -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/ -DCMAKE_INSTALL_PREFIX=./install .."')
    run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make -j ' + str(data["threads"]) + '"')
    run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make install"')
    run_cmd("cp -r install " + steam_dir + "/linux")
    chdir("..")
    run_cmd("rm -rf " + linux_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["appimage"]:
    print("### BUILDING LINUX APPIMAGE")
    begin = time.perf_counter()
    run_cmd("rm -rf " + appimg_buildir)
    run_cmd("mkdir " + appimg_buildir)
    chdir(appimg_buildir)
    run_cmd(cmake_cmd + " -DCMAKE_INSTALL_PREFIX=/usr ..")
    run_cmd("make -j " + str(data["threads"]))
    run_cmd("make install DESTDIR=install_dir")
    run_cmd(data["linuxdeploy"] + " --appdir install_dir -e install_dir/usr/bin/" + gamename + " --output appimage")
    run_cmd("cp *.AppImage " + output_dir + "/" + appname + "-gnunux.AppImage")
    chdir("..")
    run_cmd("rm -rf " + appimg_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["mac"]:
    print("### BUILDING MAC")
    begin = time.perf_counter()
    run_cmd("rm -rf " + mac_buildir)
    run_cmd("mkdir " + mac_buildir)
    chdir(mac_buildir)
    run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-osxcross.cmake"
            + " -DSDL2_INCLUDE_DIR=" + data["mac_sdl_folder"]
            + " -DCMAKE_INSTALL_PREFIX=./install ..")
    run_cmd("make -j " + str(data["threads"]))
    run_cmd("make install")
    run_cmd("python3 ../tools/fix_mac_lib_paths.py install/" + gamename + ".app/Contents/libs/*.dylib")
    run_cmd("python3 ../tools/fix_mac_lib_paths.py install/" + gamename + ".app/Contents/MacOS/" + gamename)
    run_cmd("cp -r install " + steam_dir + "/mac")
    run_cmd("genisoimage -V " + gamename + ".app -D -R -apple -no-pad -o " + gamename + ".dmg install")
    run_cmd("cp " + gamename + ".dmg " + output_dir + "/" + appname + "-mac.dmg")
    chdir("..")
    run_cmd("rm -rf " + mac_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["windows"]:
    print("### BUILDING WINDOWS")
    begin = time.perf_counter()
    run_cmd("rm -rf " + windows_buildir)
    run_cmd("mkdir " + windows_buildir)
    chdir(windows_buildir)
    run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake "
            + "-DSDL2_INCLUDE_DIR=" + data["windows_sdl_folder"] + " ..")
    run_cmd("make -j " + str(data["threads"]) + "")
    run_cmd("cpack")
    run_cmd("cp *-win32.exe " + output_dir + "/" + appname + "-windows.exe")
    run_cmd("cmake -DCMAKE_INSTALL_PREFIX=./install .")
    run_cmd("mkdir install")
    run_cmd("make -j " + str(data["threads"]))
    run_cmd("make install")
    run_cmd("cp -r install " + steam_dir + "/windows")
    chdir("..")
    run_cmd("rm -rf " + windows_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["androidapk"] or data["androidaab"]:
    print("### BUILDING ANDROID")
    begin = time.perf_counter()
    run_cmd("rm -rf " + android_buildir)
    run_cmd("mkdir " + android_buildir)
    chdir(android_buildir)
    run_cmd(cmake_cmd + " -DSOSAGE_CONFIG_ANDROID:BOOL=True"
            + " -DSDL2_SOURCE_PATH:PATH=" + data["sdl2_source_path"]
            + " -DSDL2_IMAGE_SOURCE_PATH:PATH=" + data["sdl2_image_source_path"]
            + " -DSDL2_MIXER_SOURCE_PATH:PATH=" + data["sdl2_mixer_source_path"]
            + " -DSDL2_TTF_SOURCE_PATH:PATH=" + data["sdl2_ttf_source_path"]
            + " -DYAML_SOURCE_PATH:PATH=" + data["libyaml_source_path"]
            + " -DLZ4_SOURCE_PATH:PATH=" + data["lz4_source_path"] + " ..")
    chdir("android")
    if data["androidapk"]:
        run_cmd("./gradlew assembleDebug --parallel --max-workers=" + str(data["threads"]))
        run_cmd("cp app/build/outputs/apk/debug/app-debug.apk " + output_dir + "/" + appname + "-android.apk")
    if data["androidaab"]:
        run_cmd("./gradlew bundleRelease --parallel --max-workers=" + str(data["threads"]))
        run_cmd("cp app/build/outputs/bundle/release/app.aab " + output_dir + "/" + appname + "-android.aab")
    chdir("../..")
    run_cmd("rm -rf " + android_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["browser"]:
    print("### BUILDING EMSCRIPTEN")
    begin = time.perf_counter()
    run_cmd("rm -rf " + emscripten_buildir)
    run_cmd("mkdir " + emscripten_buildir)
    chdir(emscripten_buildir)
    run_cmd("emcmake cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_DATA_FOLDER=" + data_folder
            + " -DYAML_SOURCE_PATH:PATH=" + data["libyaml_source_path"]
            + " -DLZ4_SOURCE_PATH:PATH=" + data["lz4_source_path"] + " ..")
    run_cmd("make -j " + str(data["threads"]))
    run_cmd("rm -rf " + output_dir + "/" + appname + "-web/")
    run_cmd("mkdir " + output_dir + "/" + appname + "-web/")
    run_cmd("cp -r data " + output_dir + "/" + appname + "-web/")
    run_cmd("cp *.data *.js *.wasm " + output_dir + "/" + appname + "-web/")
    chdir("..")
    run_cmd("rm -rf " + emscripten_buildir)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

all_end = time.perf_counter()
print(" ==> all done in " + str(int(all_end - all_begin)) + "s")