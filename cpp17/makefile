CXX ?= c++
DBG_FLAGS ?= -g -D DEBUG
REL_FLAGS ?= -O2
CMN_FLAGS ?= -std=c++17

BASE_FLAGS := -c -Iheaders ${CMN_FLAGS}

all: debug release

debug:
	${MAKE} DEST_DIR="build/debug" FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" target
release:
	${MAKE} DEST_DIR="build/release" FLAGS="${BASE_FLAGS} ${REL_FLAGS}" target
clean:
	rm -rfv build

target:
	${MAKE} TEST_DIR=${DEST_DIR}/obj ${DEST_DIR}/obj
	${MAKE} TEST_DIR=${DEST_DIR}/lib ${DEST_DIR}/lib
	${MAKE} ${DEST_DIR}/lib/libbinon.a

${TEST_DIR}:
	mkdir -p ${TEST_DIR}

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
OBJ := ${DEST_DIR}/obj/
OBJS := ${addprefix ${OBJ},${SRC_NAMES}}
OBJS := ${addsuffix .o,${OBJS}}

${DEST_DIR}/lib/libbinon.a: ${OBJS} ${HDR}binon.hpp
	touch ${HDR}binon.hpp
	ar -crs ${DEST_DIR}/lib/libbinon.a ${OBJS}

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

${OBJ}binonobj.o: ${SRC}binonobj.cpp ${DEP4}
	${CXX} ${FLAGS} ${SRC}binonobj.cpp -o ${OBJ}binonobj.o
${OBJ}boolobj.o: ${SRC}boolobj.cpp ${HDR}boolobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}boolobj.cpp -o ${OBJ}boolobj.o
${OBJ}bufferobj.o: ${SRC}bufferobj.cpp ${HDR}bufferobj.hpp \
	${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}bufferobj.cpp -o ${OBJ}bufferobj.o
${OBJ}byteutil.o: ${SRC}byteutil.cpp ${HDR}literals.hpp ${DEP1}
	${CXX} ${FLAGS} ${SRC}byteutil.cpp -o ${OBJ}byteutil.o
${OBJ}codebyte.o: ${SRC}codebyte.cpp ${DEP2}
	${CXX} ${FLAGS} ${SRC}codebyte.cpp -o ${OBJ}codebyte.o
${OBJ}dictobj.o: ${SRC}dictobj.cpp ${DEP4}
	${CXX} ${FLAGS} ${SRC}dictobj.cpp -o ${OBJ}dictobj.o
${OBJ}intobj.o: ${SRC}intobj.cpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}intobj.cpp -o ${OBJ}intobj.o
${OBJ}ioutil.o: ${SRC}ioutil.cpp ${HDR}ioutil.hpp ${DEP0}
	${CXX} ${FLAGS} ${SRC}ioutil.cpp -o ${OBJ}ioutil.o
${OBJ}listobj.o: ${SRC}listobj.cpp ${DEP4}
	${CXX} ${FLAGS} ${SRC}listobj.cpp -o ${OBJ}listobj.o
${OBJ}strobj.o: ${SRC}strobj.cpp ${HDR}strobj.hpp ${HDR}intobj.hpp ${DEP3}
	${CXX} ${FLAGS} ${SRC}strobj.cpp -o ${OBJ}strobj.o
