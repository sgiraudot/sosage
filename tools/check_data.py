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
exit_at_first_error = False

clean_unused = False
if len(sys.argv) > 2 and sys.argv[2] == '-c':
    clean_unused = True

def error(string):
    print("[Error in " + refname + "] " + string)

def warning(string):
    print("[Warning in " + refname + "] " + string)

def missing_translation(locale, string):
    return
    print("[Missing " + locale + " translation in " + refname + "] " + string)

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

def is_convertible_to_bool(string):
    return type(string) is bool or string == "true" or string == "false"

def child(data, key):
    if is_convertible_to_int(key):
        idx = int(key)
        if idx < len(data):
            return data[idx]
    if key == "":
        return data
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
register_access = True

def file_exists(key, value, args):
    fname = args[0] + "/" + value + "." + args[1]
    if not os.path.exists(root_folder + "/" + fname):
        error(key + " refers to a non-existing file (" + fname + ")")
        return False
    if register_access:
#        print("Register " + fname)
        accessed_files.add(root_folder + "/" + fname)
    return True

def test_id_unicity(ids, ref, new_id):
    if new_id in ids:
        if ref is None:
            error("multiple definitions of " + new_id)
        else:
            error(new_id + " already used in " + ref[new_id])
    else:
        ids.add(new_id)
        if not refname.startswith("dialogs/") and new_id[0].isupper():
            error(new_id + " has uppercase first letter, reserved for system IDs")
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
    if ' ' in line:
        warning("forbidden symbol ' ' (small NBSP) found in line " + line)
    for locale in locales:
        if line not in translation[locale]:
            missing_translation(locale, '"' + line + '"')

def is_line(key, value):
    check_line(value)

current_stated = ""
current_room = ""
current_music = ""
is_num = False
def is_action_id(tested_id):
    return tested_id in items["actions"]
def is_animation_id(tested_id):
    return tested_id in items["animations"]
def is_character_id(tested_id):
    if tested_id == "Hinter":
        return True
    if is_num:
        return True # Too complicated to test
    elif inventory_step:
        return tested_id in all_items["characters"]
    else:
        return tested_id in items["characters"]
def is_character_animation_id(tested_id):
    # Shouldn't be hardcoded...
    return tested_id in { "action", "telephone", "melange1", "melange2", "photo", "hareng" }
def is_dialog_id(tested_id):
    return tested_id in items["dialogs"]
def is_integer_id(tested_id):
    return tested_id in items["integers"]
def is_string_line(tested):
    if not isinstance(tested, str):
        return False
    check_line(tested)
    return True
def is_list_id(tested_id):
    return tested_id == "phone_numbers"
def is_lookable_id(tested_id):
    if inventory_step:
        return tested_id in all_items["characters"] or tested_id in all_items["objects"]
    else:
        return tested_id in set().union(items["objects"], items["characters"], items["animations"])
def is_menu_id(tested_id):
    return tested_id in { "End", "Exit" }
def is_movable_id(tested_id):
    return tested_id in set().union(items["objects"], items["scenery"], items["animations"], items["characters"])
def is_music_id(tested_id):
    global current_music
    current_music = tested_id
    return tested_id in items["musics"]
def is_music(tested_id):
    return tested_id == "music"
def is_player(tested_id):
    return tested_id == "Player"
def is_room_id(tested_id):
    global current_room
    current_room = tested_id
    return True
def queue_room(tested_id):
    global todo
    todo.append([current_room, tested_id])
    return True
def is_scalable_id(tested_id):
    return tested_id in set().union(items["objects"], items["scenery"], items["animations"])
received = {}
sent = {}
read = {}
def is_received_signal_id(tested_id):
    if tested_id not in received:
        received[tested_id] = set()
    received[tested_id].add(refname)
    return True
def is_read_signal_id(tested_id):
    if tested_id not in read:
        read[tested_id] = set()
    read[tested_id].add(refname)
    return True
def is_sent_signal_id(tested_id):
    if tested_id not in sent:
        sent[tested_id] = set()
    sent[tested_id].add(refname)
    return True
