#!/usr/bin/env python3

import unittest
import sys
import os

# Add the tests directory to the Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'tests'))

# Import the test module
from tests import test_sm_td

if __name__ == "__main__":
    unittest.main(module=test_sm_td)
