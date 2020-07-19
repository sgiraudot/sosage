import os
import sys
import subprocess

root = "../data/images/"

# Nice display of size
def size_str(num):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            return "%3.1f %s" % (num, x)
        num /= 1024.0
    return str(num)
    

for root, directories, filenames in os.walk(root):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)

        # Map files require exact colors and are thus skipped
        if ext != ".png" or basename.find("_map.png") != -1:
            continue

        # Skip if file was already compressed using colormaps
        fileinfo = str(subprocess.run(['file', fullname], stdout=subprocess.PIPE).stdout)
        if fileinfo.find("colormap") != -1:
            continue

        # Check size before, compress, check size after + gain
        size_before = os.path.getsize(fullname)
        cmd = "pngquant -Q 0-50 --ext .png --force " + fullname
        os.system(cmd)
        size_after = os.path.getsize(fullname)
        gain = int(100. * (1. - (size_after / float(size_before))))

        # Output
        print("[" + fullname + "]")
        print(" * Original size = " + size_str(size_before))
        print(" * Compressed = " + size_str(size_after) + " (gain = " + str(gain) + "%)")
        print("")

        
