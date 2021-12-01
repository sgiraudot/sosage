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
        errors.append([refname, string])

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

def file_exists(key, value, args):
    fname = args[0] + "/" + value + "." + args[1]
    if not os.path.exists(root_folder + "/" + fname):
        error(key + " refers to a non-existing file (" + fname + ")")

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

def test_action(key, value):
    for v in value:
        action = next(iter(v))
        args = v[action]
        if not isinstance(args, list):
            error(key + " uses ill-formed arguments: " + str(args))
            continue
        if action == "add":
            if check_signature(key, "add", args, ["string", "string"]):
                id = args[0]
                if is_convertible_to_int(args[1]):
                    if id not in integer_ids:
                        error(key + " uses function add on non-existing integer " + id)
                elif args[1] not in action_ids:
                    error(key + " uses function add on non-existing action " + args[1])
        elif action == "camera":
            if len(args) == 0:
                error(key + " uses function add without arguments")
                continue
            option = args[0]
            if option == "fadein" or option == "fadeout":
                check_signature(key, "camera/fade", args, ["string", "float"])
            elif option == "shake":
                check_signature(key, "camera/shake", args, ["string", "float", "float"])
            elif option == "target":
                if len(args) == 4:
                    check_signature(key, "camera/target", args, ["string", "int", "int", "int"])
                else:
                    check_signature(key, "camera/target", args, ["string", "int"])
            else:
                error(key + " uses function add with unrecognized option " + str(option))
        elif action == "dialog":
            if check_signature(key, "dialog", args, ["string"]):
                id = args[0]
                if id not in dialog_ids:
                    error(key + " uses function dialog on non-existing dialog " + id)
        elif action == "goto":
            if len(args) == 0:
                pass
            elif len(args) == 1:
                id = args[0]
                if id not in object_ids and id not in character_ids:
                    error(key + " uses function goto on non-existing (or non-reachable) id " + id)
            elif len(args) == 2:
                check_signature(key, "goto", args, ["int", "int"])
            elif len(args) == 3:
                if check_signature(key, "goto", args, ["string", "int", "int"]):
                    id = args[0]
                    if id not in character_ids:
                        error(key + " uses function goto on non-existing character " + id)
            else:
                error(key + " uses function goto with unhandled #arg = " + str(len(args)))
        elif action == "look":
            if len(args) == 0:
                pass
            elif check_signature(key, "look", args, ["string"]):
                id = args[0]
                if id not in object_ids and id not in character_ids:
                    error(key + " uses function look on non-existing (or non-reachable) id " + id)
        elif action == "play":
            if len(args) == 0:
                error(key + " uses function play without arguments")
                continue
            option = args[0]
            if option == "animation":
                if len(args) == 2:
                    if check_signature(key, "play/animation", args, ["string", "string"]):
                        id = args[1]
                        if id not in animation_ids:
                            error(key + " uses function play on non-existing animation " + id)
                else:
                    if check_signature(key, "play/animation", args, ["string", "string", "float"]):
                        id = args[1]
                        # TODO check if valid animation of character?
            elif option == "music":
                if check_signature(key, "play/music", args, ["string", "string"]):
                    id = args[1]
                    if id not in music_ids:
                        error(key + " uses function play on non-existing music " + id)
            elif option == "sound":
                if check_signature(key, "play/sound", args, ["string", "string"]):
                    id = args[1]
                    if id not in sound_ids:
                        error(key + " uses function play on non-existing sound " + id)
            else:
                error(key + " uses function play with unrecognized option " + str(option))
        elif action == "set":
            if len(args) == 0:
                error(key + " uses function set without arguments")
                continue
            option = args[0]
            if option == "coordinates":
                if len(args) == 6:
                    if check_signature(key, "set/coordinates", args, ["string", "string", "int", "int", "int", "int"]):
                        id = args[1]
                        if id not in object_ids and id not in scenery_ids:
                            error(key + " uses function set on non-existing (or non-reachable) id " + id)
                else:
                    if check_signature(key, "set/coordinates", args, ["string", "string", "int", "int", "int"]):
                        id = args[1]
                        if id not in object_ids and id not in scenery_ids:
                            error(key + " uses function set on non-existing (or non-reachable) id " + id)
            elif option == "state":
                if len(args) == 3:
                    if check_signature(key, "set/state", args, ["string", "string", "string"]):
                        id = args[1]
                        state0 = args[2]
                        if id not in room_ids:
                            error(key + " uses function set on non-existing (or non-reachable) id " + id)
                        if state0 not in all_states[id]:
                            error(key + " uses function set/state on non-existing state " + state0 + " of " + id)
                else:
                    if check_signature(key, "set/state", args, ["string", "string", "string", "string"]):
                        id = args[1]
                        if id not in room_ids:
                            error(key + " uses function set on non-existing (or non-reachable) id " + id)
                        state0 = args[2]
                        state1 = args[3]
                        if state0 not in all_states[id]:
                            error(key + " uses function set/state on non-existing state " + state0 + " of " + id)
                        if state1 not in all_states[id]:
                            error(key + " uses function set/state on non-existing state " + state1 + " of " + id)
            elif option == "follower":
                if check_signature(key, "set/follower", args, ["string", "string"]):
                    id = args[1]
                    if id not in character_ids:
                        error(key + " uses function set/follower on non-existing (or non-reachable) id " + id)
            elif option == "visible":
                if check_signature(key, "set/visible", args, ["string", "string"]):
                    id = args[1]
                    if id not in room_ids and id not in hints_ids:
                        error(key + " uses function set/visible on non-existing id " + id)
            elif option == "hidden":
                if check_signature(key, "set/hidden", args, ["string", "string"]):
                    id = args[1]
                    if id not in room_ids and id not in hints_ids:
                        error(key + " uses function set/hidden on non-existing id " + id)
            else:
                error(key + " uses function set with unrecognized option " + str(option))
        elif action == "stop":
            if len(args) == 0:
                error(key + " uses function stop without arguments")
                continue
            option = args[0]
            if option == "animation":
                if check_signature(key, "stop/animation", args, ["string", "string"]):
                    id = args[1]
                    if id not in animation_ids:
                        error(key + " uses function stop on non-existing animation " + id)
            elif option == "music":
                check_signature(key, "stop/music", args, ["string"])
            else:
                error(key + " uses function stop with unrecognized option " + str(option))
        elif action == "system":
            if len(args) == 0:
                error(key + " uses function system without arguments")
                continue
            option = args[0]
            if option == "load":
                if check_signature(key, "system/load", args, ["string", "string", "string"]):
                    # TODO check entry point
                    pass
            elif option == "lock":
                check_signature(key, "system/lock", args, ["string"])
            elif option == "hints":
                check_signature(key, "system/hints", args, ["string"])
            elif option == "trigger":
                if check_signature(key, "system/trigger", args, ["string", "string"]):
                    id = args[1]
                    if id not in action_ids:
                        error(key + " uses function system/trigger on non-existing id " + id)
            elif option == "menu":
                check_signature(key, "system/unlock", args, ["string", "string"])
                # TODO check menu existence
            elif option == "unlock":
                check_signature(key, "system/unlock", args, ["string"])
            elif option == "wait":
                if len(args) == 1:
                    check_signature(key, "system/wait", args, ["string"])
                else:
                    check_signature(key, "system/wait", args, ["string", "float"])
            elif option == "exit":
                check_signature(key, "system/exit", args, ["string"])
            else:
                error(key + " uses function system with unrecognized option " + str(option))
        elif action == "talk":
            if len(args) == 1:
                pass
            else:
                if check_signature(key, "talk", args, ["string", "string"]):
                    id = args[0]
                    if id not in character_ids and id != "superflu":
                        error(key + " uses function talk on non-existing character " + id)
        else:
            error(key + " contains unknown action " + action)
            continue


