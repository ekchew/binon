CXX := c++
DBG_FLAGS ?= -g
REL_FLAGS ?= -O2
CMN_FLAGS := -std=c++17
BINON_PATH := ../..

CPP17_DIR := ${BINON_PATH}/cpp17
HEADERS_DIR := ${CPP17_DIR}/headers
BASE_FLAGS := -c -I${HEADERS_DIR} ${CMN_FLAGS}
LD_FLAGS := -L/usr/local/lib -ltbb

all: debug release
debug:
	${MAKE} target FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" \
		TARGET="debug" SFX="_dbg" LD_TGT_FLAGS="-g"
release:
	${MAKE} target FLAGS="${BASE_FLAGS} ${REL_FLAGS}" \
		TARGET="release" SFX="" LD_TGT_FLAGS=""
clean:
	${MAKE} -C ${CPP17_DIR} clean
	rm -fv bin/*
	rm -fv obj/*

target: makeBinON bin/example${SFX}
makeBinON: ${CPP17_DIR}/makefile
	${MAKE} -C ${CPP17_DIR} ${TARGET} CXX=${CXX} CMN_FLAGS=${CMN_FLAGS}
${CPP17_DIR}/makefile: makefile
	${MAKE} -C ${CPP17_DIR} clean
	touch ${CPP17_DIR}/makefile

CXX_OBJS := obj/example${SFX}.o ${CPP17_DIR}/lib/libbinon${SFX}.a
bin/example${SFX}: bin obj ${CXX_OBJS}
	${CXX} ${CXX_OBJS} ${LD_FLAGS} ${LD_TGT_FLAGS} -o bin/example${SFX}

bin:
	mkdir bin
obj:
	mkdir obj

../lib/libbinon${SFX}.a:
../headers/binon.hpp:

obj/example${SFX}.o: example.cpp makefile ${HEADERS_DIR}/binon/binon.hpp
	${CXX} ${FLAGS} example.cpp -o obj/example${SFX}.o
