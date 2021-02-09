import cyy_naive_cpp_extension


def test_ffpmeg_video_reader_dict():
    reader = cyy_naive_cpp_extension.video.FFmpegVideoReader()
    reader.open("test_images/1.jpg")
    print(reader.get_frame_rate())
    res = reader.next_frame()
    assert res[0] == 0
    assert res[1].seq == 0
    # print(dir(res[1].seq))
    # print(res[1].content)
    # # print(res)