def is_showable_id(tested_id):
    if inventory_step:
        return tested_id in set().union(items["objects"], all_items["characters"], items["scenery"], items["texts"], items["windows"], items["codes"])
    else:
        return tested_id in set().union(items["objects"], items["characters"], items["scenery"], items["texts"], items["windows"], items["codes"])
def is_sound_id(tested_id):
    return tested_id in items["sounds"]
def is_source_id(tested_id):
    return current_music in sources and tested_id in sources[current_music]
def is_stated_id(tested_id):
    if is_num:
        return True # Too complicated to test
    global current_stated
    current_stated = tested_id
    if inventory_step:
        return tested_id in all_states
    else:
        return tested_id in states
def is_state_id(tested_id):
    if is_num:
        return True # Too complicated to test
    elif inventory_step:
        return current_stated in all_states and tested_id in all_states[current_stated]
    else:
        return current_stated in states and tested_id in states[current_stated]
def is_target_id(tested_id):
    return tested_id in items["objects"] or tested_id in items["characters"]
def is_text_id(tested_id):
    return tested_id in items["texts"]
current_timer = ""
def register_timer_id(tested_id):
    global current_timer
    current_timer = tested_id
    return True
def is_timer_id(tested_id):
    return tested_id == current_timer

possible_functions = [ [ "add", is_integer_id, is_convertible_to_int ],
                       [ "add", is_list_id, is_action_id ],
                       [ "camera", is_convertible_to_int ],
                       [ "camera", is_convertible_to_int, is_convertible_to_int ],
                       [ "camera", is_convertible_to_int, is_convertible_to_int, is_convertible_to_int ],
                       [ "control", is_character_id ],
                       [ "control", is_character_id, is_character_id ],
                       [ "cutscene" ],
                       [ "emit", is_sent_signal_id ],
                       [ "exit" ],
                       [ "fadein", is_convertible_to_float ],
                       [ "fadein", is_music_id, is_source_id, is_convertible_to_float ],
                       [ "fadeout", is_convertible_to_float ],
                       [ "fadeout", is_music_id, is_source_id, is_convertible_to_float ],
                       [ "goto" ],
                       [ "goto", is_target_id ],
                       [ "goto", is_convertible_to_int, is_convertible_to_int ],
                       [ "goto", is_character_id, is_target_id],
                       [ "goto", is_character_id, is_convertible_to_int, is_convertible_to_int ],
                       [ "hide", is_showable_id ],
                       [ "include" ],
                       [ "load", is_room_id, queue_room ],
                       [ "lock" ],
                       [ "look" ],
                       [ "look", is_lookable_id ],
                       [ "look", is_character_id, is_lookable_id ],
                       [ "loop" ],
                       [ "message", is_text_id ],
                       [ "move", is_character_id, is_convertible_to_int, is_convertible_to_int, is_convertible_to_bool ],
                       [ "move", is_character_id, is_convertible_to_int, is_convertible_to_int, is_convertible_to_int, is_convertible_to_bool ],
                       [ "move", is_movable_id, is_convertible_to_int, is_convertible_to_int, is_convertible_to_int ],
                       [ "move", is_movable_id, is_convertible_to_int, is_convertible_to_int, is_convertible_to_int, is_convertible_to_float ],
                       [ "move60fps", is_movable_id, is_convertible_to_int, is_convertible_to_int, is_convertible_to_int, is_convertible_to_float ],
                       [ "pause", is_animation_id ],
                       [ "play", is_animation_id ],
                       [ "play", is_music_id ],
                       [ "play", is_sound_id ],
                       [ "play", is_music_id, is_convertible_to_float ],
                       [ "play", is_character_animation_id, is_convertible_to_float ],
                       [ "play", is_character_id, is_character_animation_id, is_convertible_to_float ],
                       [ "randomize", is_stated_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id, is_state_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id ],
                       [ "randomize", is_stated_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id, is_state_id ],
                       [ "receive", is_received_signal_id ],
                       [ "receive", is_received_signal_id, is_action_id ],
                       [ "receive", is_received_signal_id, is_action_id, is_action_id ],
                       [ "remove", is_list_id, is_action_id ],
                       [ "rescale", is_scalable_id, is_convertible_to_float ],
                       [ "rescale", is_scalable_id, is_convertible_to_float, is_convertible_to_float ],
                       [ "rescale60fps", is_scalable_id, is_convertible_to_float, is_convertible_to_float ],
                       [ "set", is_stated_id, is_state_id ],
                       [ "set", is_stated_id, is_state_id, is_state_id ],
                       [ "set", is_integer_id, is_convertible_to_int ],
                       [ "set12fps", is_stated_id, is_state_id ],
                       [ "shake", is_convertible_to_float, is_convertible_to_float ],
                       [ "show", is_showable_id ],
                       [ "skip" ],
                       [ "stop", is_music ],
                       [ "stop", is_character_id ],
                       [ "stop", is_player ],
                       [ "stop", is_animation_id ],
                       [ "talk", is_string_line ],
                       [ "talk", is_character_id, is_string_line ],
                       [ "talk", is_character_id, is_string_line, is_convertible_to_float ],
                       [ "timer", register_timer_id ],
                       [ "trigger", is_action_id ],
                       [ "trigger", is_action_id, is_convertible_to_bool ],
                       [ "trigger", is_dialog_id ],
                       [ "trigger", is_menu_id ],
                       [ "unlock" ],
                       [ "wait" ],
                       [ "wait", is_convertible_to_float ],
                       [ "wait", is_timer_id, is_convertible_to_float ],
                       [ "zoom", is_convertible_to_float ] ]


