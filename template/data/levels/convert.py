from os import listdir
from os.path import dirname, abspath, isfile, join
import json

def convert(filepath):
    file = open(filepath)
    if file.mode == "r":
        # setup
        linenum = 1
        lines = file.read().splitlines()
        j = {}

        # get ambient light
        j["ambient_light"] = float(lines[0])

        # get next levels
        next_levels = []
        line = lines[linenum]
        while "enddoors" not in line:
            next_levels.append(line)
            linenum += 1
            line = lines[linenum]
        linenum += 1

        # get sign texts
        sign_texts = []
        line = lines[linenum]
        while "endsigns" not in line:
            sign_texts.append(line)
            linenum += 1
            line = lines[linenum]
        linenum += 1

        # get actual level data
        doors = []
        signs = []
        ghosts = []
        bricks = []

        sizey = 0
        sizex = 0

        for y in range(0, len(lines) - linenum):
            sizey = max(sizey, y)
            line = lines[y + linenum]
            for x in range(0, len(line)):
                sizex = max(sizex, x)
                obj = { "pos": { "x": x, "y": y } }
                char = line[x]
                if char is "D":
                    obj["next_level"] = next_levels[0]
                    next_levels.pop(0)
                    doors.append(obj)
                    continue
                if char is "S":
                    obj["text"] = sign_texts[0]
                    sign_texts.pop(0)
                    signs.append(obj)
                    continue
                if char is "G":
                    ghosts.append(obj)
                    continue
                if char is "R":
                    j["spawn"] = obj
                    continue
                if char is "B":
                    obj["colour"] = { "r": 1.0, "g": 1.0, "b": 1.0 }
                    bricks.append(obj)
                    continue
                if char is "C":
                    obj["colour"] = { "r": 1.0, "g": 0.0, "b": 0.0 }
                    bricks.append(obj)
                    continue
                if char is "M":
                    obj["colour"] = { "r": 0.0, "g": 1.0, "b": 0.0 }
                    bricks.append(obj)
                    continue
                if char is "N":
                    obj["colour"] = { "r": 0.0, "g": 0.0, "b": 1.0 }
                    bricks.append(obj)
                    continue
                if char is "Y":
                    obj["colour"] = { "r": 1.0, "g": 1.0, "b": 0.0 }
                    bricks.append(obj)
                    continue
                if char is "Z":
                    obj["colour"] = { "r": 1.0, "g": 0.0, "b": 1.0 }
                    bricks.append(obj)
                    continue
                if char is "L":
                    obj["colour"] = { "r": 0.0, "g": 1.0, "b": 1.0 }
                    bricks.append(obj)
                    continue

        j["size"] = { "width": sizex + 1, "height": sizey + 1}
        j["doors"] = doors
        j["signs"] = signs
        j["ghosts"] = ghosts
        j["bricks"] = bricks

        dirpath = dirname(abspath(__file__))
        filename = filepath.replace(dirpath, "")
        filename = filename.replace("\\", "")
        filename = filename.replace(".txt", ".json")
        writepath = join(dirpath, "json", filename)

        file = open(writepath, "w+")
        file.write(json.dumps(j))
        file.close()

def convertall():
    path = dirname(abspath(__file__))
    for f in listdir(path):
        filepath = join(path, f)
        if isfile(filepath) and ".txt" in f:
            convert(filepath)

if __name__ == "__main__":
    convertall()
