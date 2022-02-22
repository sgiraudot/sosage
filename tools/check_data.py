import os
import sys
import subprocess
import yaml
import re

if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " [data_folder]")
    exit()

root_folder = sys.argv[1]
verbose = False
if len(sys.argv) > 2 and sys.argv[2] == '-v':
    verbose = True
exit_at_first_error = True

errors = []
def error(string):
    if exit_at_first_error:
        print("[" + refname + "] " + string)
        exit(0)
    else:
        errors.append([True, refname, string])

def warning(string):
    errors.append([False, refname, string])

def load_yaml(filename):
    try:
        yaml_file = open(filename, 'r')
        data = yaml.safe_load(yaml_file)
        return data
    except yaml.scanner.ScannerError:
        return None

def is_convertible_to_int(string):
    try:
        int(string)
        return True
    except ValueError:
        return False

def is_convertible_to_float(string):
    try:
        float(string)
        return True
    except ValueError:
        return False

def child(data, key):
    if is_convertible_to_int(key):
        idx = int(key)
        if idx < len(data):
            return data[idx]
    if key in data:
        return data[key]
    return None

def get(data, key):
    if '/' in key:
        current, future = key.split('/', 1)
        c = child(data, current)
        if not c:
            return None
        return get(c, future)
    return child(data, key)

def test(data, key, func=None, args=None):
    if verbose:
        if "id" in data:
            print("Testing " + data["id"] + " on key " + key + " with " + str(func))
        else:
            print("Testing on key " + key + " with " + str(func))
    value = get(data, key)
    if value is None:
        if "id" in data:
            error(key + " not found in " + data["id"])
        else:
            error(key + " not found")
        return False
    if func:
        if args:
            func(key, value, args)
        else:
            func(key, value)
    return True

def is_int(key, value):
    if type(value) is not int:
        error(key + "=" + str(value) + " is not an integer")

def is_bool(key, value):
    if type(value) is not bool:
        error(key + "=" + str(value) + " is not a boolean")

def is_color(key, value):
    if not re.search(r'^(?:[0-9a-fA-F]{3}){1,2}$', value):
        error(key + " is not a valid color (" + value + ")")

def is_array(key, value, args=None):
    if not isinstance(value, list):
        error(key + " is not a list")
    elif args and len(value) != args:
        error(key + " is a list of size " + str(len(value)) + " instead of " + str(args))

def is_time(key, value):
    is_t = True
    if ':' not in value:
        is_t = False
    else:
        minutes, seconds = value.split(':', 1)
        if not is_convertible_to_int(minutes) or not is_convertible_to_float(seconds):
            is_t = False
        elif float(seconds) >= 60.0:
            is_t = False
    if not is_t:
        error(key + " is not a valid time (" + value + ")")

accessed_files = set()

def file_exists(key, value, args):
    fname = args[0] + "/" + value + "." + args[1]
    if not os.path.exists(root_folder + "/" + fname):
        error(key + " refers to a non-existing file (" + fname + ")")
    else:
        accessed_files.add(root_folder + "/" + fname)

def test_id_unicity(ids, new_id, ref=None):
    if new_id in ids:
        if ref is None:
            error("multiple definitions of " + new_id)
        else:
            error(new_id + " already used in " + ref[new_id])
    else:
        ids.add(new_id)
        if ref is not None:
            ref[new_id] = filename
    return ids, ref

def check_signature(key, funcname, args, signature):
    if len(args) != len(signature):
        error(key + " uses function " + funcname + " with #args != " + str(len(signature)))
        return False
    else:
        for i in range(len(args)):
            if signature[i] == "int" and not is_convertible_to_int(args[i]):
                error(key + " uses function " + funcname + " with non-int argument " + str(args[i]))
                return False
            elif signature[i] == "float" and not is_convertible_to_float(args[i]):
                error(key + " uses function " + funcname + " with non-float argument " + str(args[i]))
                return False
    return True

def check_line(line):
    if '…' in line:
        warning("forbidden symbol '…' found in line " + line)
    for locale in locales:
        if line not in translation[locale]:
            warning("missing " + locale + " translation of '" + line + "'")

def is_line(key, value):
    check_line(value)