def test_step(action, args):
    found = False
    candidates = []
    for func in possible_functions:
        if action != func[0]:
            continue
        if len(func) - 1 != len(args):
            continue
        candidates.append([func[0]])
        okay = True
        for a, t in zip(args, func[1:]):
            if not t:
                candidates[-1] += ["is_string(" + str(a) + ")"]
            elif t(a):
                candidates[-1] += [t.__name__ + "(" + str(a) + ")"]
            else:
                candidates[-1] += ["!" + t.__name__ + "(" + str(a) + ")"]
                okay = False
        if okay:
            found = True
            break

    if not found:
        if candidates:
            err_msg = action + str(args) + " is invalid, candidates are:"
            for c in candidates:
                err_msg += "\n  " + c[0] + ": [" + ", ".join(c[1:]) + "]"
        else:
            err_msg = action + str(args) + " is invalid"
        error(err_msg)

inventory_step = False

def test_action(init_value, has_inventory = False):
    global inventory_step
    inventory_step = has_inventory
    value = []
    for v in init_value:
        action = next(iter(v))
        args = v[action]
        if action == "func":
            for a in functions[args[0]]:
                laction = next(iter(a))
                largs = a[laction].copy()
                for idx, val in enumerate(largs):
                    if isinstance(val, str) and val.startswith("ARG"):
                        largs[idx] = args[1 + int(val.split('ARG')[1])]
                value.append({ laction: largs })
        else:
            value.append({ action: args })

    for v in value:
        action = next(iter(v))
        args = v[action]
        if not isinstance(args, list):
            error(action + " uses ill-formed arguments: " + str(args))
            continue
        test_step(action, args)

    global current_timer
    current_timer = ""


data_folder = root_folder + "/data/"
yaml_files = []
skip_list = { "ba_fochougny.yaml", "test_rendu.yaml" }

print("# LOADING YAML FILES")

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
        refname = filename
        current_id = filename.split('/', 1)[-1].split('.',1)[0]
        data = load_yaml(data_folder + relname)
        if not data:
            error("invalid YAML input")
            continue
        yaml_files.append(relname)

yaml_files.sort()

print("# TESTING ID UNICITY")

all_ids = set()
ref_ids = {}

sections = [ "musics", "actions", "animations", "characters", "codes", "dialogs", "integers",
             "objects", "scenery", "sounds", "texts", "windows" ]

def has_key(item, key):
    return isinstance(item, dict) and key in item