data_folder = root_folder + "/data/"
yaml_files = []

for root, directories, filenames in os.walk(data_folder):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)
        if ext != ".yaml":
            print("Warning: non yaml file found: " + fullname)
            continue
        relname = fullname.split("data/")[-1]
        yaml_files.append(relname)


yaml_files.sort()

all_ids = set()
ref_ids = {}
inventory_ids = set()
all_states = {}
hints_ids = set()

data = load_yaml(data_folder + "hints.yaml")
if test(data, "hints"):
    for h in data["hints"]:
        if test(h, "id"):
            all_ids, hints_id = test_id_unicity(hints_ids, h["id"])
        test(h, "question")
        test(h, "answer")

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
        test(data, "name")
        test(data, "coordinates/0", is_int)
        test(data, "coordinates/1", is_int)
        test(data, "looking_right", is_bool)
        test(data, "color", is_color)
        test(data, "mouth/skin", file_exists, ["images/characters", "png"])
        test(data, "mouth/dx_right", is_int)
        test(data, "mouth/dx_left", is_int)
        test(data, "mouth/dy", is_int)
        test(data, "head/skin")
        test(data, "head/size", is_int)
        test(data, "head/dx_right", is_int)
        test(data, "head/dx_left", is_int)
        test(data, "head/dy", is_int)
        test(data, "walk/skin", file_exists, ["images/characters", "png"])
        test(data, "idle/skin", file_exists, ["images/characters", "png"])
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
    elif filename.startswith("cutscenes/"):
        test(data, "name")
        if test(data, "content"):
            ids = set()
            for s in data["content"]:
                if test(s, "id"):
                    ids, _ = test_id_unicity(ids, s["id"])
                if "skin" in s:
                    test(s, "skin", file_exists, ["images/cutscenes", "png"])
                elif "music" in s:
                    test(s, "music", file_exists, ["sounds/musics", "ogg"])
                if "keyframes" in s:
                    for k in s["keyframes"]:
                        test(k, "time", is_int)
                        test(k, "coordinates", is_array, 3)
                if "loop" in s:
                    test(s, "loop", is_bool)
                if "frames" in s:
                    if test(s, "length", is_array, 2):
                        test(s, "length/0", is_int)
                        test(s, "length/1", is_int)
                        nb_frames = s["length"][0] * s["length"][1]
                        test(s, "frames", is_array)
                        for f in s["frames"]:
                            if type(f) is not int:
                                if 'x' not in f:
                                    error(s["id"] + " has a non-integer frame (" + str(f) + ")")
                                    continue
                                else:
                                    ff = f.split('x')
                                    if len(ff) != 2:
                                        error(s["id"] + " has a ill-formed frame (" + str(f) + ")")
                                    if int(ff[1]) >= nb_frames:
                                           error(s["id"] + " has out-of-bound frame index " + str(f) + "/" + str(nb_frames))
                            elif f >= nb_frames:
                                error(s["id"] + " has out-of-bound frame index " + str(f) + "/" + str(nb_frames))
        if test(data, "timeline"):
            ids_on = set()
            for t in data["timeline"]:
                test(t, "time", is_time)
                if "begin" in t:
                    frame_type = "begin"
                elif "end" in t:
                    frame_type = "end"
                elif "load" in t:
                    # TODO: test if entry point exists
                    continue
                else:
                    error("frame is neither begin nor end")
                    continue
                items = t[frame_type]
                if not isinstance(items, list):
                    items = [items]
                for i in items:
                    if frame_type == "begin":
                        if i in ids_on:
                            error(i + " has already begun")
                        else:
                            ids_on.add(i)
                    else:
                        if i not in ids_on:
                            error(i + " has not begun")
                        else:
                            ids_on.remove(i)
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
                        test(c, "line")
                        test(c, "once", is_bool)
                        if test(c, "goto"):
                            if c["goto"] not in ids:
                                error("goto refers to invalid id " + str(c["goto"]))
                            if c["goto"] == "end":
                                has_end = True
                elif "line" in l:
                    test(l, "line", is_array, 2)
                elif "target" not in l:
                    if test(l, "goto"):
                        if l["goto"] not in ids:
                            error("goto refers to invalid id " + str(c["goto"]))
                        if l["goto"] == "end":
                            has_end = True
            if not has_end:
                error("dialog has no end")
                
    elif filename.startswith("objects/"):
        test(data, "name")
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
                if sid == "none":
                    pass
                elif sid != "inventory":
                    inventory_only = False
                elif "skin" in s:
                    if sid == "inventory":
                        test(s, "skin", file_exists, ["images/inventory", "png"])
                        inventory_ids.add(current_id)
                    else:
                        test(s, "skin", file_exists, ["images/objects", "png"])
                else:
                    test(s, "size/0", is_int)
                    test(s, "size/1", is_int)
                if "frames" in s:
                    test(s, "frames", is_int)
                    test(s, "duration", is_int)

        if not inventory_only:
            test(data, "label/0", is_int)
            test(data, "label/1", is_int)
            
    elif filename.startswith("rooms/"):
        test(data, "name")
        test(data, "background", file_exists, ["images/backgrounds", "png"])
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
                    
                if test(a, "states"):
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
                test(a, "skin", file_exists, ["images/animations/", "png"])
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
                                if st["id"] != "none":
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

        if "origins" in data:
            for o in data["origins"]:
                refname = filename + ":origins"
                if test(o, "id"):
                    refname = filename + ":" + o["id"]
                    room_ids.add(o["id"])
                    all_ids, ref_ids = test_id_unicity(all_ids, o["id"], ref_ids)
                test(o, "coordinates/0", is_int)
                test(o, "coordinates/1", is_int)
                test(o, "looking_right", is_bool)
                if "action" in o:
                    test(o, "action", test_action)

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
                if test(a, "states"):
                    for s in a["states"]:
                        test(s, "effect", test_action)

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
    else: # init file
        pass

if not errors:
    print("Data is valid")
else:
    print("Data is invalid, some errors found:")
    for e in errors:
        print("[" + e[0] + "] " + e[1])