def test_step(key, action, args):
    if action == "add":
        if check_signature(key, action, args, ["string", "string"]):
            id = args[0]
            if is_convertible_to_int(args[1]):
                if id not in integer_ids:
                    error(key + " uses function add on non-existing integer " + id)
            elif args[1] not in action_ids:
                error(key + " uses function add on non-existing action " + args[1])

    elif action == "camera":
        if len(args) == 1:
            check_signature(key, action, args, ["int"])
        elif len(args) == 2:
            check_signature(key, action, args, ["int", "int"])
        elif len(args) == 3:
            check_signature(key, action, args, ["int", "int", "float"])
        else:
            error(key + " uses function camera with unhandled #arg = " + str(len(args)))

    elif action == "control":
        if len(args) == 1:
            id = args[0]
            if id not in character_ids:
                error(key + " uses function control on non-existing character " + id)
        elif len(args) == 2:
            for id in args:
                if id not in character_ids:
                    error(key + " uses function control on non-existing character " + id)
        else:
            error(key + " uses function control with " + str(len(args)) + " arguments")

    elif action in {"cutscene", "exit", "lock", "loop", "unlock", "skip", "include" }:
        if len(args) != 0:
            error(key + " uses function exit with arguments " + str(args))

    elif action == "fadein" or action == "fadeout":
        check_signature(key, action, args, ["float"])

    elif action == "goto":
        if len(args) == 0:
            pass
        elif len(args) == 1:
            id = args[0]
            if id not in object_ids and id not in character_ids:
                error(key + " uses function goto on non-existing (or non-reachable) id " + id)
        elif len(args) == 2:
            check_signature(key, action, args, ["int", "int"])
        elif len(args) == 3:
            if check_signature(key, action, args, ["string", "int", "int"]):
                id = args[0]
                if id not in character_ids:
                    error(key + " uses function goto on non-existing character " + id)
        else:
            error(key + " uses function goto with unhandled #arg = " + str(len(args)))

    elif action == "hide":
        if check_signature(key, action, args, ["string"]):
            id = args[0]
            if id not in room_ids and id not in hints_ids:
                error(key + " uses function hide on non-existing id " + id)

    elif action == "load":
        if check_signature(key, action, args, ["string", "string"]):
            # TODO check entry point
            pass

    elif action == "look":
        if len(args) == 0:
            pass
        elif len(args) == 1:
            id = args[0]
            if id not in object_ids and id not in character_ids:
                error(key + " uses function look on non-existing (or non-reachable) id " + id)
        elif len(args) == 2:
            char_id = args[0]
            if char_id not in character_ids:
                error(key + " uses function look on non-existing (or non-reachable) character " + id)
            id = args[1]
            if id not in object_ids and id not in character_ids:
                error(key + " uses function look on non-existing (or non-reachable) id " + id)
        else:
            error(key + " uses function look with unhandled #arg = " + str(len(args)))

    elif action == "message":
        check_signature(key, action, args, ["string"])
        id = args[0]
        if id not in text_ids:
            error(key + " uses function message on non-existing (or non-reachable) id " + id)

    elif action == "move":
        if len(args) == 4:
            id = args[0]
            if id in character_ids:
                check_signature(key, action, args, ["string", "int", "int", "bool"])
            else:
                check_signature(key, action, args, ["string", "int", "int", "int"])
                if id not in object_ids and id not in scenery_ids:
                    error(key + " uses function move on non-existing (or non-reachable) id " + id)
        elif len(args) == 5:
            id = args[0]
            if id in character_ids:
                check_signature(key, action, args, ["string", "int", "int", "int", "bool"])
            else:
                check_signature(key, action, args, ["string", "int", "int", "int", "float"])
                if id not in object_ids and id not in scenery_ids and id not in animation_ids:
                    error(key + " uses function move on non-existing (or non-reachable) id " + id)
        else:
            error(key + " uses function move with unhandled #arg = " + str(len(args)))

    elif action == "move60fps":
        check_signature(key, action, args, ["string", "int", "int", "int", "float"])
        id = args[0]
        if id not in object_ids and id not in scenery_ids and id not in animation_ids:
            error(key + " uses function move on non-existing (or non-reachable) id " + id)

    elif action == "play":
        if len(args) == 1:
            id = args[0]
            if id not in animation_ids and id not in music_ids and id not in sound_ids:
                error(key + " uses function play on non-existing animation/music/sound " + id)
        elif len(args) == 2:
            check_signature(key, action, args, ["string", "float"])
            id = args[0]
            # TODO check if valid animation of character?
        else:
            error(key + " uses function play with unhandled #arg = " + str(len(args)))

    elif action == "rescale":
        if len(args) == 2:
            check_signature(key, action, args, ["string", "float"])
            id = args[0]
            if id not in room_ids:
                error(key + " uses function rescale on non-existing (or non-reachable) id " + id)
        else:
            check_signature(key, action, args, ["string", "float", "float"])
            id = args[0]
            if id not in room_ids:
                error(key + " uses function rescale on non-existing (or non-reachable) id " + id)

    elif action == "rescale60fps":
        check_signature(key, action, args, ["string", "float", "float"])
        id = args[0]
        if id not in room_ids:
            error(key + " uses function rescale60fps on non-existing (or non-reachable) id " + id)

    elif action == "set":
        if len(args) == 2:
            check_signature(key, action, args, ["string", "string"])
            id = args[0]
            state0 = args[1]
            if id not in room_ids:
                error(key + " uses function set on non-existing (or non-reachable) id " + id)
            if state0 not in all_states[id]:
                error(key + " uses function set on non-existing state " + state0 + " of " + id)
        elif len(args) == 3:
            id = args[0]
            if id not in room_ids:
                error(key + " uses function set on non-existing (or non-reachable) id " + id)
            state0 = args[1]
            state1 = args[2]
            if state0 not in all_states[id]:
                error(key + " uses function set on non-existing state " + state0 + " of " + id)
            if state1 not in all_states[id]:
                error(key + " uses function set on non-existing state " + state1 + " of " + id)
        else:
            error(key + " uses function set with unhandled #arg = " + str(len(args)))

    elif action == "shake":
        check_signature(key, action, args, ["float", "float"])

    elif action == "show":
        if check_signature(key, action, args, ["string"]):
            id = args[0]
            if id not in room_ids and id not in hints_ids and id not in text_ids:
                error(key + " uses function show on non-existing id " + id)

    elif action == "stop":
        if check_signature(key, action, args, ["string"]):
            id = args[0]
            if id != "music" and id not in animation_ids:
                error(key + " uses function stop on non-existing animation " + id)

    elif action == "talk":
        if len(args) == 1:
            check_line(args[0])
        elif len(args) == 2:
            id = args[0]
            if id not in character_ids and id != "superflu":
                error(key + " uses function talk on non-existing character " + id)
            check_line(args[1])
        else:
            check_signature(key, action, args, ["string", "string", "float"])
            id = args[0]
            if id not in character_ids and id != "superflu":
                error(key + " uses function talk on non-existing character " + id)
            check_line(args[1])

    elif action == "timer":
        check_signature(key, action, args, ["string"])

    elif action == "trigger":
        if check_signature(key, action, args, ["string"]):
            id = args[0]
            menus = { "End", "Exit" } # TODO really test menus
            if id != "hints" and id not in action_ids and id not in dialog_ids and id not in menus:
                # TODO check also menu existence
                error(key + " uses function trigger on non-existing action/dialog/menu " + id)

    elif action == "wait":
        if len(args) == 0:
            pass
        elif len(args) == 1:
            check_signature(key, action, args, ["float"])
        else:
            # TODO check timer
            check_signature(key, action, args, ["string", "float"])

    elif action == "zoom":
        check_signature(key, action, args, ["float"])

    else:
        error(key + " uses unknown function " + action)

