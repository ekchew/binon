CXX := c++
#CXX := xcrun g++-10
DBG_FLAGS := -g
REL_FLAGS := -O2
CMN_FLAGS := -std=c++17
#CMN_FLAGS := -std=c++20

BINON_DIR := ../..
CPP17_DIR := ${BINON_DIR}/cpp17
HEADERS_DIR := ${CPP17_DIR}/headers

BASE_FLAGS := -c -I${HEADERS_DIR} ${CMN_FLAGS}
LD_FLAGS := -L/usr/local/lib -ltbb

all: mk_dirs
	${MAKE} all_target
all_target: debug_target release_target

debug: mk_dirs
	${MAKE} debug_target
debug_target:
	${MAKE} final_target FLAGS="${BASE_FLAGS} ${DBG_FLAGS}" \
		TARGET="debug_target" SFX="_dbg" LD_TGT_FLAGS="-g"

release: mk_dirs
	${MAKE} release_target
release_target:
	${MAKE} final_target FLAGS="${BASE_FLAGS} ${REL_FLAGS}" \
		TARGET="release_target" SFX="" LD_TGT_FLAGS=""

mk_dirs: bin obj
	${MAKE} -C "${CPP17_DIR}" mk_dirs

clean:
	${MAKE} -C ${CPP17_DIR} clean
	rm -frv bin
	rm -frv obj

final_target: ${CPP17_DIR}/makefile
	${MAKE} -C "${CPP17_DIR}" "${TARGET}" \
		CXX="${CXX}" DBG_FLAGS="${DBG_FLAGS}" REL_FLAGS="${REL_FLAGS}" \
		CMN_FLAGS="${CMN_FLAGS}"
	${MAKE} "bin/example${SFX}"

#	This should force binon to rebuild after the current makefile is modified.
#	This is in case you say modified some compiler flags and want those to be
#	applied to the binon library as well.
${CPP17_DIR}/makefile: makefile
	touch ${CPP17_DIR}/makefile

SRC_NAMES := example
OBJS := ${addprefix obj/,${SRC_NAMES}}
OBJS := ${addsuffix ${SFX}.o,${OBJS}}
bin/example${SFX}: ${OBJS}
	${CXX} ${OBJS} ${CPP17_DIR}/lib/libbinon${SFX}.a \
	${LD_FLAGS} ${LD_TGT_FLAGS} -o bin/example${SFX}

bin:
	mkdir bin
obj:
	mkdir obj

obj/example${SFX}.o: example.cpp makefile ${HEADERS_DIR}/binon/binon.hpp
	${CXX} ${FLAGS} example.cpp -o obj/example${SFX}.o
