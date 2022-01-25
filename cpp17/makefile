CXX ?= c++
DBG_FLAGS ?= -g -D DEBUG
REL_FLAGS ?= -O2
CMN_FLAGS ?= -std=c++17

BASE_FLAGS := -c -Iheaders ${CMN_FLAGS}
HDR := headers/binon/

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

${TEST_DIR}: ${HDR}/binon.hpp
	mkdir -p ${TEST_DIR}

SRC := source/
SRC_NAMES := \
	boolobj \
	binonobj \
	bufferobj \
	byteutil \
	codebyte \
	dictobj \
	floatobj \
	intobj \
	ioutil \
	listobj \
	packelems \
	strobj \
	varobj
OBJ := ${DEST_DIR}/obj/
OBJS := ${addprefix ${OBJ},${SRC_NAMES}}
OBJS := ${addsuffix .o,${OBJS}}

${DEST_DIR}/lib/libbinon.a: ${OBJS}
	ar -crs ${DEST_DIR}/lib/libbinon.a ${OBJS}

DEPS := \
	${HDR}binonobj.hpp \
	${HDR}boolobj.hpp \
	${HDR}bufferobj.hpp \
	${HDR}byteutil.hpp \
	${HDR}codebyte.hpp \
	${HDR}crtp.hpp \
	${HDR}dictobj.hpp \
	${HDR}floatobj.hpp \
	${HDR}floattypes.hpp \
	${HDR}generator.hpp \
	${HDR}hashutil.hpp \
	${HDR}hystr.hpp \
	${HDR}idgen.hpp \
	${HDR}intobj.hpp \
	${HDR}ioutil.hpp \
	${HDR}listobj.hpp \
	${HDR}literals.hpp \
	${HDR}macros.hpp \
	${HDR}mixins.hpp \
	${HDR}nullobj.hpp \
	${HDR}optutil.hpp \
	${HDR}pbinonobjt.hpp \
	${HDR}packelems.hpp \
	${HDR}refutil.hpp \
	${HDR}strobj.hpp \
	${HDR}typeconv.hpp \
	${HDR}typeinfo.hpp \
	${HDR}varobj.hpp \
	makefile
${HDR}/binon.hpp: ${DEPS}
	touch ${HDR}/binon.hpp

${OBJ}binonobj.o: ${SRC}binonobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}binonobj.cpp -o ${OBJ}binonobj.o
${OBJ}boolobj.o: ${SRC}boolobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}boolobj.cpp -o ${OBJ}boolobj.o
${OBJ}bufferobj.o: ${SRC}bufferobj.cpp ${HDR}bufferobj.hpp \
	${HDR}intobj.hpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}bufferobj.cpp -o ${OBJ}bufferobj.o
${OBJ}byteutil.o: ${SRC}byteutil.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}byteutil.cpp -o ${OBJ}byteutil.o
${OBJ}codebyte.o: ${SRC}codebyte.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}codebyte.cpp -o ${OBJ}codebyte.o
${OBJ}dictobj.o: ${SRC}dictobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}dictobj.cpp -o ${OBJ}dictobj.o
${OBJ}floatobj.o: ${SRC}floatobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}floatobj.cpp -o ${OBJ}floatobj.o
${OBJ}intobj.o: ${SRC}intobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}intobj.cpp -o ${OBJ}intobj.o
${OBJ}ioutil.o: ${SRC}ioutil.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}ioutil.cpp -o ${OBJ}ioutil.o
${OBJ}listobj.o: ${SRC}listobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}listobj.cpp -o ${OBJ}listobj.o
${OBJ}packelems.o: ${SRC}packelems.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}packelems.cpp -o ${OBJ}packelems.o
${OBJ}strobj.o: ${SRC}strobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}strobj.cpp -o ${OBJ}strobj.o
${OBJ}varobj.o: ${SRC}varobj.cpp ${DEPS}
	${CXX} ${FLAGS} ${SRC}varobj.cpp -o ${OBJ}varobj.o
