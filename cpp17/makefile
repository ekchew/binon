CXX ?= c++
DBG_FLAGS ?= -g -D DEBUG
REL_FLAGS ?= -O2
CMN_FLAGS ?= -std=c++17

# SUFFIX is an optional string that gets added to the end of all build products
# just before the file extension. The idea is that you can create different
# builds for different compile options. For example, you could run:
#
#     CMN_FLAGS=-std=c++20 SUFFIX=_cpp20 make
#
# This will build for C++20 (the library does have some support for C++20
# concepts) but build libbinon_cpp20.a libraries (as well as whatever_cpp20.o
# object files on the way there).
SUFFIX ?=

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
	${MAKE} ${DEST_DIR}/lib/libbinon${SUFFIX}.a

${TEST_DIR}: ${HDR}/binon.hpp
	mkdir -p ${TEST_DIR}

OBJ_DIR := ${DEST_DIR}/obj
OBJS := \
	${OBJ_DIR}/binonobj${SUFFIX}.o \
	${OBJ_DIR}/boolobj${SUFFIX}.o \
	${OBJ_DIR}/bufferobj${SUFFIX}.o \
	${OBJ_DIR}/byteutil${SUFFIX}.o \
	${OBJ_DIR}/codebyte${SUFFIX}.o \
	${OBJ_DIR}/dicthelpers${SUFFIX}.o \
	${OBJ_DIR}/dictobj${SUFFIX}.o \
	${OBJ_DIR}/floatobj${SUFFIX}.o \
	${OBJ_DIR}/hashutil${SUFFIX}.o \
	${OBJ_DIR}/intobj${SUFFIX}.o \
	${OBJ_DIR}/ioutil${SUFFIX}.o \
	${OBJ_DIR}/listhelpers${SUFFIX}.o \
	${OBJ_DIR}/listobj${SUFFIX}.o \
	${OBJ_DIR}/objhelpers${SUFFIX}.o \
	${OBJ_DIR}/packelems${SUFFIX}.o \
	${OBJ_DIR}/strobj${SUFFIX}.o

${DEST_DIR}/lib/libbinon${SUFFIX}.a: ${OBJS}
	ar -crs ${DEST_DIR}/lib/libbinon${SUFFIX}.a ${OBJS}

binon_macros_hpp_deps := \
	headers/binon/macros.hpp \
	makefile
binon_hystr_hpp_deps := \
	headers/binon/hystr.hpp \
	${binon_macros_hpp_deps}
binon_ioutil_hpp_deps := \
	headers/binon/ioutil.hpp \
	${binon_macros_hpp_deps}
binon_errors_hpp_deps := \
	headers/binon/errors.hpp \
	makefile
binon_byteutil_hpp_deps := \
	headers/binon/byteutil.hpp \
	${binon_errors_hpp_deps} \
	${binon_ioutil_hpp_deps}
binon_codebyte_hpp_deps := \
	headers/binon/codebyte.hpp \
	${binon_byteutil_hpp_deps}
binon_typeutil_hpp_deps := \
	headers/binon/typeutil.hpp \
	${binon_macros_hpp_deps}
binon_hashutil_hpp_deps := \
	headers/binon/hashutil.hpp \
	${binon_byteutil_hpp_deps} \
	${binon_typeutil_hpp_deps}
binon_mixins_hpp_deps := \
	headers/binon/mixins.hpp \
	${binon_codebyte_hpp_deps} \
	${binon_hashutil_hpp_deps}
binon_intobj_hpp_deps := \
	headers/binon/intobj.hpp \
	${binon_hystr_hpp_deps} \
	${binon_mixins_hpp_deps}
binon_bufferobj_hpp_deps := \
	headers/binon/bufferobj.hpp \
	${binon_intobj_hpp_deps}
binon_floattypes_hpp_deps := \
	headers/binon/floattypes.hpp \
	makefile
binon_floatobj_hpp_deps := \
	headers/binon/floatobj.hpp \
	${binon_floattypes_hpp_deps} \
	${binon_mixins_hpp_deps}
binon_boolobj_hpp_deps := \
	headers/binon/boolobj.hpp \
	${binon_mixins_hpp_deps}
binon_strobj_hpp_deps := \
	headers/binon/strobj.hpp \
	${binon_hystr_hpp_deps} \
	${binon_mixins_hpp_deps}
binon_listobj_hpp_deps := \
	headers/binon/listobj.hpp \
	${binon_mixins_hpp_deps}
binon_dictobj_hpp_deps := \
	headers/binon/dictobj.hpp \
	${binon_listobj_hpp_deps}
binon_nullobj_hpp_deps := \
	headers/binon/nullobj.hpp \
	${binon_mixins_hpp_deps}
binon_optutil_hpp_deps := \
	headers/binon/optutil.hpp \
	${binon_macros_hpp_deps}
binon_binonobj_hpp_deps := \
	headers/binon/binonobj.hpp \
	${binon_boolobj_hpp_deps} \
	${binon_bufferobj_hpp_deps} \
	${binon_dictobj_hpp_deps} \
	${binon_floatobj_hpp_deps} \
	${binon_nullobj_hpp_deps} \
	${binon_optutil_hpp_deps} \
	${binon_strobj_hpp_deps}
