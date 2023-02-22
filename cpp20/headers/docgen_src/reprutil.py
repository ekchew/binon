def MakeReprStr(obj, clsName, args=(), kwargs=(), xattrs=()):
	sArgs = ", ".join(f"{getattr(obj,a)!r}" for a in args)
	sKWArgs = ", ".join(f"{k}={getattr(obj,k)!r}" for k in kwargs)
	sRepr = f"{clsName}({', '.join((sArgs, sKWArgs))})"
	if xattrs:
		sXAttrs = ", ".join(f"{k}={getattr(obj,k)!r}" for k in xattrs)
		sRepr = f"<{sRepr},{sXAttrs}>"
	return sRepr
