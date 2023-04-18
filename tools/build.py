import os
import sys
import subprocess
import time
import yaml
import re

if len(sys.argv) != 2:
    print("Usage: " + sys.argv[0] + "[build.yaml]")

yaml_file = open(sys.argv[1], 'r')
cwd = os.getcwd()
data = yaml.safe_load(yaml_file)

gamename = ''
fullname = ''
for line in open(data["folder"] + '/config.cmake').readlines():
    if 'SOSAGE_EXE_NAME "' in line:
        gamename = line.split('SOSAGE_EXE_NAME "')[1].split('"')[0]
    if 'SOSAGE_NAME "' in line:
        fullname = line.split('SOSAGE_NAME "')[1].split('"')[0]

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
if not data["use_compressed_data"]:
    if data["compress_data"]:
        print("Error: uncoherent use_compressed / compress")
        exit()
    data_folder = data["folder"]
    
data_dir = data["buildfolder"] + "/data"
linux_buildir = data["buildfolder"] + "/linux"
steam_buildir = data["buildfolder"] + "/steamos"
appimg_buildir = data["buildfolder"] + "/appimg"
android_buildir = data["buildfolder"] + "/android"
mac_buildir = data["buildfolder"] + "/mac"
win32_buildir = data["buildfolder"] + "/win32"
win64_buildir = data["buildfolder"] + "/win64"
emscripten_buildir = data["buildfolder"] + "/emscripten"
output_dir = data["output"] + "/release-v" + version
steam_dir = output_dir + "/steam"
appname = gamename + "-" + version
use_schroot = True
configure_only = data["configure_only"]

cmake_cmd = "cmake -DCMAKE_BUILD_TYPE=" + data["build"]
cmake_cmd += " -DSOSAGE_BUILD_TYPE=" + data["buildtype"]
cmake_cmd += " -DSOSAGE_CFG_DISPLAY_DEBUG_INFO:BOOL=" + str(data["debug"])
cmake_cmd += " -DSOSAGE_DATA_FOLDER=" + data_folder
if data["testinput"]:
    cmake_cmd += " -DSOSAGE_CFG_TEST_INPUT:BOOL=True"
    appname += "-testinput"
if data["use_sdl_mixer_ext"]:
    cmake_cmd += " -DSOSAGE_CFG_USE_SDL_MIXER_EXT:BOOL=True"

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
    run_cmd("mkdir -p " + output_dir)
    run_cmd("mkdir -p " + steam_dir)
end = time.perf_counter()
print("  -> done in " + str(int(end - begin)) + "s\n")

if data["compress_data"]:
    print("### COMPILING DATA PACKAGER")
    begin = time.perf_counter()
    run_cmd("rm -rf " + linux_buildir)
    run_cmd("mkdir -p " + linux_buildir)
    chdir(linux_buildir)
    run_cmd("cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_COMPILE_SCAP:BOOL=True -DSOSAGE_DATA_FOLDER=" + raw_data_folder + " " + cwd)
    run_cmd("make -j " + str(data["threads"]) + " SCAP")
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

    print("### PACKAGING DATA")
    begin = time.perf_counter()
    run_cmd("rm -rf " + data_folder)
    run_cmd("mkdir -p " + data_folder)
    run_cmd("./SCAP " + raw_data_folder + " " + data_folder)
    chdir(cwd)
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