for filename in yaml_files:
    refname = filename
    current_id = filename.split('/', 1)[-1].split('.',1)[0]
    all_ids, ref_ids = test_id_unicity(all_ids, ref_ids, current_id)

    if not filename.startswith("rooms/"):
        continue

    data = load_yaml(data_folder + filename)

    for s in sections:
        if has_key(data, s):
            for item in data[s]:
                if has_key(item, "id"):
                    all_ids, ref_ids = test_id_unicity(all_ids, ref_ids, item["id"])

todo = []
functions = {}

print("# READING LOCALE")
data = load_yaml(data_folder + "locale.yaml")
locales = [ l["id"] for l in data["locales"] if l["id"] != 'fr_FR']

translation = { l: {} for l in locales }
for line in data["lines"]:
    for locale in locales:
        translation[locale][line['fr_FR']] = line[locale]

print("# TESTING INIT")
filename = "init.yaml"
refname = filename
current_id = filename.split('/', 1)[-1].split('.',1)[0]
data = load_yaml(data_folder + filename)
(data, "version")
test(data, "locale", is_array)
test(data, "name", is_line)
test(data, "icon", file_exists, ["images/interface", "png"])
test(data, "cursor/0", file_exists, ["images/interface", "png"])
test(data, "cursor/1", file_exists, ["images/interface", "png"])
test(data, "cursor/2", file_exists, ["images/interface", "png"])
test(data, "cursor/3", file_exists, ["images/interface", "png"])
test(data, "hand", file_exists, ["images/interface", "png"])
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
test(data, "step_sound", file_exists, ["sounds/effects", "ogg"])
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
if test(data, "load/0", file_exists, ["data/rooms", "yaml"]) and test(data, "load/1"):
    todo.append(data["load"])
if test(data, "functions", is_array):
    for f in data["functions"]:
        if test(f, "id") and test(f, "effect", is_array):
            functions[f["id"]] = f["effect"]
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


print("# GETTING GLOBAL ITEMS")
global_items = {}
global_states = {}
for s in sections:
    global_items[s] = set()
    if has_key(data, s):
        if test(data, s, is_array):
            for item in data[s]:
                global_items[s].add(item)

register_access = False


print("# TESTING INVENTORY OBJECTS")

inventory_objects = set()
for filename in yaml_files:
    refname = filename
    current_id = filename.split('/', 1)[-1].split('.',1)[0]
    if not filename.startswith("objects/"):
        continue
    data = load_yaml(data_folder + filename)
    if test(data, "states", is_array):
        for s in data["states"]:
            if test(s, "id") and s["id"].startswith("inventory"):
                inventory_objects.add(current_id)
                global_items["objects"].add(current_id)
                break

print("# GETTING ALL STATES")

def add_states(states, item_id, data):
    if "states" in data:
        for st in data["states"]:
            if test(st, "id"):
                if item_id not in states:
                    states[item_id] = set()
                states[item_id].add(st["id"])
    return states

def add_character_states(states, item_id, data):
    if "states" in data:
        for st in data["states"]:
            if item_id not in states:
                states[item_id] = set()
            states[item_id].add(st)
    else:
        states[item_id] = {"default"}
    return states

all_states = {}
all_items = {}
for s in sections:
    all_states[s] = set()
    all_items[s] = set()

for filename in yaml_files:
    if '/' not in filename:
        continue
    refname = filename
    section, current_id = filename.split('/', 1)
    current_id = current_id.split('.',1)[0]
    ldata = load_yaml(data_folder + filename)

    if section == "rooms":
        for s in sections:
            if has_key(ldata, s):
                for item in ldata[s]:
                    if not has_key(item, "id"):
                        continue
                    item_id = item["id"]
                    all_items[s].add(item_id)
                    if s == "characters":
                        all_states = add_character_states(all_states, item_id, item)
                    else:
                        all_states = add_states(all_states, item_id, item)
    else:
        all_items[section].add(current_id)
        if section == "characters":
            all_states = add_character_states(all_states, current_id, ldata)
        else:
            all_states = add_states(all_states, current_id, ldata)