def test_action(key, value):
    for v in value:
        action = next(iter(v))
        args = v[action]
        if not isinstance(args, list):
            error(key + " uses ill-formed arguments: " + str(args))
            continue
        test_step(key, action, args)


data_folder = root_folder + "/data/"
yaml_files = []
skip_list = { "ba_fochougny.yaml" }

for root, directories, filenames in os.walk(data_folder):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)
        if ext != ".yaml":
            if "locale.yaml" not in fullname:
                print("Warning: non yaml file found: " + fullname)
            continue

        if basename in skip_list:
            continue
        relname = fullname.split("data/")[-1]
        yaml_files.append(relname)


yaml_files.sort()

all_ids = set()
ref_ids = {}
inventory_ids = set()
all_states = {}
hints_ids = set()

data = load_yaml(data_folder + "locale.yaml")
locales = [ l["id"] for l in data["locales"] if l["id"] != 'fr_FR']

translation = { l: {} for l in locales }
for line in data["lines"]:
    for locale in locales:
        translation[locale][line['fr_FR']] = line[locale]

data = load_yaml(data_folder + "hints.yaml")
if test(data, "hints"):
    for h in data["hints"]:
        if test(h, "id"):
            all_ids, hints_id = test_id_unicity(hints_ids, h["id"])
        test(h, "question", is_line)
        test(h, "answer", is_line)

