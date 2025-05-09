import sys
import numpy as np

content = """
DIMENSIONS 256 256
RAY_DEPTH 6
SAMPLES 32

BG_COLOR 0 0 0

CAMERA_POSITION 0 0 15
CAMERA_RIGHT 1 0 0
CAMERA_UP 0 1 0
CAMERA_FORWARD 0 0 -1
CAMERA_FOV_X 0.927295218

NEW_PRIMITIVE
PLANE 0 1 0
POSITION 0 -5 0
COLOR 1 1 1

NEW_PRIMITIVE
PLANE 0 0 1
POSITION 0 0 -5
COLOR 1 1 1

NEW_PRIMITIVE
PLANE 0 -1 0
POSITION 0 5 0
COLOR 1 1 1

NEW_PRIMITIVE
PLANE 1 0 0
POSITION -5 0 0
COLOR 1 0.25 0.25

NEW_PRIMITIVE
PLANE -1 0 0
POSITION 5 0 0
COLOR 0.25 1 0.25

NEW_PRIMITIVE
BOX 2 0.1 2
POSITION 0 5 0
EMISSION 2 2 2

"""

BOX_TEMPLATE = """
NEW_PRIMITIVE
BOX 0.2 0.2 0.2
COLOR 0.5 0.5 0.0
POSITION {} {} {}
"""

N = 50
positions = np.random.uniform(-4.5, 4.5, size=(N, 3))
for i in range(N):
    content += BOX_TEMPLATE.format(*positions[i])

with open(sys.argv[1], "w+") as f:
    f.write(content)