print("# TESTING ALL ROOMS")
done = set()

def test_actions(data, has_inventory = False):
    global is_num
    if "label" in data: # Special actions, maybe tested later
        is_num = True
    if "states" in data:
        for s in data["states"]:
            test(s, "id")
            if test(s, "effect", is_array):
                test_action(s["effect"], has_inventory)
    else:
        test_action(data["effect"], has_inventory)
    is_num = False

def test_animations(data):
    test(data, "coordinates/0", is_int)
    test(data, "coordinates/1", is_int)
    test(data, "coordinates/2", is_int)
    test(data, "skin", file_exists, ["images/animations", "png"])
    if test(data, "length"):
        if isinstance(data["length"], list):
            test(data, "length/0", is_int)
            test(data, "length/1", is_int)
        else:
            test(data, "length", is_int)
    test(data, "loop", is_bool)
    if "duration" in data:
        test(data, "duration", is_int)
    if "frames" in data:
        test(data, "frames", is_array) # Test frames individually?

def test_characters(data):
    test(data, "name", is_line)
    test(data, "coordinates/0", is_int)
    test(data, "coordinates/1", is_int)
    if has_key(data, "label"):
        test(data, "label/0", is_int)
        test(data, "label/1", is_int)
    test(data, "looking_right", is_bool)
    test(data, "color", is_color)
    if "skin" in data:
        test(data, "skin/mouth/skin", file_exists, ["images/characters", "png"])
        test(data, "skin/mouth/dx_right", is_int)
        test(data, "skin/mouth/dx_left", is_int)
        test(data, "skin/mouth/dy", is_int)
        test(data, "skin/head/skin", file_exists, ["images/characters", "png"])
        test(data, "skin/head/dx_right", is_int)
        test(data, "skin/head/dx_left", is_int)
        test(data, "skin/head/dy", is_int)
        test(data, "skin/head/positions", is_array)
        if "walk" in data["skin"]:
            test(data, "skin/walk/skin", file_exists, ["images/characters", "png"])
        test(data, "skin/idle/skin", file_exists, ["images/characters", "png"])
        test(data, "skin/idle/positions", is_array)

    for act in possible_actions:
        if act in data:
            test_object_action(data[act], True)

def test_codes(data):
    if test(data, "states"):
        for s in data["states"]:
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

    if test(data, "on_success", is_array):
        test_action(data["on_success"])

def test_dialogs(data):
    if "end" in data:
        if data["end"] not in items["actions"]:
            error("dialog ends on non-existing action " + data["end"])
    if test(data, "lines"):
        ids = { "end" }
        has_end = False
        for l in data["lines"]:
            if "target" in l:
                ids, _ = test_id_unicity(ids, None, l["target"])
        for l in data["lines"]:
            if "choices" in l:
                for c in l["choices"]:
                    test(c, "line", is_line)
                    if "if" in c:
                        is_read_signal_id(c["if"])
                    if "unless" in c and c["unless"] != "said":
                        is_read_signal_id(c["unless"])
                    if test(c, "goto"):
                        if c["goto"] not in ids:
                            error("goto refers to invalid id " + str(c["goto"]))
                        if c["goto"] == "end":
                            has_end = True
            elif "line" in l:
                if test(l, "line", is_array):
                    test(l, "line/1", is_line)
                    if len(l["line"]) == 3:
                        is_sent_signal_id(l["line"][2])
            elif "target" not in l:
                if test(l, "goto"):
                    if l["goto"] not in ids:
                        error("goto refers to invalid id " + str(c["goto"]))
                    if l["goto"] == "end":
                        has_end = True
        if not has_end:
            error("dialog has no end")

def test_integers(data):
    test(data, "value", is_int)
    if test(data, "triggers"):
        for t in data["triggers"]:
            test(t, "value")
            if test(t, "effect", is_array):
                test_action(t["effect"])

possible_actions = { "look", "move", "take", "inventory", "use", "combine", "goto" }

