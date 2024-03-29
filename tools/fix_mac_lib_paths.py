import subprocess
import sys

verbose = True
simulate = False

def run_cmd(cmd):
    if simulate:
        print(cmd)
    else:
        try:
            if verbose:
                print(cmd)
                out = subprocess.run(cmd, shell=True, check=True)
            else:
                out = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True, check=True)
        except Exception as e:
            print("The following command raised an exception:")
            print("  " + cmd)
            print("  " + str(e))
            exit()
        else:
            if out.returncode != 0:
                print("The following command returned error code " + str(out.returncode))
                print("  " + cmd)
                exit()

for exe in sys.argv[1:]:
    libs = subprocess.check_output('x86_64-apple-darwin19-otool -L "' + exe + '"', shell=True).decode().split('\n')
    for l in libs:
        if '/opt/local/lib/' in l:
            lname = l.split('/opt/local/lib/')[1].split(' ')[0]
            old = '/opt/local/lib/' + lname
            new = '@executable_path/../libs/' + lname
            if lname == exe.split('/')[-1]:
                run_cmd('x86_64-apple-darwin19-install_name_tool -id "' + new + '" "' + exe + '"')
            else:
                run_cmd('x86_64-apple-darwin19-install_name_tool -change "' + old + '" "' + new + '" "' + exe + '"')
        elif '@loader_path/' in l:
            lname = l.split('@loader_path/')[1].split(' ')[0]
            old = '@loader_path/' + lname
            new = '@executable_path/../libs/' + lname
            if lname == exe.split('/')[-1]:
                run_cmd('x86_64-apple-darwin19-install_name_tool -id "' + new + '" "' + exe + '"')
            else:
                run_cmd('x86_64-apple-darwin19-install_name_tool -change "' + old + '" "' + new + '" "' + exe + '"')
