#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Tests for C++ module onesided.core.
"""

import sys
sys.path.insert(0,'.')

import mpicts.core
import pytest


def test_6():
    ok = mpicts.core.test6()
    print(f"ok = {ok}")
    assert ok

def test_7():
    ok = mpicts.core.test7()
    print(f"ok = {ok}")
    assert ok


#===============================================================================
# The code below is for debugging a particular test in eclipse/pydev.
# (normally all tests are run with pytest)
#===============================================================================
if __name__ == "__main__":
    print("------------------------------------------------------------")
    the_test_you_want_to_debug = test_7
    print(f"__main__ running {the_test_you_want_to_debug} ...")
    the_test_you_want_to_debug()
    print("------------------------------------------------------------")
    print('-*# finished #*-')
#===============================================================================
