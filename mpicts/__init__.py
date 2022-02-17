# -*- coding: utf-8 -*-

"""
Package mpicts
=======================================

Top-level package for mpicts.
"""

__version__ = "1.0.1"

try:
    import mpicts.core_dyn
except ModuleNotFoundError as e:
    # Try to build this binary extension:
    from pathlib import Path
    import click
    from et_micc2.project import auto_build_binary_extension
    msg = auto_build_binary_extension(Path(__file__).parent, 'core_dyn')
    if not msg:
        import mpicts.core_dyn
    else:
        click.secho(msg, fg='bright_red')

# try:
#     import mpicts.core
# except ModuleNotFoundError as e:
#     # Try to build this binary extension:
#     from pathlib import Path
#     import click
#     from et_micc2.project import auto_build_binary_extension
#     msg = auto_build_binary_extension(Path(__file__).parent, 'core')
#     if not msg:
#         import mpicts.core
#     else:
#         click.secho(msg, fg='bright_red')
#

def hello(who='world'):
    """'Hello world' method.

    :param str who: whom to say hello to
    :returns: a string
    """
    result = "Hello " + who
    return result

# Your code here...