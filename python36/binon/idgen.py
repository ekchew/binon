import threading

class IDGen:
	"""
	This is a thread-safe iterable class which generates ID numbers you can use
	for various purposes. You may be thinking why not use a simple counter which
	can generate as many unique IDs as you need? In the context of BinON,
	smaller numbers encode more tightly than larger ones, so IDGen tries to keep
	them small by recycling IDs that are no longer in use.

	An ID is an unsigned integer suitable for encoding as a UInt. IDGen tries to
	keep IDs in the range [1,127] since UInts require only a single data byte to
	encode such values. (The value 0 is typically kept off limits to leave room
	for a no-ID value should you need it.)

	If more than 127 IDs are in play simultaneously, extra "overflow" IDs will
	be generated beginning at 128. (Overflow IDs are not tracked for re-use as
	thoroughly as regular ones, though you should nevertheless discard them when
	you are done with them.)

	Example:

		>>> gen = binon.IDGen()
		>>> it = iter(gen)
		>>> idA = next(it)
		>>> idB = next(it)
		>>> idC = next(it)
		>>> gen.discard(idA)
		>>> idD = next(it)
		>>> print(idB, idC, idD)
		2 3 1

	Class Attributes:
		kNoID (int): the value 0 IDGen never generates by default
			(If you really want 0 to be an eligible ID, you can call discard(0)
			on an IDGen instance.)
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
	def __iter__(self):
		"""
		Returns:
			iterator of int: produces an infinite supply of unused IDs
		"""
		while True:
			yield self.acquire()
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
	def discard(self, theID):
		"""
		Once you are done with an ID, call discard() to recycle it.

		WARNING:
			IDGen makes no assertions that an ID has not been discarded already.
			It is up to you to track your ID life cycles lest the uniqueness
			logic break down.

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
