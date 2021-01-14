#CXX := c++
#CXX := clang++
CXX := xcrun g++-10
DBG_FLAGS := -g -D DEBUG
REL_FLAGS := -O2
CMN_FLAGS := -std=c++17
#CMN_FLAGS := -std=c++20

BINON_DIR := ../..
CPP17_DIR := ${BINON_DIR}/cpp17
HEADERS_DIR := ${CPP17_DIR}/headers

BASE_FLAGS := -c -I${HEADERS_DIR} ${CMN_FLAGS}
LD_FLAGS := -L/usr/local/lib #-ltbb

all: debug release

debug: ${CPP17_DIR}/makefile
	${MAKE} -C ${CPP17_DIR} CXX="${CXX}" \
		DBG_FLAGS="${DBG_FLAGS}" CMN_FLAGS=${CMN_FLAGS} debug
	${MAKE} DEST_DIR="build/debug" FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" target
release: ${CPP17_DIR}/makefile
	${MAKE} -C ${CPP17_DIR} CXX="${CXX}" \
		REL_FLAGS="${REL_FLAGS}" CMN_FLAGS=${CMN_FLAGS} release
	${MAKE} DEST_DIR="build/release" FLAGS="${BASE_FLAGS} ${REL_FLAGS}" target
clean:
	rm -rfv build

#	If this makefile is modified, trigger binon to rebuild by touching its
#	makefile also. This is done in case you altered some compiler settings that
#	should propogate back to the binon library.
${CPP17_DIR}/makefile: makefile
	touch ${CPP17_DIR}/makefile

target:
	${MAKE} TEST_DIR=${DEST_DIR}/bin ${DEST_DIR}/bin
	${MAKE} TEST_DIR=${DEST_DIR}/obj ${DEST_DIR}/obj
	${MAKE} ${DEST_DIR}/bin/example

${TEST_DIR}:
	mkdir -p ${TEST_DIR}

SRC_NAMES := example
OBJ := ${DEST_DIR}/obj/
OBJS := ${addprefix ${OBJ},${SRC_NAMES}}
OBJS := ${addsuffix .o,${OBJS}}
LIBBINON := ${CPP17_DIR}/${DEST_DIR}/lib/libbinon.a

${DEST_DIR}/bin/example: ${OBJS}
	${CXX} ${OBJS}  ${LIBBINON} ${LD_FLAGS} ${LD_TGT_FLAGS} \
	-o ${DEST_DIR}/bin/example

${OBJ}example.o: example.cpp makefile ${LIBBINON}
	${CXX} ${FLAGS} example.cpp -o ${OBJ}example.o
