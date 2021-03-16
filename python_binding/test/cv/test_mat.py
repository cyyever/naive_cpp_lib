import os

import cv2
import cyy_naive_cpp_extension


def test_ffpmeg_video_reader_dict():
    image_file = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        "..",
        "..",
        "..",
        "cv",
        "test",
        "test_images",
        "1.jpg",
    )
    img = cv2.imread(image_file)
    mat = cyy_naive_cpp_extension.cv.Mat(img)
    print(mat.MSSIM(mat))