iteration = 0
for filename in yaml_files:
    iteration += 1

    refname = filename
    current_id = filename.split('/', 1)[-1].split('.',1)[0]
    all_ids, ref_ids = test_id_unicity(all_ids, current_id, ref_ids)

    data = load_yaml(data_folder + filename)
    if not data:
        error("invalid YAML input")
        continue

    if filename.startswith("characters/"):
        test(data, "name", is_line)
        test(data, "coordinates/0", is_int)
        test(data, "coordinates/1", is_int)
        test(data, "looking_right", is_bool)
        test(data, "color", is_color)
        test(data, "mouth/skin", file_exists, ["images/characters", "png"])
        test(data, "mouth/dx_right", is_int)
        test(data, "mouth/dx_left", is_int)
        test(data, "mouth/dy", is_int)
        test(data, "head/skin", file_exists, ["images/characters", "png"])
        test(data, "head/dx_right", is_int)
        test(data, "head/dx_left", is_int)
        test(data, "head/dy", is_int)
        test(data, "head/positions", is_array)
        if "walk" in data:
            test(data, "walk/skin", file_exists, ["images/characters", "png"])
        test(data, "idle/skin", file_exists, ["images/characters", "png"])
        test(data, "idle/positions", is_array)
    elif filename.startswith("codes/"):
        if test(data, "states"):
            all_states[current_id] = set()
            for s in data["states"]:
                if test(s, "id"):
                    all_states[current_id].add(s["id"])
                test(s, "skin/0", file_exists, ["images/windows", "png"])
                test(s, "skin/1", file_exists, ["images/windows", "png"])
        test(data, "button_sound", file_exists, ["sounds/effects", "ogg"])
        test(data, "success_sound", file_exists, ["sounds/effects", "ogg"])
        test(data, "failure_sound", file_exists, ["sounds/effects", "ogg"])

        values = []
        if test(data, "buttons"):
            for b in data["buttons"]:
                if test(b, "value"):
                    values.append(b["value"])
        if test(data, "answer"):
            for a in data["answer"]:
                if a not in values:
                    error(str(a) + " is not a valid button")
    elif filename.startswith("dialogs/"):
        if test(data, "lines"):
            ids = { "end" }
            has_end = False
            for l in data["lines"]:
                if "target" in l:
                    ids, _ = test_id_unicity(ids, l["target"])
            for l in data["lines"]:
                if "choices" in l:
                    for c in l["choices"]:
                        test(c, "line", is_line)
                        test(c, "once", is_bool)
                        if test(c, "goto"):
                            if c["goto"] not in ids:
                                error("goto refers to invalid id " + str(c["goto"]))
                            if c["goto"] == "end":
                                has_end = True
                elif "line" in l:
                    if test(l, "line", is_array, 2):
                        test(l, "line/1", is_line)
                elif "target" not in l:
                    if test(l, "goto"):
                        if l["goto"] not in ids:
                            error("goto refers to invalid id " + str(c["goto"]))
                        if l["goto"] == "end":
                            has_end = True
            if not has_end:
                error("dialog has no end")

    elif filename.startswith("objects/"):
        test(data, "name", is_line)
        test(data, "coordinates/0", is_int)
        test(data, "coordinates/1", is_int)
        test(data, "coordinates/2", is_int)
        test(data, "box_collision", is_bool)
        test(data, "view/0", is_int)
        test(data, "view/1", is_int)

        inventory_only = True
        if test(data, "states"):
            all_states[current_id] = set()
            for s in data["states"]:
                if not test(s, "id"):
                    continue
                sid = s["id"]
                all_states[current_id].add(sid)
                if "skin" in s:
                    if sid != "inventory":
                        inventory_only = False
                    if sid == "inventory":
                        test(s, "skin", file_exists, ["images/inventory", "png"])
                        inventory_ids.add(current_id)
                    else:
                        test(s, "skin", file_exists, ["images/objects", "png"])
                elif "size" in s:
                    if sid != "inventory":
                        inventory_only = False
                    test(s, "size/0", is_int)
                    test(s, "size/1", is_int)
                if "frames" in s:
                    test(s, "frames", is_int)
                    test(s, "duration", is_int)

        if not inventory_only:
            test(data, "label/0", is_int)
            test(data, "label/1", is_int)

    elif filename.startswith("rooms/"):
        test(data, "name", is_line)
        if "background" in data:
            test(data, "background", file_exists, ["images/backgrounds", "png"])
        if "ground_map" in data:
            test(data, "ground_map", file_exists, ["images/backgrounds", "png"])
            test(data, "front_z", is_int)
            test(data, "back_z", is_int)

        room_ids = inventory_ids.copy()

        action_ids = set()
        if "actions" in data:
            for a in data["actions"]:
                refname = filename + ":actions"
                id = ''
                if "id" in a:
                    id = a["id"]
                    refname = filename + ":" + a["id"]
                    all_ids, ref_ids = test_id_unicity(all_ids, id, ref_ids)
                else:
                    fname = data_folder + "actions/" + a + ".yaml"
                    if not os.path.exists(fname):
                        error(fname + " does not exist")
                        continue
                    else:
                        id = a
                        a = load_yaml(fname)
                action_ids.add(id)
                room_ids.add(id)

                if "states" in a:
                    all_states[id] = set()
                    for s in a["states"]:
                        if test(s, "id"):
                            all_states[id].add(s["id"])

        animation_ids = set()
        if "animations" in data:
            for a in data["animations"]:
                refname = filename + ":animations"
                if test(a, "id"):
                    refname = filename + ":" + a["id"]
                    room_ids.add(a["id"])
                    animation_ids.add(a["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, a["id"], ref_ids)
                test(a, "skin", file_exists, ["images/animations", "png"])
                if test(a, "length"):
                    if isinstance(a["length"], list):
                        test(a, "length/0", is_int)
                        test(a, "length/1", is_int)
                    else:
                        test(a, "length", is_int)
                test(a, "coordinates/0", is_int)
                test(a, "coordinates/1", is_int)
                test(a, "coordinates/2", is_int)
                test(a, "loop", is_bool)

        character_ids = set()
        if "characters" in data:
            for c in data["characters"]:
                refname = filename + ":" + c
                room_ids.add(c)
                character_ids.add(c)
                fname = data_folder + "characters/" + c + ".yaml"
                if not os.path.exists(fname):
                    error(fname + " does not exist")

        dialog_ids = set()
        if "dialogs" in data:
            for d in data["dialogs"]:
                refname = filename + ":" + d
                room_ids.add(d)
                dialog_ids.add(d)
                fname = data_folder + "dialogs/" + d + ".yaml"
                if not os.path.exists(fname):
                    error(fname + " does not exist")
                else:
                    subdata = load_yaml(fname)
                    if test(subdata, "lines"):
                        for l in subdata["lines"]:
                            if "line" in l:
                                if test(l, "line", is_array, 2):
                                    if l["line"][0] not in character_ids:
                                        error("non-existing character " + l["line"][0])

        integer_ids = set()
        if "integers" in data:
            for i in data["integers"]:
                refname = filename + ":integers"
                if test(i, "id"):
                    refname = filename + ":" + i["id"]
                    room_ids.add(i["id"])
                    integer_ids.add(i["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, i["id"], ref_ids)
                test(i, "value", is_int)

        music_ids = set()
        if "musics" in data:
            for m in data["musics"]:
                refname = filename + ":musics"
                if test(m, "id"):
                    refname = filename + ":" + m["id"]
                    room_ids.add(m["id"])
                    music_ids.add(m["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, m["id"], ref_ids)
                if "states" in m:
                    all_states[m["id"]] = set()
                    for s in m["states"]:
                        if test(s, "id"):
                            all_states[m["id"]].add(s["id"])
                        test(s, "sound", file_exists, ["sounds/musics", "ogg"])
                else:
                    test(m, "sound", file_exists, ["sounds/musics", "ogg"])

        object_ids = inventory_ids.copy()
        if "objects" in data:
            for o in data["objects"]:
                refname = filename + ":" + o
                room_ids.add(o)
                object_ids.add(o)
                fname = data_folder + "objects/" + o + ".yaml"
                if not os.path.exists(fname):
                    error(fname + " does not exist")

        scenery_ids = set()
        if "scenery" in data:
            for s in data["scenery"]:
                refname = filename + ":scenery"
                if test(s, "id"):
                    refname = filename + ":" + s["id"]
                    room_ids.add(s["id"])
                    scenery_ids.add(s["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, s["id"], ref_ids)
                    if "states" in s:
                        all_states[s["id"]] = set()
                        for st in s["states"]:
                            if test(st, "id"):
                                all_states[s["id"]].add(st["id"])
                                if "skin" in st:
                                    test(st, "skin", file_exists, ["images/scenery", "png"])
                    else:
                        test(s, "skin", file_exists, ["images/scenery", "png"])
                test(s, "coordinates/0", is_int)
                test(s, "coordinates/1", is_int)
                test(s, "coordinates/2", is_int)

        sound_ids = set()
        if "sounds" in data:
            for s in data["sounds"]:
                refname = filename + ":sounds"
                if test(s, "id"):
                    refname = filename + ":" + s["id"]
                    room_ids.add(s["id"])
                    sound_ids.add(s["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, s["id"], ref_ids)
                test(s, "sound", file_exists, ["sounds/effects", "ogg"])

        text_ids = set()
        if "texts" in data:
            for t in data["texts"]:
                if test(t, "id"):
                    refname = filename + ":" + t["id"]
                    room_ids.add(t["id"])
                    text_ids.add(t["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, t["id"], ref_ids)
                test(t, "text", is_line)
                if "coordinates" in t:
                    test(t, "coordinates/0", is_int)
                    test(t, "coordinates/1", is_int)
                    test(t, "coordinates/2", is_int)

        if "codes" in data:
            for c in data["codes"]:
                refname = filename + ":" + c
                room_ids.add(c)
                fname = data_folder + "codes/" + c + ".yaml"
                if not os.path.exists(fname):
                    error(fname + " does not exist")
                else:
                    subdata = load_yaml(fname)
                    test(subdata, "on_success", test_action)

        if "windows" in data:
            for w in data["windows"]:
                refname = filename + ":windows"
                if test(w, "id"):
                    refname = filename + ":" + w["id"]
                    room_ids.add(w["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, w["id"], ref_ids)
                test(w, "skin", file_exists, ["images/windows", "png"])

        if "integers" in data:
            for i in data["integers"]:
                refname = filename + ":integers"
                if test(i, "id"):
                    refname = filename + ":" + i["id"]
                if test(i, "triggers", is_array):
                    for t in i["triggers"]:
                        if test(t, "value"):
                            if not is_convertible_to_int(t["value"]) and t["value"] != "default":
                                error("value is neither int nor default (" + t["value"] + ")")
                        if "effect" in t:
                            test(t, "effect", test_action)

        if "actions" in data:
            for a in data["actions"]:
                refname = filename + ":actions"
                if "id" in a:
                    refname = filename + ":" + a["id"]
                else:
                    refname = filename + ":" + a
                    fname = data_folder + "actions/" + a + ".yaml"
                    a = load_yaml(fname)
                if "states" in a:
                    for s in a["states"]:
                        test(s, "effect", test_action)
                else:
                    test(a, "effect", test_action)

        for o in object_ids:
            refname = filename + ":" + o
            fname = data_folder + "objects/" + o + ".yaml"
            subdata = load_yaml(fname)
            if subdata:
                for act in [ "look", "primary", "secondary", "inventory", "use"]:
                    if act in subdata:
                        a = subdata[act]
                        if isinstance(a, list):
                            for ai in a:
                                if act == "inventory":
                                    if test(ai, "object"):
                                        if ai["object"] not in object_ids:
                                            error(o + "/" + act + " refers to non-existing object " + ai["object"])
                                else:
                                    if test(ai, "state"):
                                        if ai["state"] not in all_states[o]:
                                            error(o + "/" + act + " refers to non-existing (or non-reachable) state " + ai["state"])
                                if "effect" in ai:
                                    test(ai, "effect", test_action)
                        else:
                            if "effect" in a:
                                test(a, "effect", test_action)

        if "hints" in data:
            pass
    elif filename == "init.yaml":
        test(data, "version")
        test(data, "locale", is_array)
        test(data, "name", is_line)
        test(data, "icon", file_exists, ["images/interface", "png"])
        test(data, "cursor/0", file_exists, ["images/interface", "png"])
        test(data, "cursor/1", file_exists, ["images/interface", "png"])
        test(data, "cursor/2", file_exists, ["images/interface", "png"])
        test(data, "cursor/3", file_exists, ["images/interface", "png"])
        test(data, "loading_spin/0", file_exists, ["images/interface", "png"])
        test(data, "loading_spin/1", is_int)
        test(data, "debug_font", file_exists, ["fonts", "ttf"])
        test(data, "interface_font", file_exists, ["fonts", "ttf"])
        test(data, "interface_light_font", file_exists, ["fonts", "ttf"])
        test(data, "dialog_font", file_exists, ["fonts", "ttf"])
        test(data, "inventory_arrows/0", file_exists, ["images/interface", "png"])
        test(data, "inventory_arrows/1", file_exists, ["images/interface", "png"])
        test(data, "inventory_chamfer", file_exists, ["images/interface", "png"])
        test(data, "click_sound", file_exists, ["sounds/effects", "ogg"])
        test(data, "circle/0", file_exists, ["images/interface", "png"])
        test(data, "circle/1", file_exists, ["images/interface", "png"])
        test(data, "circle/2", file_exists, ["images/interface", "png"])
        test(data, "menu_background", file_exists, ["images/interface", "png"])
        test(data, "menu_logo", file_exists, ["images/interface", "png"])
        test(data, "menu_arrows/0", file_exists, ["images/interface", "png"])
        test(data, "menu_arrows/1", file_exists, ["images/interface", "png"])
        test(data, "menu_buttons/0", file_exists, ["images/interface", "png"])
        test(data, "menu_buttons/1", file_exists, ["images/interface", "png"])
        test(data, "menu_oknotok/0", file_exists, ["images/interface", "png"])
        test(data, "menu_oknotok/1", file_exists, ["images/interface", "png"])
        test(data, "load/0", file_exists, ["data/rooms", "yaml"])
        test(data, "load/1") # TODO: check action
        if test(data, "text", is_array):
            for t in data["text"]:
                test(t, "id")
                test(t, "value", is_line)
                if "icon" in t:
                    test(t, "icon", file_exists, ["images/interface", "png"])

        for act in ["look", "move", "take", "inventory_button", "inventory", "use",
                    "combine", "goto"]:
            test(data, "default/" + act + "/label", is_line)
            if "effect" in data["default"][act]:
                for e in data["default"][act]["effect"]:
                    test(e, "talk/0", is_line)





for root, directories, filenames in os.walk(root_folder):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)
        if ext == ".graph" or "en_US" in basename or ext == ".yaml":
            continue
        if fullname not in accessed_files:
            refname = '/'.join(fullname.split('/')[-3:]);
            warning("unused file")

if not errors:
    print("Data is valid")
else:
    print("Data may be invalid:")
    for e in errors:
        if e[0]:
            print("Error: [" + e[1] + "] " + e[2])
        else:
            print("Warning: [" + e[1] + "] " + e[2])
