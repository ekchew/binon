from . import reprutil

class Context:
	def __init__(self, parent=None, symbols=None):
		self.parent = parent
		self.symbols = symbols or set(())
	def __repr__(self):
		return MakeReprStr(kwargs=("parent","symbols"))