binon_typeconv_hpp_deps := \
	headers/binon/typeconv.hpp \
	${binon_binonobj_hpp_deps}
binon_objhelpers_hpp_deps := \
	headers/binon/objhelpers.hpp \
	${binon_typeconv_hpp_deps}
binon_dicthelpers_hpp_deps := \
	headers/binon/dicthelpers.hpp \
	${binon_objhelpers_hpp_deps}
binon_listhelpers_hpp_deps := \
	headers/binon/listhelpers.hpp \
	${binon_objhelpers_hpp_deps}
binon_packelems_hpp_deps := \
	headers/binon/packelems.hpp \
	${binon_binonobj_hpp_deps}

headers/binon/binon.hpp: \
	${binon_dicthelpers_hpp_deps} \
	${binon_idgen_hpp_deps} \
	${binon_iterable_hpp_deps} \
	${binon_listhelpers_hpp_deps}
	touch ${HDR}/binon.hpp

${OBJ_DIR}/binonobj${SUFFIX}.o: source/binonobj.cpp ${binon_objhelpers_hpp_deps}
	${CXX} ${FLAGS} source/binonobj.cpp -o ${OBJ_DIR}/binonobj${SUFFIX}.o
${OBJ_DIR}/boolobj${SUFFIX}.o: source/boolobj.cpp ${binon_boolobj_hpp_deps}
	${CXX} ${FLAGS} source/boolobj.cpp -o ${OBJ_DIR}/boolobj${SUFFIX}.o
${OBJ_DIR}/bufferobj${SUFFIX}.o: source/bufferobj.cpp ${binon_bufferobj_hpp_deps}
	${CXX} ${FLAGS} source/bufferobj.cpp -o ${OBJ_DIR}/bufferobj${SUFFIX}.o
${OBJ_DIR}/byteutil${SUFFIX}.o: source/byteutil.cpp ${binon_byteutil_hpp_deps}
	${CXX} ${FLAGS} source/byteutil.cpp -o ${OBJ_DIR}/byteutil${SUFFIX}.o
${OBJ_DIR}/codebyte${SUFFIX}.o: source/codebyte.cpp ${binon_codebyte_hpp_deps}
	${CXX} ${FLAGS} source/codebyte.cpp -o ${OBJ_DIR}/codebyte${SUFFIX}.o
${OBJ_DIR}/dicthelpers${SUFFIX}.o: source/dicthelpers.cpp ${binon_dicthelpers_hpp_deps}
	${CXX} ${FLAGS} source/dicthelpers.cpp -o ${OBJ_DIR}/dicthelpers${SUFFIX}.o
${OBJ_DIR}/dictobj${SUFFIX}.o: source/dictobj.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/dictobj.cpp -o ${OBJ_DIR}/dictobj${SUFFIX}.o
${OBJ_DIR}/floatobj${SUFFIX}.o: source/floatobj.cpp ${binon_floatobj_hpp_deps}
	${CXX} ${FLAGS} source/floatobj.cpp -o ${OBJ_DIR}/floatobj${SUFFIX}.o
${OBJ_DIR}/hashutil${SUFFIX}.o: source/hashutil.cpp ${binon_hashutil_hpp_deps}
	${CXX} ${FLAGS} source/hashutil.cpp -o ${OBJ_DIR}/hashutil${SUFFIX}.o
${OBJ_DIR}/intobj${SUFFIX}.o: source/intobj.cpp ${binon_intobj_hpp_deps}
	${CXX} ${FLAGS} source/intobj.cpp -o ${OBJ_DIR}/intobj${SUFFIX}.o
${OBJ_DIR}/ioutil${SUFFIX}.o: source/ioutil.cpp ${binon_ioutil_hpp_deps}
	${CXX} ${FLAGS} source/ioutil.cpp -o ${OBJ_DIR}/ioutil${SUFFIX}.o
${OBJ_DIR}/listhelpers${SUFFIX}.o: source/listhelpers.cpp ${binon_listhelpers_hpp_deps}
	${CXX} ${FLAGS} source/listhelpers.cpp -o ${OBJ_DIR}/listhelpers${SUFFIX}.o
${OBJ_DIR}/listobj${SUFFIX}.o: source/listobj.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/listobj.cpp -o ${OBJ_DIR}/listobj${SUFFIX}.o
${OBJ_DIR}/objhelpers${SUFFIX}.o: source/objhelpers.cpp ${binon_objhelpers_hpp_deps}
	${CXX} ${FLAGS} source/objhelpers.cpp -o ${OBJ_DIR}/objhelpers${SUFFIX}.o
${OBJ_DIR}/packelems${SUFFIX}.o: source/packelems.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/packelems.cpp -o ${OBJ_DIR}/packelems${SUFFIX}.o
${OBJ_DIR}/strobj${SUFFIX}.o: source/strobj.cpp \
	${binon_intobj_hpp_deps} \
	${binon_strobj_hpp_deps}
	${CXX} ${FLAGS} source/strobj.cpp -o ${OBJ_DIR}/strobj${SUFFIX}.o
