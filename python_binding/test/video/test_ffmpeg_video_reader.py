import cv2
import cyy_naive_cpp_extension
import numpy as np


def test_ffpmeg_video_reader_dict():
    reader = cyy_naive_cpp_extension.video.FFmpegVideoReader()
    reader.open("test_images/1.jpg")
    print(reader.get_frame_rate())
    reader.drop_non_key_frames()
    res = reader.next_frame()
    assert res[0] == 1
    assert res[1].seq == 0
    assert res[1].is_key
    np_array = np.array(res[1].content, copy=False)
    cv2.imwrite("a.jpg", np_array)
