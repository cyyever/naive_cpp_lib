import cyy_naive_cpp_extension


def test_ffpmeg_video_reader_dict():
    reader = cyy_naive_cpp_extension.video.FFmpegVideoReader()
    reader.open("test_images/1.jpg")
    print(reader.get_frame_rate())
    res = reader.next_frame()
    print(res)
