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

def inlight(x1, y1, x2, y2, bricks):
    if x1 is x2 and y1 is y2:
        return True

    sign = lambda x: copysign(1, x)

    dy = y2 - y1
    dx = x2 - x1

    x = x1
    y = y1

    if abs(dy) > abs(dx):
        if dy is 0:
            dx = 1
        else:
            dx = dx / abs(dy)
            dy = sign(dy)
    else:
        if dx is 0:
            dy = 1
        else:
            dy = dy / abs(dx)
            dx = sign(dx)

    while line_len(x1, y1, x, y) < line_len(x1, y1, x2, y2):

        if bricks[x, y][0] == 0:
            return False

        x += 10 * dx
        y += 10 * dy

    return True

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
        writepath = join(dirpath, "json", filename + ".json")

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
                    brickpixels[i, j] = (0, 0, 0)

        brickimage.save(join(dirpath, "shadow", filename + "_brickmap.png"))

        print("    Loading shadows")

        shadowimage = Image.new("RGB", (sizex, sizey), color = (0, 0, 0))
        shadowpixels = shadowimage.load()

        t_num = 1
        for t in torches:
            print("        Assessing torch " + str(t_num) + " out of " + str(len(torches)))
            t_num += 1
            x = 64 * t["pos"]["x"]
            y = 64 * t["pos"]["y"]
            for i in range(max(0, x - 384), min(sizex, x + 384)):
                for j in range(max(0, y - 384), min(sizey, y + 384)):
                    if shadowpixels[i, j] is (255, 255, 255):
                        continue;
                    if inlight(i, j, x, y, brickpixels):
                        shadowpixels[i, j] = (255, 255, 255)

        print("    Refining shadows")

        shadowimage = shadowimage.filter(ImageFilter.GaussianBlur(16))

        for i in range(0, sizex):
            for j in range(0, sizey):
                val = min(2 * shadowpixels[i, j][0], 255)
                shadowpixels[i, j] = (val, val, val)

        print("    Saving images")
        shadowimage.save(join(dirpath, "shadow", filename + "_shadowmap.png"))

def convertall():
    path = dirname(abspath(__file__))
    for f in listdir(path):
        filepath = join(path, f)
        if isfile(filepath) and ".txt" in f:
            convert(filepath)

if __name__ == "__main__":
    convertall()
