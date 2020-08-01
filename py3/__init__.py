"""
This implementation of BinON requires Python 3.6 or later.
"""

from pathlib import Path

#	Importing the ...Codec classes registers them to the BinON class.
py3Dir = Path(__file__).parent
for path in py3Dir.iterdir():
	modName = path.stem
	if modName.endswith("codec") and modName != "binoncodec":
		exec(f"from . import {modName}")
