import mxslc
from data_utils import get_data_path, assert_matches_groundtruth

def test_compile_signatures():    
    result = mxslc.compile_file_to_string(get_data_path("signatures.mxsl"))
    assert_matches_groundtruth(result, "signatures.mtlx")

