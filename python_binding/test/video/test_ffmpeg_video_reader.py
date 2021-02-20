import os

import cv2
import cyy_naive_cpp_extension
import numpy as np


def test_ffpmeg_video_reader_dict():
    reader = cyy_naive_cpp_extension.video.FFmpegVideoReader()
    video_file = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        "..",
        "..",
        "..",
        "video",
        "test",
        "test_video",
        "output.mp4",
    )
    reader.open(video_file)
    print(reader.get_frame_rate())
    reader.drop_non_key_frames()
    res = reader.next_frame()
    assert res[0] == 1
    assert res[1].seq == 1
    assert res[1].is_key

    np_array = np.array(res[1].content, copy=False)
    cv2.imwrite("a.jpg", np_array)
    reader.seek_frame(1)
    res = reader.next_frame()
    assert res[0] == 1
    assert res[1].seq == 1
    assert res[1].is_key
