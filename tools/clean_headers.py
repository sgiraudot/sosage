import os
import sys
import subprocess

verbose = (len(sys.argv) == 3 and sys.argv[2] == "-v")
simulate = (len(sys.argv) == 3 and sys.argv[2] == "-s")

def run_cmd(cmd):
    if simulate:
        print(cmd)
    else:
        try:
            if verbose:
                out = subprocess.run(cmd, shell=True, check=True)
            else:
                out = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True, check=True)
        except Exception as e:
            raise e
        else:
            if out.returncode != 0:
                raise Exception("'" + cmd + "' returned error code " + str(out.returncode))

def chdir(folder):
    if simulate:
        print("cd " + folder)
    else:
        os.chdir(folder)

if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " [root_folder]")
    exit()

root = sys.argv[1]

headers = []
sources = []

for r, directories, filenames in os.walk(root + "/include/"):
    for filename in filenames:
        fullname = os.path.join(r, filename)
        basename = fullname.split("/include/Sosage/")[1]
        headers.append(basename)

for r, directories, filenames in os.walk(root + "/src/"):
    for filename in filenames:
        fullname = os.path.join(r, filename)
        basename = fullname.split("/src/Sosage/")[1]
        sources.append(basename)

files = headers + sources
includes = {}

for f in files:
    if f.endswith('.h'):
        content = open(root + "/include/Sosage/" + f).read().split('\n')
    else:
        content = open(root + "/src/Sosage/" + f).read().split('\n')

    includes[f] = []
    for c in content:
        if "<Sosage/" in c:
            header = c.split('<Sosage/')[1].split('>')[0]
            includes[f].append(header)

nb_included = {}

for f in files:
    if f not in nb_included:
        nb_included[f] = 0
    todo = [f]
    while todo:
        current = todo[0]
        todo = todo[1:]
        for i in includes[current]:
            if i in nb_included:
                nb_included[i] += 1
            else:
                nb_included[i] = 1
            todo.append(i)

files.sort(key=lambda x: nb_included[x])

#for f in files:
#    print(f + ": " + str(nb_included[f]))

copy_root = "/tmp/sosage_clean_headers"
run_cmd("rm -rf " + copy_root)
run_cmd("mkdir " + copy_root)
run_cmd("cp -r " + root + "/include/ " + copy_root)
run_cmd("cp -r " + root + "/src/ " + copy_root)
run_cmd("cp -r " + root + "/cmake/ " + copy_root)
run_cmd("cp " + root + "/CMakeLists.txt " + copy_root)
run_cmd("cp " + root + "/LICENSE.md " + copy_root)
run_cmd("mkdir " + copy_root + "/build")
chdir(copy_root + "/build")
run_cmd("cmake -DCMAKE_CXX_FLAGS=-Wfatal-errors -DCMAKE_BUILD_TYPE=Debug -DSOSAGE_CFG_PROFILE:BOOL=True -DSOSAGE_DATA_FOLDER=/home/gee/art/superflu_riteurnz ..")
run_cmd("make -j 6")

success = []

for f in files:
    if len(includes[f]) == 0:
        continue

    if f.endswith('.h'):
        folder = "include/Sosage"
    else:
        folder = "src/Sosage"
    content = open(root + "/" + folder + "/" + f).read().split('\n')

    removed = set()

    for i in includes[f]:
        if "Config/" in i:
            continue
        run_cmd("rm " + copy_root + "/" + folder + "/" + f)
        file = open(copy_root + "/" + folder + "/" + f, 'w')
        for c in content:
            if "<Sosage/" + i in c:
                print("Trying to remove " + i + " from " + f)
                print(c)
                continue
            if "<Sosage/" in c:
                header = c.split('<Sosage/')[1].split('>')[0]
                if header in removed:
                    continue
            file.write(c + "\n")
        file.close()

        try:
            run_cmd("make -j 6")
        except:
            print(" -> failure")
        else:
            print(" -> success!")
            removed.add(i)
            success.append("Removing " + i + " from " + f)

    run_cmd("rm " + copy_root + "/" + folder + "/" + f)
    file = open(copy_root + "/" + folder + "/" + f, 'w')
    for c in content:
        if "<Sosage/" in c:
            header = c.split('<Sosage/')[1].split('>')[0]
            if header in removed:
                continue
        file.write(c + "\n")
    file.close()

print(str(len(success)) + " successful removal(s):")
for s in success:
    print(s)

# Time before = 73s
    
