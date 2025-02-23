import os
import lit.formats
import lit.util

from pathlib import Path as _Path

config.name = "Lit tests for paraCL"
config.test_format = lit.formats.ShTest("0")
config.suffixes = {".test"}
config.test_source_root = os.path.dirname(__file__)

_directory = os.path.dirname(os.path.abspath(__file__)) 
pcl_lib_path = _directory + "/../lib/std_pcl_lib/"
config.test_source_root = _directory
_directory += "/../build/"
config.test_exec_root = _directory


config.substitutions += [("%paracl", os.path.join(config.test_exec_root, "paracl")),]
config.substitutions += [("%pcllib", os.path.join(pcl_lib_path, "pcllib.cpp")),]
print(pcl_lib_path)