if data["data"]:
    try:
        print("### BUILDING DATA ZIP")
        begin = time.perf_counter()
        run_cmd("rm -rf " + output_dir + "/" + appname + "-data.zip")
        run_cmd("rm -rf " + data_dir)
        run_cmd("mkdir -p " + data_dir)
        chdir(data_dir)
        copy_data_dir = appname + "-data"
        run_cmd("rm -rf " + copy_data_dir)
        run_cmd("mkdir -p " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/resources " + copy_data_dir)
        run_cmd("cp -r " + data_folder + "/data " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/config.cmake " + copy_data_dir)
#        run_cmd("cp -r " + data_folder + "/LICENSE.md " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/README.md " + copy_data_dir)
        run_cmd("zip -r " + output_dir + "/" + appname + "-data.zip " + copy_data_dir)
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["linux"]:
    try:
        print("### BUILDING LINUX DEB/RPM")
        begin = time.perf_counter()
        run_cmd("rm -rf " + linux_buildir)
        run_cmd("mkdir -p " + linux_buildir)
        chdir(linux_buildir)

        if use_schroot:
            cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
            cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
            cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_bullseye/lib/libSDL2_mixer_ext.a'
            cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/ ' + cwd
            run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "' + cfg_cmd + '"')
            if not configure_only:
                run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "make -j ' + str(data["threads"]) + '"')
                run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "cpack"')
        else:
            # Use static SDL2 mixer ext for simplicity
            cfg_cmd = cmake_cmd +' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
            cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build/lib/libSDL2_mixer_ext.a'
            run_cmd(cfg_cmd + " " + cwd)
            if not configure_only:
                run_cmd("make -j " + str(data["threads"]))
                run_cmd("cpack")
        if not configure_only:
            run_cmd("cp *.deb " + output_dir + "/" + appname + "-gnunux.deb")
            run_cmd("cp *.rpm " + output_dir + "/" + appname + "-gnunux.rpm")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["linux32"]:
    try:
        print("### BUILDING LINUX 32")
        begin = time.perf_counter()
        run_cmd("rm -rf " + linux_buildir)
        run_cmd("mkdir -p " + linux_buildir)
        chdir(linux_buildir)

        cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
        cfg_cmd += ' -DCMAKE_INSTALL_PREFIX=./install'
        cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_bullseye_32/lib/libSDL2_mixer_ext.a'
        cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/ ' + cwd
        run_cmd('schroot --chroot debian_bullseye -- sh -c "' + cfg_cmd + '"')
        if not configure_only:
            run_cmd('schroot --chroot debian_bullseye -- sh -c "make -j ' + str(data["threads"]) + '"')
            run_cmd('schroot --chroot debian_bullseye -- sh -c "cpack"')
            run_cmd("cp *.deb " + output_dir + "/" + appname + "-gnunux.deb")
            run_cmd("cp *.rpm " + output_dir + "/" + appname + "-gnunux.rpm")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["steam"]:
    try:
        print("### BUILDING LINUX/STEAMOS")
        begin = time.perf_counter()
        run_cmd("rm -rf " + steam_buildir)
        run_cmd("mkdir -p " + steam_buildir)
        chdir(steam_buildir)
        run_cmd("mkdir -p install")
        # Use static SDL2 mixer ext for simplicity
        cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
        cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_steam/lib/libSDL2_mixer_ext.a'
        cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/'
        cfg_cmd += ' -DCMAKE_INSTALL_PREFIX=./install ' + cwd
        run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "' + cfg_cmd + '"')
        if not configure_only:
            run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make -j ' + str(data["threads"]) + '"')
            run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make install"')
            run_cmd("rm -rf " + steam_dir + "/linux")
            run_cmd("cp -r install " + steam_dir + "/linux")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["appimage"]:
    try:
        print("### BUILDING LINUX APPIMAGE")
        begin = time.perf_counter()
        run_cmd("rm -rf " + appimg_buildir)
        run_cmd("mkdir -p " + appimg_buildir)
        chdir(appimg_buildir)
        if use_schroot:
            cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
            cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
            cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_bullseye/lib/libSDL2_mixer_ext.a'
            cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/'
            cfg_cmd += ' -DCMAKE_INSTALL_PREFIX=/usr ' + cwd
            run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "' + cfg_cmd + '"')
            if not configure_only:
                run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "make -j ' + str(data["threads"]) + '"')
                run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "make install DESTDIR=install_dir"')
                run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "' + data["linuxdeploy"] + ' --appdir install_dir -e install_dir/usr/bin/' + gamename + ' --output appimage"')
        else:
            # Use static SDL2 mixer ext for simplicity
            cfg_cmd = cmake_cmd +' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
            cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build/lib/libSDL2_mixer_ext.a'
            run_cmd(cfg_cmd + " " + cwd)
            if not configure_only:
                run_cmd("make -j " + str(data["threads"]))
                run_cmd("make install DESTDIR=install_dir")
                run_cmd(data["linuxdeploy"] + " --appdir install_dir -e install_dir/usr/bin/" + gamename + " --output appimage")
        if not configure_only:
            run_cmd("cp *.AppImage " + output_dir + "/" + appname + "-gnunux.AppImage")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["mac"]:
    try:
        print("### BUILDING MAC")
        begin = time.perf_counter()
        run_cmd("rm -rf " + mac_buildir)
        run_cmd("mkdir -p " + mac_buildir)
        chdir(mac_buildir)
        cfg_cmd = cmake_cmd +' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_osxcross/lib/libSDL2_mixer_ext.a'
        run_cmd(cfg_cmd + " -DCMAKE_TOOLCHAIN_FILE=" + cwd + "/cmake/Toolchain-osxcross.cmake"
                + " -DSDL2_INCLUDE_DIR=" + data["mac_sdl_folder"]
                + " -DCMAKE_INSTALL_PREFIX=./install " + cwd)
        if not configure_only:
            run_cmd("make -j " + str(data["threads"]))
            run_cmd("make install")
            run_cmd('python3 ' + cwd + '/tools/fix_mac_lib_paths.py install/' + fullname + '.app/Contents/libs/*.dylib')
            run_cmd('python3 ' + cwd + '/tools/fix_mac_lib_paths.py install/' + fullname + '.app/Contents/MacOS/' + gamename)
            run_cmd("rm -rf " + steam_dir + "/mac")
            run_cmd("cp -r install " + steam_dir + "/mac")
            run_cmd("genisoimage -V " + gamename + ".app -D -R -apple -no-pad -o " + gamename + ".dmg install")
            run_cmd("cp " + gamename + ".dmg " + output_dir + "/" + appname + "-macos.dmg")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["win32"]:
    try:
        print("### BUILDING WIN32")
        begin = time.perf_counter()
        run_cmd("rm -rf " + win32_buildir)
        run_cmd("mkdir -p " + win32_buildir)
        chdir(win32_buildir)
        run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=" + cwd + "/cmake/Toolchain-mingw32.cmake "
                + "-DSDL2_INCLUDE_DIR=" + data["win32_sdl_folder"] + " " + cwd)
        if not configure_only:
            run_cmd("make -j " + str(data["threads"]) + "")
            run_cmd("cpack")
            run_cmd("cp *-win32.exe " + output_dir + "/" + appname + "-win32.exe")
            run_cmd("cmake -DCMAKE_INSTALL_PREFIX=./install .")
            run_cmd("mkdir -p install")
            run_cmd("make -j " + str(data["threads"]))
            run_cmd("make install")
            run_cmd("rm -rf " + steam_dir + "/windows")
            run_cmd("cp -r install " + steam_dir + "/windows")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["win64"]:
    try:
        print("### BUILDING WIN64")
        begin = time.perf_counter()
        run_cmd("rm -rf " + win64_buildir)
        run_cmd("mkdir -p " + win64_buildir)
        chdir(win64_buildir)
        run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=" + cwd + "/cmake/Toolchain-mingw32-x86_64.cmake "
                + "-DSDL2_INCLUDE_DIR=" + data["win64_sdl_folder"] + " " + cwd)
        if not configure_only:
            run_cmd("make -j " + str(data["threads"]) + "")
            run_cmd("cpack")
            run_cmd("cp *.exe " + output_dir + "/" + appname + "-win64.exe")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["androidapk"] or data["androidaab"]:
    try:
        print("### BUILDING ANDROID")
        begin = time.perf_counter()
        run_cmd("rm -rf " + android_buildir)
        run_cmd("mkdir -p " + android_buildir)
        chdir(android_buildir)
        run_cmd(cmake_cmd + " -DSOSAGE_CONFIG_ANDROID:BOOL=True"
                + " -DSDL2_SOURCE_PATH:PATH=" + data["sdl2_source_path"]
                + " -DSDL2_IMAGE_SOURCE_PATH:PATH=" + data["sdl2_image_source_path"]
                + " -DSDL2_MIXER_SOURCE_PATH:PATH=" + (data["sdl2_mixer_ext_source_path"] if data["use_sdl_mixer_ext"] else data["sdl2_mixer_source_path"])
                + " -DSDL2_TTF_SOURCE_PATH:PATH=" + data["sdl2_ttf_source_path"]
                + " -DYAML_SOURCE_PATH:PATH=" + data["libyaml_source_path"]
                + " -DLZ4_SOURCE_PATH:PATH=" + data["lz4_source_path"] + " " + cwd)
        chdir("android")
        if not configure_only:
            if data["androidapk"]:
                run_cmd("./gradlew assembleDebug --parallel --max-workers=" + str(data["threads"]))
                run_cmd("cp app/build/outputs/apk/debug/*.apk " + output_dir + "/" + appname + "-android.apk")
            if data["androidaab"]:
                run_cmd("cp " + raw_data_folder + "/*.keystore app/")
                run_cmd("./gradlew bundleRelease --parallel --max-workers=" + str(data["threads"]))
                run_cmd("cp app/build/outputs/bundle/release/*.aab " + output_dir + "/" + appname + "-android.aab")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["browser"]:
    try:
        print("### BUILDING EMSCRIPTEN")
        begin = time.perf_counter()
        run_cmd("rm -rf " + emscripten_buildir)
        run_cmd("mkdir -p " + emscripten_buildir)
        chdir(emscripten_buildir)
        run_cmd("emcmake cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_DATA_FOLDER=" + data_folder
                + " -DYAML_SOURCE_PATH:PATH=" + data["libyaml_source_path"]
                + " -DLZ4_SOURCE_PATH:PATH=" + data["lz4_source_path"] + " " + cwd)
        if not configure_only:
            run_cmd("make -j " + str(data["threads"]))
            run_cmd("rm -rf " + output_dir + "/" + appname + "-web/")
            run_cmd("mkdir -p " + output_dir + "/" + appname + "-web/")
            run_cmd("cp -r data " + output_dir + "/" + appname + "-web/")
            run_cmd("cp *.data *.js *.wasm " + output_dir + "/" + appname + "-web/")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

all_end = time.perf_counter()
print(" ==> all done in " + str(int(all_end - all_begin)) + "s")
