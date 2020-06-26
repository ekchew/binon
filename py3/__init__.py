from pathlib import Path

py3Dir = Path(__file__).parent
for path in py3Dir.iterdir():
	modName = path.stem
	if modName.endswith("codec") and modName != "codec":
		exec(f"from . import {modName}")
