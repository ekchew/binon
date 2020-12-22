#CXX := c++
CXX := xcrun g++-10
DBG_FLAGS := -g
REL_FLAGS := -O2
#CMN_FLAGS := -std=c++17
CMN_FLAGS := -std=c++20

BINON_DIR := ../..
CPP17_DIR := ${BINON_DIR}/cpp17
HEADERS_DIR := ${CPP17_DIR}/headers

BASE_FLAGS := -c -I${HEADERS_DIR} ${CMN_FLAGS}
LD_FLAGS := -L/usr/local/lib -ltbb

#export SDKROOT=`xcrun --show-sdk-path`

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

#	Check if binon needs to be rebuilt before building the current project.
target: makeBinON bin/example${SFX}
makeBinON: ${CPP17_DIR}/makefile
	${MAKE} -C ${CPP17_DIR} ${TARGET} \
		CXX="${CXX}" DBG_FLAGS=${DBG_FLAGS} REL_FLAGS=${REL_FLAGS} \
		CMN_FLAGS=${CMN_FLAGS}

#	This should force binon to rebuild after the current makefile is modified.
#	This is in case you say modified some compiler flags and want those to be
#	applied to the binon library as well.
${CPP17_DIR}/makefile: makefile
	touch ${CPP17_DIR}/makefile

SRC_NAMES := example
OBJS := ${addprefix obj/,${SRC_NAMES}}
OBJS := ${addsuffix ${SFX}.o,${OBJS}}
OBJS := ${OBJS} ${CPP17_DIR}/lib/libbinon${SFX}.a
bin/example${SFX}: bin obj ${OBJS}
	${CXX} ${OBJS} ${LD_FLAGS} ${LD_TGT_FLAGS} -o bin/example${SFX}
${CPP17_DIR}/lib/libbinon${SFX}.a:

bin:
	mkdir bin
obj:
	mkdir obj

obj/example${SFX}.o: example.cpp makefile ${HEADERS_DIR}/binon/binon.hpp
	${CXX} ${FLAGS} example.cpp -o obj/example${SFX}.o