def test_object_action(data, has_inventory):
    if isinstance(data, list):
        for act in data:
            test_object_action(act, has_inventory)
        return
    if "object" in data:
        if data["object"] not in inventory_objects:
            error("action using non-inventory object " + data["object"])
    if "state" in data:
        if data["state"] not in states[item_id]:
            error("action using non-existing state " + data["state"])
    if "right" in data:
        test(data, "right", is_bool)
    if "effect" in data:
        test_action(data["effect"], has_inventory)

def test_objects(data):
    test(data, "name", is_line)
    test(data, "coordinates/0", is_int)
    test(data, "coordinates/1", is_int)
    test(data, "coordinates/2", is_int)
    test(data, "box_collision", is_bool)
    test(data, "view/0", is_int)
    test(data, "view/1", is_int)

    has_inventory = False
    inventory_only = True
    if test(data, "states"):
        for s in data["states"]:
            if not test(s, "id"):
                continue
            sid = s["id"]
            if "skin" in s:
                if sid.startswith("inventory"):
                    test(s, "skin", file_exists, ["images/inventory", "png"])
                    has_inventory = True
                else:
                    inventory_only = False
                    test(s, "skin", file_exists, ["images/objects", "png"])
            elif "size" in s:
                if not sid.startswith("inventory"):
                    inventory_only = False
                test(s, "size/0", is_int)
                test(s, "size/1", is_int)
            elif "mask" in s:
                if not sid.startswith("inventory"):
                    inventory_only = False
                test(s, "mask", file_exists, ["images/masks", "png"])
            if "frames" in s:
                test(s, "frames", is_int)
                test(s, "duration", is_int)

    if not inventory_only:
        test(data, "label/0", is_int)
        test(data, "label/1", is_int)

    for act in possible_actions:
        if act in data:
            test_object_action(data[act], has_inventory)

sources = {}

def test_musics(data):
    sources[item_id] = set()
    nb_tracks = 0
    if test(data, "tracks", is_array):
        for t in data["tracks"]:
            test(t, "", file_exists, ["sounds/musics", "ogg"])
            nb_tracks += 1
    if test(data, "sources", is_array):
        idx = 0
        for s in data["sources"]:
            if test(s, "id"):
                sources[item_id].add(s["id"])
            test(s, "mix", is_array, nb_tracks)

def test_scenery(data):
    if "states" in data:
        for st in data["states"]:
            if test(st, "id"):
                if "skin" in st:
                    test(st, "skin", file_exists, ["images/scenery", "png"])
    else:
        test(data, "skin", file_exists, ["images/scenery", "png"])

def test_sounds(data):
    test(data, "sound", file_exists, ["sounds/effects", "ogg"])

def test_texts(data):
    test(data, "text", is_line)
    if "coordinates" in data:
        test(data, "coordinates/0", is_int)
        test(data, "coordinates/1", is_int)
        test(data, "coordinates/2", is_int)

def test_windows(data):
    test(data, "skin", file_exists, ["images/windows", "png"])


tests = {}
for s in sections:
    tests[s] = locals()["test_" + s]

def get_item(section, item):
    global refname
    if has_key(item, "id"):
        refname = filename + ":" + section + ":" + item["id"]
        return item["id"], item
    else:
        rname = section + "/" + item + ".yaml"
        fname = data_folder + rname
        refname = rname
        done.add(rname)
        if not os.path.exists(fname):
            error(fname + " does not exist")
            return None, None
        ldata = load_yaml(fname)
        if register_access:
#            print("Register " + fname)
            accessed_files.add(fname)
        return item, ldata

global_states = {}
for section, items in global_items.items():
    for item in items:
        if register_access:
#            print("Register " + fname)
            accessed_files.add(data_folder + section + "/" + item + ".yaml")
        global_states = add_states(global_states, item, load_yaml(data_folder + section + "/" + item + ".yaml"))

register_access = True

