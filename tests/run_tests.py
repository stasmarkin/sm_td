#!/usr/bin/env python3

import unittest
import sys
import os

# Add the parent directory to the Python path
project_root = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, project_root)

# Import the test module
from tests import test_complex_layout

if __name__ == "__main__":
    unittest.main(module=test_complex_layout)
