#!/usr/bin/env python

import argparse, os, re, sys
from pathlib import Path

class DocGen:
	gHeaderExts = set(("h", "hh", "hpp", "h++"))

	@classmethod
	def FromCommandLine(cls, args=None):
		ap = argparse.ArgumentParser(
			description="Generates C++ BinON-style documentation"
		)
		ap.add_argument(
			"-i", "--input-dir", default="",
			help="""
				Path to headers directory
				(current working directory by default)
				"""
		)
		ap.add_argument(
			"-o", "--output-dir", default="",
			help="""
				Path to output html directory ("doc" subdirectory
				within current working directory by default)
				"""
		)
		pa = ap.parse_args(args=args)
		return cls(inDir=pa.input_dir, outDir=pa.output_dir)
	@classmethod
	def IterDirFiles(cls, inDir, fileExts=gHeaderExts):
		for entry in os.scandir(inDir):
			if entry.is_dir():
				for path in cls.IterDirFiles(entry.path, fileExts):
					yield Path(entry.name, path)
			else:
				name = entry.name
				pos = name.rfind(".")
				if pos >= 0 and name[pos+1:] in fileExts:
					yield Path(name)

	def __init__(self, inDir=None, outDir=None):
		cwd = Path.cwd()
		if not inDir:
			inDir = cwd
		if not outDir:
			outDir = Path(cwd, "doc")
		self.inDir = inDir
		self.outDir = outDir
	def __repr__(self):
		attrs = ("inDir", "outDir")
		kwargs = (f"{a} = {getattr(self,a)!r}" for a in attrs)
		return "DocGen({})".format(", ".join(kwargs))
	def run(self):
		for symbol in self._iterSymbols():
			print(symbol)
	def _iterSymbols(self):
		rx = re.compile(r"/\*{2}\s*([^\n]+)")
		for path in self.IterDirFiles(self.inDir):
			fullPath = Path(self.inDir, path)
			with open(fullPath) as inF:
				code = inF.read()
			pos = 0
			while True:
				res = rx.search(code, pos)
				if not res:
					break
				yield str(path), res.group(1)
				pos = res.end()

if __name__ == "__main__":
	try:
		DocGen.FromCommandLine().run()
	except Exception as err:
		print("ERROR:", err, file=sys.stderr)
		sys.exit(1)