while todo:
    filename = "rooms/" + todo[0][0] + ".yaml"
    refname = filename
    if not file_exists("load", todo[0][0], ["data/rooms", "yaml"]):
        todo = todo[1:]
        continue

    filename = "rooms/" + todo[0][0] + ".yaml"
    refname = filename
    action = todo[0][1]
    todo = todo[1:]

    data = load_yaml(data_folder + filename)

    action_found = False
    if test(data, "actions", is_array):
        for a in data["actions"]:
            item_id, ldata = get_item("actions", a)
            if not item_id:
                continue
            if item_id == action:
                action_found = True
                break
    refname = filename

    if not action_found:
        error("room does not have action " + action)

    if refname in done:
        continue

    print("## TESTING ROOM " + refname)

    done.add(refname)

    test(data, "name", is_line)
    if "background" in data:
        test(data, "background", file_exists, ["images/backgrounds", "png"])
    if "ground_map" in data:
        if isinstance(data["ground_map"], list):
            ground_map = data["ground_map"]
            if ground_map[0]:
                test(ground_map, "0", file_exists, ["images/backgrounds", "png"])
            test(ground_map, "1", file_exists, ["images/backgrounds", "png"])
        else:
            test(data, "ground_map", file_exists, ["images/backgrounds", "png"])
        test(data, "front_z", is_int)
        test(data, "back_z", is_int)

    items = {}
    for section, it in global_items.items():
        items[section] = it.copy()
    states = global_states.copy()

    for s in sections:
        if has_key(data, s):
            for item in data[s]:
                item_id, ldata = get_item(s, item)
                if not item_id:
                    continue
                items[s].add(item_id)
                if s == "characters":
                    states = add_character_states(states, item_id, ldata)
                else:
                    states = add_states(states, item_id, ldata)
    refname = filename

    for s in sections:
        if has_key(data, s):
            for item in data[s]:
                item_id, ldata = get_item(s, item)
                if not item_id:
                    continue
                tests[s](ldata)

register_access = False

for section, it in global_items.items():
    for item_id in it:
        refname = section + "/" + item_id + ".yaml"
        tests[section](load_yaml(data_folder + refname))

received_or_read = set().union(set(received.keys()), set(sent.keys()))
not_received = set(sent.keys()).difference(received_or_read)
for signal in not_received:
    refname = next(iter(sent[signal]))
    warning("signal " + signal + " sent but never received")
not_sent = received_or_read.difference(set(sent.keys()))
for signal in not_sent:
    if signal in received:
        refname = next(iter(received[signal]))
    else:
        refname = next(iter(read[signal]))
    warning("signal " + signal + " received but never sent")

all_signals = sorted(list(set().union(received_or_read, set(sent.keys()))))
display_signals = True
osignals = open('signals_sumup.log', 'w')
for signal in all_signals:
    if signal in not_received:
        osignals.write("*NOT RECEIVED SIGNAL " + signal + '\n')
    elif signal in not_sent:
        osignals.write("*NOT SENT SIGNAL " + signal + '\n')
    else:
        osignals.write("SIGNAL " + signal + '\n')
    if signal in sent:
        for ref in sent[signal]:
            osignals.write("  >> Sent from   " + ref + '\n')
    if signal in read:
        for ref in read[signal]:
            osignals.write("  == Read in     " + ref + '\n')
    if signal in received:
        for ref in received[signal]:
            osignals.write("  << Received in " + ref + '\n')
    osignals.write('\n')
osignals.close()

to_clean = []
for root, directories, filenames in os.walk(root_folder):
    for filename in filenames:
        fullname = os.path.join(root, filename)
        basename = os.path.basename(fullname)
        name, ext = os.path.splitext(basename)
        if basename in skip_list:
            continue
        if basename in {"locale.yaml", "init.yaml"}:
            continue
        if ext == ".graph" or "en_US" in basename or ".backup." in basename:
            continue
        if fullname not in accessed_files:
            to_clean.append(fullname)
            refname = '/'.join(fullname.split('/')[-3:]);
            warning("unused file")

safety = True

if clean_unused:
    if safety:
        print("Deactivate safety to remove files")
        exit()
    for file in to_clean:
        print('rm ' + file)
        out = subprocess.run('rm ' + file, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True, check=True)
