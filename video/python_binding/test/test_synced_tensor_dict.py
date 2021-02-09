import torch
import cyy_naive_cpp_extension


def test_synced_tensor_dict():
    tensor_dict = cyy_naive_cpp_extension.data_structure.SyncedTensorDict("")
    tensor_dict.set_in_memory_number(10)
    tensor_dict.set_storage_dir("tensor_dict_dir")
    # tensor_dict.set_permanent_storage()
    for i in range(100):
        tensor_dict[str(i)] = torch.Tensor([i])

    tensor_dict.prefetch([str(i) for i in range(100)])
    assert str(0) in tensor_dict
    for i in tensor_dict.keys():
        assert tensor_dict[i] == torch.Tensor([int(i)])
