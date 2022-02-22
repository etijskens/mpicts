#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Tests for C++ module mpicts.core_dyn.
"""

import os
import sys
sys.path.insert(0,'.')

import numpy as np

import mpicts

# create an alias for the binary extension cpp module
cpp = mpicts.core_dyn


def test_MessageBuffer():
    cpp.test_MessageBuffer()

def test_MessageBufferPool():
    cpp.test_MessageBufferPool()

def test_MessageHeader():
    cpp.test_MessageHeader()

def test_MessageHandler():
    cpp.test_MessageHandler()

#===============================================================================
# The code below is for debugging a particular test in eclipse/pydev.
# (normally all tests are run with pytest)
#===============================================================================
if __name__ == "__main__":
    the_test_you_want_to_debug = test_MessageHandler

    print(f"__main__ running {the_test_you_want_to_debug} ...")
    the_test_you_want_to_debug()
    print('-*# finished #*-')
#===============================================================================
