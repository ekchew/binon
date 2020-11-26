"""
This implementation of BinON requires Python 3.6 or later.
"""

from pathlib import Path

#	Importing the ...Obj classes registers them to the BinON class.
py3Dir = Path(__file__).parent
for path in py3Dir.iterdir():
	modName = path.stem
	if modName.endswith("obj") and modName != "binonobj":
		exec(f"from . import {modName}")
