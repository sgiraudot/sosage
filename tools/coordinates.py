import sys

nb_arg = len(sys.argv)
H = 1080
inkscape_updated = True

try:
    if nb_arg == 4:
        if sys.argv[1] == "depth":
            back = int(sys.argv[2])
            front = int(sys.argv[3])
            if back > front:
                back, front = front, back
            # Z = 3240 -> H = 532
            # Z = 1330 -> H = 217
            # Z = a * H
            # a = 3240 / 535 = 6.06
            factor = 6.06
            print("back_z: " + str(int(back * factor)))
            print("front_z: " + str(int(front * factor)))
        else:
            raise Exception()
    elif nb_arg == 5:
        ix = int(sys.argv[1])
        iy = int(sys.argv[2])
        iw = int(sys.argv[3])
        ih = int(sys.argv[4])
        ox = ix + iw // 2
        oy = iy + ih
        print("coordinates: [" + str(ox) + ", " + str(oy) + "]")

    elif nb_arg == 6:
        if sys.argv[1] == "label":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            iw = int(sys.argv[4])
            ih = int(sys.argv[5])
            ox = ix + iw // 2
            oy = iy + ih // 2 if inkscape_updated else H - iy - ih // 2
            print("label: [" + str(ox) + ", " + str(oy) + "]")
        elif sys.argv[1] == "char":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            iw = int(sys.argv[4])
            ih = int(sys.argv[5])
            ox = ix + iw // 2
            oy = iy + round(0.05 * ih) if inkscape_updated else H - iy - round(0.05 * ih)
            print("coordinates: [" + str(ox) + ", " + str(oy) + "]")
        else:
            raise Exception()
    elif nb_arg == 7:
        if sys.argv[1] == "code":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            iw = int(sys.argv[4])
            ih = int(sys.argv[5])
            H = int(sys.argv[6])
            ox = ix
            oy = iy + ih if inkscape_updated else H - iy - ih
            print("coordinates: [" + str(ox) + ", " + str(oy) + ", " + str(iw) + ", " + str(ih) + "]")
        else:
            raise Exception()
    else:
        raise Exception()
except:
    print("Usages: ")
    print(" * " + sys.argv[0] + " X Y W H")
    print(" * " + sys.argv[0] + " char X Y W H")
    print(" * " + sys.argv[0] + " label X Y W H")
    print(" * " + sys.argv[0] + " depth HEIGHT_BACK HEIGHT_FRONT")
