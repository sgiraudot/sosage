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

v_major = ''
v_minor = ''
v_patch = ''
v_data = ''
v_variant = ''

for line in open('cmake/Sosage_version.cmake').readlines():
    if 'SOSAGE_VERSION_MAJOR ' in line:
        v_major = line.split('SOSAGE_VERSION_MAJOR ')[1].split(')')[0]
    if 'SOSAGE_VERSION_MINOR ' in line:
        v_minor = line.split('SOSAGE_VERSION_MINOR ')[1].split(')')[0]
    if 'SOSAGE_VERSION_PATCH ' in line:
        v_patch = line.split('SOSAGE_VERSION_PATCH ')[1].split(')')[0]
for line in open(data["folder"] + '/data/data/init.yaml').readlines():
    if 'data_version: ' in line:
        v_data = line.split('data_version: ')[1][:-1]
    if 'data_variant: ' in line:
        v_variant = line.split('data_variant: ')[1][1:-2]
        break

if v_data == '' or v_major == '' or v_minor == '' or v_patch == '':
    print("Couldn't read version")
    exit()

#version = v_major + '.' + v_minor + '.' + v_patch + '-d' + v_data + '-' + v_variant
version = v_major + '.' + v_minor + '.' + v_patch + '-d' + v_data
print(version)
    
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
scap_buildir = data["buildfolder"] + "/scap"
linux_buildir = data["buildfolder"] + "/linux"
steam_linux_buildir = data["buildfolder"] + "/steam_linux"
android_buildir = data["buildfolder"] + "/android"
mac_buildir = data["buildfolder"] + "/mac"
steam_mac_buildir = data["buildfolder"] + "/steam_mac"
win_buildir = data["buildfolder"] + "/win"
steam_win_buildir = data["buildfolder"] + "/steam_win"
output_dir = data["output"] + "/release-v" + version
steam_dir = output_dir + "/steam"
appname = gamename + "-" + version
dataname = gamename + "-d" + v_data
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


def configure(ifile, ofile):
    open(ofile, 'w').write(open(ifile, 'r').read().replace('${SOSAGE_EXE_NAME}', gamename))
    run_cmd("chmod +x " + ofile)
        
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
    run_cmd("rm -rf " + scap_buildir)
    run_cmd("mkdir -p " + scap_buildir)
    chdir(scap_buildir)
    run_cmd("cmake -DCMAKE_BUILD_TYPE=" + data["build"] + " -DSOSAGE_COMPILE_SCAP:BOOL=True -DSOSAGE_DATA_FOLDER=" + raw_data_folder + " " + cwd)
    run_cmd("make -j " + str(data["threads"]) + " SCAP")
    end = time.perf_counter()
    print("  -> done in " + str(int(end - begin)) + "s\n")

    if not configure_only:
        print("### PACKAGING DATA")
        begin = time.perf_counter()
        run_cmd("rm -rf " + data_folder)
        run_cmd("mkdir -p " + data_folder)
        run_cmd("./SCAP " + raw_data_folder + " " + data_folder)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    chdir(cwd)

