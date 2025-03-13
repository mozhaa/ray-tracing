import sys

import numpy as np
from PIL import Image


def diff(x, y):
    return np.linalg.norm(x - y)


image1 = np.asarray(Image.open(sys.argv[1]).convert("RGB")) / 255
image2 = np.asarray(Image.open(sys.argv[2]).convert("RGB")) / 255

w, h, _ = image1.shape

total_error = 0
total_wrong_pixels = 0
for i in range(w):
    for j in range(h):
        error = diff(image1[i, j], image2[i, j])
        total_error += error
        if error > 0:
            total_wrong_pixels += 1

print(f"Total error:\t\t{total_error}")
print(f"Wrong pixels:\t\t{total_wrong_pixels / w / h * 100:.3f}%")
