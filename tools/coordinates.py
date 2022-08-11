import sys

nb_arg = len(sys.argv)
H = 1080

try:
    if nb_arg == 2:
        h = int(sys.argv[1])
        print("depth: " + str(3240 * h / 519.113))
    elif nb_arg == 4:
        if sys.argv[1] == "view":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            ox = ix
            oy = H - iy
            print("view: [" + str(ox) + ", " + str(oy) + "]")
        elif sys.argv[1] == "depth":
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
            ix = int(sys.argv[1])
            iy = int(sys.argv[2])
            iw = int(sys.argv[3])
            ox = ix + iw // 2
            oy = H - iy
            print("coordinates: [" + str(ox) + ", " + str(oy) + "]")

    elif nb_arg == 6:
        if sys.argv[1] == "label":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            iw = int(sys.argv[4])
            ih = int(sys.argv[5])
            ox = ix + iw // 2
            oy = H - iy - ih // 2
            print("label: [" + str(ox) + ", " + str(oy) + "]")
        elif sys.argv[1] == "char":
            ix = int(sys.argv[2])
            iy = int(sys.argv[3])
            iw = int(sys.argv[4])
            ih = int(sys.argv[5])
            ox = ix + iw // 2
            oy = H - iy - round(0.05 * ih)
            print("coordinates: [" + str(ox) + ", " + str(oy) + "]")

    else:
        raise Exception()
except:
    print("Usages: ")
    print(" * " + sys.argv[0] + " X Y W")
    print(" * " + sys.argv[0] + " char X Y W H")
    print(" * " + sys.argv[0] + " label X Y W H")
    print(" * " + sys.argv[0] + " view X Y")
    print(" * " + sys.argv[0] + " depth HEIGHT_BACK HEIGHT_FRONT")
    print(" * " + sys.argv[0] + " CHARACTER_HEIGHT")
