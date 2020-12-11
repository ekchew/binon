CXX ?= c++
DBG_FLAGS ?= -g
REL_FLAGS ?= -O2
CMN_FLAGS ?= -std=c++17

BASE_FLAGS := -c -I headers -${CMN_FLAGS}

all: debug release
clean:
	rm -frv lib
	rm -frv obj
debug:
	${MAKE} lib/libbinon_dbg.a FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" SFX="_dbg"
release:
	${MAKE} lib/libbinon.a FLAGS="${BASE_FLAGS} ${REL_FLAGS}" SFX=""

HDR := headers/binon/
SRC := source/
SRC_NAMES := \
	boolobj \
	binonobj \
	bufferobj \
	byteutil \
	codebyte \
	intobj \
	ioutil \
	listobj \
	strobj
OBJS := ${addprefix obj/,${SRC_NAMES}}
OBJS := ${addsuffix ${SFX}.o,${OBJS}}

lib/libbinon${SFX}.a: lib obj ${OBJS}
	ar -crs lib/libbinon${SFX}.a ${OBJS}

lib:
	mkdir lib
obj:
	mkdir obj

DEP0 := makefile
DEP1 := ${HDR}byteutil.hpp ${HDR}ioutil.hpp ${DEP0}
DEP2 := ${HDR}codebyte.hpp ${DEP1}
DEP3 := ${HDR}binonobj.hpp ${HDR}crtp.hpp ${HDR}floattypes.hpp ${DEP2}
DEP4 := \
	${HDR}boolobj.hpp \
	${HDR}bufferobj.hpp \
	${HDR}intobj.hpp \
	${HDR}floatobj.hpp \
	${HDR}listobj.hpp \
	${HDR}nullobj.hpp \
	${HDR}strobj.hpp \
	${DEP3}

obj/binonobj${SFX}.o: ${SRC}binonobj.cpp ${DEP4}
	${CXX} ${FLAGS} ${SRC}binonobj.cpp -o obj/binonobj${SFX}.o
obj/boolobj${SFX}.o: ${SRC}boolobj.cpp ${HDR}boolobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}boolobj.cpp -o obj/boolobj${SFX}.o
obj/bufferobj${SFX}.o: ${SRC}bufferobj.cpp ${HDR}bufferobj.hpp \
	${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}bufferobj.cpp -o obj/bufferobj${SFX}.o
obj/byteutil${SFX}.o: ${SRC}byteutil.cpp ${DEP1}
	${CXX} ${FLAGS} ${SRC}byteutil.cpp -o obj/byteutil${SFX}.o
obj/codebyte${SFX}.o: ${SRC}codebyte.cpp ${DEP2}
	${CXX} ${FLAGS} ${SRC}codebyte.cpp -o obj/codebyte${SFX}.o
obj/intobj${SFX}.o: ${SRC}intobj.cpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}intobj.cpp -o obj/intobj${SFX}.o
obj/ioutil${SFX}.o: ${SRC}ioutil.cpp ${HDR}ioutil.hpp ${DEP0}
	${CXX} ${FLAGS} ${SRC}ioutil.cpp -o obj/ioutil${SFX}.o
obj/listobj${SFX}.o: ${SRC}listobj.cpp ${HDR}listobj.hpp \
	${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}listobj.cpp -o obj/listobj${SFX}.o
obj/strobj${SFX}.o: ${SRC}strobj.cpp ${HDR}strobj.hpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}strobj.cpp -o obj/strobj${SFX}.o
