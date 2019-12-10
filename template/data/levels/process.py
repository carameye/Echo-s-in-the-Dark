from os import listdir
from os.path import dirname, abspath, isfile, join
import json

from PIL import Image
from PIL import ImageFilter

from math import sqrt, copysign

def line_len(x1, y1, x2, y2):
    l = (x2 - x1)**2
    l += (y2 - y1)**2
    return sqrt(l)

def convert(filepath):
    print("Converting " + filepath)
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
        torches = []

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
                    obj["colour"] = { "r": 1.0, "g": 1.0, "b": 1.0 }
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
                if char is "I":
                    obj["colour"] = { "r": 0.0, "g": 0.0, "b": 0.0 }
                    bricks.append(obj)
                    continue
                if char is "Y":
                    obj["colour"] = { "r": 1.0, "g": 0.0, "b": 0.0 }
                    ghosts.append(obj)
                    continue
                if char is "Z":
                    obj["colour"] = { "r": 0.0, "g": 1.0, "b": 0.0 }
                    ghosts.append(obj)
                    continue
                if char is "L":
                    obj["colour"] = { "r": 0.0, "g": 0.0, "b": 1.0 }
                    ghosts.append(obj)
                    continue
                if char is "T":
                    torches.append(obj)
                    continue

        sizex += 1
        sizey += 1

        j["size"] = { "width": sizex, "height": sizey}
        j["doors"] = doors
        j["signs"] = signs
        j["ghosts"] = ghosts
        j["bricks"] = bricks
        j["torches"] = torches

        dirpath = dirname(abspath(__file__))
        filename = filepath.replace(dirpath, "")
        filename = filename.replace("\\", "")
        filename = filename.replace(".txt", "")
        writepath = "".join([dirpath, "/json/", filename, ".json"])
        print(writepath)

        file = open(writepath, "w+")
        file.write(json.dumps(j))
        file.close()

        sizex *= 64
        sizey *= 64

        print("    Loading bricks")

        brickimage = Image.new("RGB", (sizex, sizey), color = (255, 255, 255))
        brickpixels = brickimage.load()

        for b in bricks:
            startx = 64 * b["pos"]["x"]
            starty = 64 * b["pos"]["y"]
            for i in range(startx, startx + 64):
                for j in range(starty, starty + 64):
                    colour = b["colour"]
                    if colour["r"] == 1.0 and colour["g"] == 1.0 and colour["b"] == 1.0:
                        brickpixels[i, j] = (0, 0, 0)

        brickimage.save("".join([dirpath, "/shadow/", filename + "_brickmap.bmp"]))

def convertall():
    path = dirname(abspath(__file__))
    for f in listdir(path):
        filepath = join(path, f)
        if isfile(filepath) and ".txt" in f:
            convert(filepath)

if __name__ == "__main__":
    convertall()
