CXX ?= c++
DBG_FLAGS ?= -g
REL_FLAGS ?= -O2
CMN_FLAGS ?= -std=c++17

BASE_FLAGS := -c -I headers -${CMN_FLAGS}

#	You can see that in addition to the usual all, debug, and release targets,
#	there are also all_target, debug_target, and release_target. The latter do
#	not check whether or not the output lib and obj directories exist.
#
#	If you are building binon directly from the command line, you should go
#	with the regular targets that do not end in _target.
#
#	If you are building a separate project that also makes binon, it may be
#	best to target mk_dirs first, followed by the relevant _target target. This
#	will ensure that the directories are only made once. (It can be a problem
#	when you use make -j# that it may try to build the same directories several
#	times simultaneously if you are not careful.) Look at the example project's
#	makefile to see how this can be done.

all: mk_dirs
	${MAKE} all_target
all_target: debug_target release_target

debug: mk_dirs
	${MAKE} debug_target
debug_target:
	${MAKE} lib/libbinon_dbg.a FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" SFX="_dbg"

release: mk_dirs
	${MAKE} release_target
release_target:
	${MAKE} lib/libbinon.a FLAGS="${BASE_FLAGS} ${REL_FLAGS}" SFX=""

mk_dirs: lib obj

clean:
	rm -frv lib
	rm -frv obj

lib:
	mkdir lib
obj:
	mkdir obj

HDR := headers/binon/
SRC := source/
SRC_NAMES := \
	boolobj \
	binonobj \
	bufferobj \
	byteutil \
	codebyte \
	dictobj \
	intobj \
	ioutil \
	listobj \
	strobj
OBJS := ${addprefix obj/,${SRC_NAMES}}
OBJS := ${addsuffix ${SFX}.o,${OBJS}}

lib/libbinon${SFX}.a: ${HDR}binon.hpp ${OBJS}
	ar -crs lib/libbinon${SFX}.a ${OBJS}

DEP0 := ${HDR}macros.hpp makefile
DEP1 := ${HDR}byteutil.hpp ${HDR}ioutil.hpp ${DEP0}
DEP2 := ${HDR}codebyte.hpp ${HDR}literals.hpp ${DEP1}
DEP3 := \
	${HDR}binonobj.hpp \
	${HDR}crtp.hpp \
	${HDR}floattypes.hpp \
	${HDR}hashutil.hpp \
	${DEP2}
DEP4 := \
	${HDR}boolobj.hpp \
	${HDR}bufferobj.hpp \
	${HDR}dictobj.hpp \
	${HDR}intobj.hpp \
	${HDR}floatobj.hpp \
	${HDR}listobj.hpp \
	${HDR}nullobj.hpp \
	${HDR}strobj.hpp \
	${DEP3}
${HDR}binon.hpp: ${DEP4}
	touch ${HDR}binon.hpp

obj/binonobj${SFX}.o: ${SRC}binonobj.cpp ${DEP4}
	${CXX} ${FLAGS} ${SRC}binonobj.cpp -o obj/binonobj${SFX}.o
obj/boolobj${SFX}.o: ${SRC}boolobj.cpp ${HDR}boolobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}boolobj.cpp -o obj/boolobj${SFX}.o
obj/bufferobj${SFX}.o: ${SRC}bufferobj.cpp ${HDR}bufferobj.hpp \
	${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}bufferobj.cpp -o obj/bufferobj${SFX}.o
obj/byteutil${SFX}.o: ${SRC}byteutil.cpp ${HDR}literals.hpp ${DEP1}
	${CXX} ${FLAGS} ${SRC}byteutil.cpp -o obj/byteutil${SFX}.o
obj/codebyte${SFX}.o: ${SRC}codebyte.cpp ${DEP2}
	${CXX} ${FLAGS} ${SRC}codebyte.cpp -o obj/codebyte${SFX}.o
obj/dictobj${SFX}.o: ${SRC}dictobj.cpp ${HDR}dictobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}dictobj.cpp -o obj/dictobj${SFX}.o
obj/intobj${SFX}.o: ${SRC}intobj.cpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}intobj.cpp -o obj/intobj${SFX}.o
obj/ioutil${SFX}.o: ${SRC}ioutil.cpp ${HDR}ioutil.hpp ${DEP0}
	${CXX} ${FLAGS} ${SRC}ioutil.cpp -o obj/ioutil${SFX}.o
obj/listobj${SFX}.o: ${SRC}listobj.cpp ${HDR}listobj.hpp \
	${HDR}boolobj.hpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}listobj.cpp -o obj/listobj${SFX}.o
obj/strobj${SFX}.o: ${SRC}strobj.cpp ${HDR}strobj.hpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}strobj.cpp -o obj/strobj${SFX}.o