if data["data"]:
    try:
        print("### BUILDING DATA ZIP")
        begin = time.perf_counter()
        run_cmd("rm -rf " + output_dir + "/" + dataname + "-data.zip")
        run_cmd("rm -rf " + data_dir)
        run_cmd("mkdir -p " + data_dir)
        chdir(data_dir)
        copy_data_dir = dataname + "-data"
        run_cmd("rm -rf " + copy_data_dir)
        run_cmd("mkdir -p " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/resources " + copy_data_dir)
        run_cmd("cp -r " + data_folder + "/data " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/config.cmake " + copy_data_dir)
        run_cmd("cp -r " + raw_data_folder + "/README.md " + copy_data_dir)
        run_cmd("zip -r " + output_dir + "/" + dataname + "-data-only.zip " + copy_data_dir)
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["linux"]:
    try:
        print("### BUILDING LINUX")
        begin = time.perf_counter()
        run_cmd("rm -rf " + linux_buildir)
        run_cmd("mkdir -p " + linux_buildir)
        chdir(linux_buildir)
        run_cmd("mkdir -p install")
        configure(cwd + '/platform/linux/install.sh', 'install/Install-' + gamename + '.sh')
        configure(cwd + '/platform/linux/uninstall.sh', 'install/Uninstall-' + gamename + '.sh')
        run_cmd("cp -r " + data_folder + "/resources/LICENSE.md install/")
 
        cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
        cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_bullseye/lib/libSDL2_mixer_ext.a'
        cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/ ' + cwd
        cfg_cmd += ' -DCMAKE_INSTALL_PREFIX=./install ' + cwd
        run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "' + cfg_cmd + '"')
        if not configure_only:
            run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "make -j ' + str(data["threads"]) + '"')
            run_cmd('schroot --chroot debian_bullseye_64 -- sh -c "make install"')
            run_cmd("mv install " + appname)
            run_cmd("tar -czvf " + output_dir + "/" + appname + "-gnunux.tar.gz " + appname)
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["steam_linux"]:
    try:
        print("### BUILDING LINUX FOR STEAM")
        begin = time.perf_counter()
        run_cmd("rm -rf " + steam_linux_buildir)
        run_cmd("mkdir -p " + steam_linux_buildir)
        chdir(steam_linux_buildir)
        run_cmd("mkdir -p install")
        # Use static SDL2 mixer ext for simplicity
        cfg_cmd = cmake_cmd + ' -DYAML_INCLUDE_DIR=' + data["libyaml_source_path"] + '/include/'
        cfg_cmd += ' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSDL2_MIXER_EXT_LIBRARY:FILEPATH=' + data["sdl2_mixer_ext_source_path"] + '/build_steam/lib/libSDL2_mixer_ext.a'
        cfg_cmd += ' -DLZ4_INCLUDE_DIR=' + data["lz4_source_path"] + '/lib/'
        cfg_cmd += ' -DSTEAMSDK_ROOT:PATH=' + data["steam_sdk"]
        cfg_cmd += ' -DSOSAGE_STEAM_APP_ID=' + data["steam_app_id"]
        cfg_cmd += ' -DCMAKE_INSTALL_PREFIX=./install ' + cwd
        run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "' + cfg_cmd + '"')
        if not configure_only:
            run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make -j ' + str(data["threads"]) + '"')
            run_cmd('schroot --chroot steamrt_scout_amd64 -- sh -c "make install"')
            run_cmd("mkdir -p install/lib")
            run_cmd('cp ' + data["steam_sdk"] + "/redistributable_bin/linux64/libsteam_api.so install/lib")
            run_cmd('patchelf --replace-needed libsteam_api.so ../lib/libsteam_api.so install/bin/' + gamename)
            run_cmd("rm -rf " + steam_dir + "/linux")
            run_cmd("cp -r install " + steam_dir + "/linux")
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
            run_cmd("genisoimage -V " + gamename + ".app -D -R -apple -no-pad -o " + gamename + ".dmg install")
            run_cmd("cp " + gamename + ".dmg " + output_dir + "/" + appname + "-macos.dmg")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["steam_mac"]:
    try:
        print("### BUILDING MAC FOR STEAM")
        begin = time.perf_counter()
        run_cmd("rm -rf " + steam_mac_buildir)
        run_cmd("mkdir -p " + steam_mac_buildir)
        chdir(steam_mac_buildir)
        cfg_cmd = cmake_cmd +' -DSDL2_MIXER_EXT_INCLUDE_DIR:PATH=' + data["sdl2_mixer_ext_source_path"] + '/include/SDL_mixer_ext'
        cfg_cmd += ' -DSTEAMSDK_ROOT:PATH=' + data["steam_sdk"]
        cfg_cmd += ' -DSOSAGE_STEAM_APP_ID=' + data["steam_app_id"]
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
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["win"]:
    try:
        print("### BUILDING WINDOWS")
        begin = time.perf_counter()
        run_cmd("rm -rf " + win_buildir)
        run_cmd("mkdir -p " + win_buildir)
        chdir(win_buildir)
        run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=" + cwd + "/cmake/Toolchain-mingw32-x86_64.cmake "
                + "-DSDL2_INCLUDE_DIR=" + data["win64_sdl_folder"] + " " + cwd)
        if not configure_only:
            run_cmd("make -j " + str(data["threads"]) + "")
            run_cmd("cpack")
            run_cmd("cp *-win64.exe " + output_dir + "/" + appname + "-windows.exe")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

if data["steam_win"]:
    try:
        print("### BUILDING WINDOWS FOR STEAM")
        begin = time.perf_counter()
        run_cmd("rm -rf " + steam_win_buildir)
        run_cmd("mkdir -p " + steam_win_buildir)
        chdir(steam_win_buildir)
        run_cmd(cmake_cmd + " -DCMAKE_TOOLCHAIN_FILE=" + cwd + "/cmake/Toolchain-mingw32-x86_64.cmake "
                + ' -DSOSAGE_STEAM_APP_ID=' + data["steam_app_id"]
                + ' -DSTEAMSDK_ROOT:PATH=' + data["steam_sdk"] + " -DCMAKE_INSTALL_PREFIX=./install ."
                + " -DSDL2_INCLUDE_DIR=" + data["win64_sdl_folder"] + " " + cwd)
        if not configure_only:
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

if data["androidapk"] or data["androidaab"]:
    try:
        print("### CONFIGURING ANDROID")
        begin = time.perf_counter()
        run_cmd("rm -rf " + android_buildir)
        run_cmd("mkdir -p " + android_buildir)
        chdir(android_buildir)
        run_cmd(cmake_cmd + " -DSOSAGE_CONFIG_ANDROID:BOOL=True"
                + " -DSOSAGE_KEYSTORE_ALIAS=" + data["keystore_alias"]
                + " -DSOSAGE_KEYSTORE_PASSWORD=" + data["keystore_password"]
                + " -DSDL2_SOURCE_PATH:PATH=" + data["sdl2_source_path"]
                + " -DSDL2_IMAGE_SOURCE_PATH:PATH=" + data["sdl2_image_source_path"]
                + " -DSDL2_MIXER_SOURCE_PATH:PATH=" + (data["sdl2_mixer_ext_source_path"] if data["use_sdl_mixer_ext"] else data["sdl2_mixer_source_path"])
                + " -DSDL2_TTF_SOURCE_PATH:PATH=" + data["sdl2_ttf_source_path"]
                + " -DYAML_SOURCE_PATH:PATH=" + data["libyaml_source_path"]
                + " -DLZ4_SOURCE_PATH:PATH=" + data["lz4_source_path"] + " " + cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")

        if not configure_only and data["androidapk"]:
            print("### BUILDING ANDROID APK")
            begin = time.perf_counter()
            chdir("android_apk")
            run_cmd("./gradlew assembleDebug --parallel --max-workers=" + str(data["threads"]))
            run_cmd("cp app/build/outputs/apk/debug/*.apk " + output_dir + "/" + appname + "-android.apk")
            chdir("..")
            end = time.perf_counter()
            print("  -> done in " + str(int(end - begin)) + "s\n")
        if not configure_only and data["androidaab"]:
            print("### BUILDING ANDROID AAB")
            begin = time.perf_counter()
            chdir("android_aab")
            run_cmd("cp " + raw_data_folder + "/*.keystore app/")
            run_cmd("./gradlew bundleRelease --parallel --max-workers=" + str(data["threads"]))
            run_cmd("cp app/build/outputs/bundle/release/*.aab " + output_dir + "/" + appname + "-android.aab")
            chdir("..")
            end = time.perf_counter()
            print("  -> done in " + str(int(end - begin)) + "s\n")
        chdir(cwd)
        end = time.perf_counter()
        print("  -> done in " + str(int(end - begin)) + "s\n")
    except:
        chdir(cwd)
        print("  -> failed")

all_end = time.perf_counter()
print(" ==> all done in " + str(int(all_end - all_begin)) + "s")
