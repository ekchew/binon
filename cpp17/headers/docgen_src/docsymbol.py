class DocSymbol:
	def __init__(self, name, anchor, path):
		self.name = name
		self.anchor = anchor
		self.path = path
	def __repr__(self):
		attrs = ("name", "anchor", "path")
		kwargs = (f"{a} = {getattr(self,a)!r}" for a in attrs)
		return "DocSymbol({})".format(", ".join(kwargs))

