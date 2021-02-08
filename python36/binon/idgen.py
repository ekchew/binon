import contextlib, threading

class IDGen:
	"""
	This is a thread-safe class that generates unsigned integer IDs you can use
	for any purpose. In the context of BinON, the advantage of IDGen over a
	simple counter for generating unique IDs comes from the fact that BinON has
	a tighter encoding for smaller integers that larger ones, and IDGen tries to
	keep them small by recycling IDs that are no longer in use.

	Notably, IDGen tries to keep IDs in the range [1,127] as much as possible.
	That's because the UInt data format can encode numbers < 128 in only a
	single byte. (The value 0 is kept off limits to leave room for a special
	kNoID code should you need it.)

	If more than 127 IDs are needed simulataneously, extra "overflow" IDs will
	be generated beginning at 128. These IDs are not tracked for re-use as
	rigorously as the regular ones, though they should nevertheless still be
	released when you are done.

	IDGen is not implemented as a classic Python generator. It is not an
	iterable class. Rather, you can call its acquire() and release() methods
	manually to access IDs or use the NewID() context manager, which ensures
	automatic release of your ID as you leave the context.

	Example:

		>>> gen = binon.IDGen()
		>>> with binon.NewID(gen) as idA:
				with binon.NewID(gen) as idB:
					print(idA, idB, end=" ")
				with binon.NewID(gen) as idC:
					print(idC)
			1 2 2

	You can see here that idC reuses 2 since idB no longer needs it.

	Class Attributes:
		kNoID (int): the value 0 IDGen avoids generating
	"""

	kNoID = 0

	def __init__(self):
		"""
		IDGen() takes no args, though its __init__() method does initialize
		internal state.
		"""

		#	__freeIDs is a bytearray accessed as a stack of unused IDs in the
		#	regular (non-overflow) ID space. As such, we initialize it with
		#	integers running from 127 down to 1.
		self.__freeIDs = bytearray(range(0x7f, 0, -1))

		#	There are 2 integer attributes that manage the overflow space.
		#	__oflwID gives a tentative value to the next overflow ID to be
		#	generated. __oflwID starts at 128 keeps incrementing each time it
		#	is used until __oflwCnt, which tracks how many overflow IDs are
		#	currently in play, drops to 0. At that point, __oflwID can be reset
		#	to 128. (It's a cruder mechanism than __freeIDs to be sure, but
		#	does not require substantial storage.)
		self.__oflwID = 0x80
		self.__oflwCnt = 0

		#	All access to the other attributes is guarded by this mutex to
		#	ensure thread-safety, since IDs often have a global scope.
		self.__mutex = threading.Lock()
	def __repr__(self):
		sFreeIDs = ",".join(map(str, self.__freeIDs))
		sFreeIDs = f"freeIDs=bytearray([{sFreeIDs}])"
		sOflwID = f"oflwID={self.__oflwID}"
		sOflwCnt = f"oflwCnt={self.__oflwCnt}"
		sMutex = f"mutex={self.__mutex}"
		sArgs = ", ".join((sFreeIDs, sOflwID, sOflwCnt, sMutex))
		return f"IDGen({sArgs})"
	def acquire(self):
		"""
		If you prefer not to use an iterator, you can acquire new IDs by calling
		this method.

		Returns:
			int: the next unused ID
		"""
		with self.__mutex:

			#	The __freeIDs bytearray is treated like a stack of unused IDs.
			#	(In Python 3, these should pop off as int.)
			if self.__freeIDs:
				theID = self.__freeIDs.pop()

			#	If the __freeIDs are exhausted, we need to start grabbing them
			#	from the overflow space.
			else:

				#	If there are no overflow IDs in play at the moment, we can
				#	reset the __oflwID counter to 128 (0x80).
				if self.__oflwCnt == 0:
					self.__oflwID = 0x80

				#	Now grab the next overflow ID and increment both __oflwID
				#	and __oflwCnt for the next time we need one.
				theID = self.__oflwID
				self.__oflwID += 1
				self.__oflwCnt += 1

		return theID
	def release(self, theID):
		"""
		Once you are done with an ID, call release() to recycle it.

		WARNING:
			IDGen makes no assertions that an ID has not been released already.
			It is up to you to track your ID life cycles lest the uniqueness
			logic break down.

		You can use the NewID() context manager instead if you'd rather avoid
		having to call release() manually.

		Args:
			int: the ID to discard
		"""
		with self.__mutex:

			#	If theID is in the regular space, we can push it back onto the
			#	__freeIDs stack to be re-used the next time an ID is needed.
			if theID < 0x80:
				self.__freeIDs.append(theID)

			#	If theID is in the overflow space instead, we simply decrement
			#	the __oflwCnt counter. (Once this drops to 0, self.__oflwID can
			#	be reset to 128, but this is left up to the acquire() method in
			#	a lazy-evaluation sort of way.)
			else:
				self.__oflwCnt -= 1

@contextlib.contextmanager
def NewID(idGen):
	"""
	This is a context manager which acquires a new ID for you and then releases
	it automatically as you leave the context.

	Args:
		idGen (IDGen): an ID generator

	Enters with:
		int: a new ID
	"""
	theID = idGen.acquire()
	try:
		yield theID
	finally:
		idGen.release(theID)
