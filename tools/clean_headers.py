import os
import sys
import subprocess

if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " [header_folder] [build_folder]")
    exit()

root = sys.argv[1]
build = sys.argv[2]

#fileinfo = str(subprocess.run(['file', fullname], stdout=subprocess.PIPE).stdout)

def write_file(filename, header, includes, content):
    with open(filename, "w") as f:
        for h in header:
            f.write(h)
        for h in includes:
            f.write(h)
        for h in content:
            f.write(h)

def test_name(filename):
    return "check_header_" + os.path.abspath(filename).replace('/', '_').replace('.', '_')


def test(filename, build, header, includes, content):

    for i in range(len(includes)):
        if not includes[i].startswith("#include"):
            continue
        inc = includes[:i] + includes[i+1:]
        write_file(filename, header, inc, content)
        wd = os.getcwd()
        os.chdir(build)
        subprocess.run(['make', 'clean'], stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))
        if subprocess.run(['make', '-j', '6', test_name(filename)], stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb')).returncode == 0:
            print("  " + includes[i][:-1])
        os.chdir(wd)

    write_file(filename, header, includes, content)



for root, directories, filenames in os.walk(root):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)

        # Map files require exact colors and are thus skipped
        if ext != ".h":
            continue

        print("[" + fullname + "]")

        with open(fullname) as fp:
            line = fp.readline()
            cnt = 1

            header = []
            includes = []
            content = []

            in_include = False
            in_content = False

            while line:
                if line.startswith("namespace"):
                    in_content = True
                    in_include = False

                if in_include or line.startswith("#include"):
                    in_include = True
                    includes.append(line)
                elif in_content:
                    content.append(line)
                else:
                    header.append(line)
                line = fp.readline()
                cnt += 1

#            print(str(len(header)) + " " + str(len(includes)) + " " + str(len(content)))
            test(fullname, build, header, includes, content)
