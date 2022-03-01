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

OBJS := \
	build/packelems.o \
	build/codebyte.o \
	build/listhelpers.o \
	build/hashutil.o \
	build/binonobj.o \
	build/ioutil.o \
	build/listobj.o \
	build/dictobj.o \
	build/floatobj.o \
	build/strobj.o \
	build/intobj.o \
	build/boolobj.o \
	build/dicthelpers.o \
	build/objhelpers.o \
	build/bufferobj.o \
	build/byteutil.o

${DEST_DIR}/lib/libbinon.a: ${OBJS}
	ar -crs ${DEST_DIR}/lib/libbinon.a ${OBJS}

binon_macros_hpp_deps := \
	headers/binon/macros.hpp \
	makefile
binon_ioutil_hpp_deps := \
	headers/binon/ioutil.hpp \
	${binon_macros_hpp_deps}
binon_errors_hpp_deps := \
	headers/binon/errors.hpp \
	makefile
binon_byteutil_hpp_deps := \
	headers/binon/byteutil.hpp \
	${binon_ioutil_hpp_deps} \
	${binon_errors_hpp_deps}
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
binon_nullobj_hpp_deps := \
	headers/binon/nullobj.hpp \
	${binon_mixins_hpp_deps}
binon_boolobj_hpp_deps := \
	headers/binon/boolobj.hpp \
	${binon_mixins_hpp_deps}
binon_floattypes_hpp_deps := \
	headers/binon/floattypes.hpp \
	makefile
binon_floatobj_hpp_deps := \
	headers/binon/floatobj.hpp \
	${binon_mixins_hpp_deps} \
	${binon_floattypes_hpp_deps}
binon_listobj_hpp_deps := \
	headers/binon/listobj.hpp \
	${binon_mixins_hpp_deps}
binon_dictobj_hpp_deps := \
	headers/binon/dictobj.hpp \
	${binon_listobj_hpp_deps}
binon_optutil_hpp_deps := \
	headers/binon/optutil.hpp \
	${binon_macros_hpp_deps}
binon_hystr_hpp_deps := \
	headers/binon/hystr.hpp \
	${binon_macros_hpp_deps}
binon_strobj_hpp_deps := \
	headers/binon/strobj.hpp \
	${binon_hystr_hpp_deps} \
	${binon_mixins_hpp_deps}
binon_intobj_hpp_deps := \
	headers/binon/intobj.hpp \
	${binon_hystr_hpp_deps} \
	${binon_mixins_hpp_deps}
binon_bufferobj_hpp_deps := \
	headers/binon/bufferobj.hpp \
	${binon_intobj_hpp_deps}
binon_binonobj_hpp_deps := \
	headers/binon/binonobj.hpp \
	${binon_nullobj_hpp_deps} \
	${binon_boolobj_hpp_deps} \
	${binon_floatobj_hpp_deps} \
	${binon_dictobj_hpp_deps} \
	${binon_optutil_hpp_deps} \
	${binon_strobj_hpp_deps} \
	${binon_bufferobj_hpp_deps}
binon_packelems_hpp_deps := \
	headers/binon/packelems.hpp \
	${binon_binonobj_hpp_deps}
binon_typeconv_hpp_deps := \
	headers/binon/typeconv.hpp \
	${binon_binonobj_hpp_deps}
binon_objhelpers_hpp_deps := \
	headers/binon/objhelpers.hpp \
	${binon_typeconv_hpp_deps}
binon_listhelpers_hpp_deps := \
	headers/binon/listhelpers.hpp \
	${binon_objhelpers_hpp_deps}
binon_dicthelpers_hpp_deps := \
	headers/binon/dicthelpers.hpp \
	${binon_objhelpers_hpp_deps}

headers/binon/binon.hpp: \
	${binon_iterable_hpp_deps} \
	${binon_dicthelpers_hpp_deps} \
	${binon_idgen_hpp_deps} \
	${binon_listhelpers_hpp_deps}
	touch ${HDR}/binon.hpp

build/packelems.o: source/packelems.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/packelems.cpp -o build/packelems.o
build/codebyte.o: source/codebyte.cpp ${binon_codebyte_hpp_deps}
	${CXX} ${FLAGS} source/codebyte.cpp -o build/codebyte.o
build/listhelpers.o: source/listhelpers.cpp ${binon_listhelpers_hpp_deps}
	${CXX} ${FLAGS} source/listhelpers.cpp -o build/listhelpers.o
build/hashutil.o: source/hashutil.cpp ${binon_hashutil_hpp_deps}
	${CXX} ${FLAGS} source/hashutil.cpp -o build/hashutil.o
build/binonobj.o: source/binonobj.cpp ${binon_objhelpers_hpp_deps}
	${CXX} ${FLAGS} source/binonobj.cpp -o build/binonobj.o
build/ioutil.o: source/ioutil.cpp ${binon_ioutil_hpp_deps}
	${CXX} ${FLAGS} source/ioutil.cpp -o build/ioutil.o
build/listobj.o: source/listobj.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/listobj.cpp -o build/listobj.o
build/dictobj.o: source/dictobj.cpp ${binon_packelems_hpp_deps}
	${CXX} ${FLAGS} source/dictobj.cpp -o build/dictobj.o
build/floatobj.o: source/floatobj.cpp ${binon_floatobj_hpp_deps}
	${CXX} ${FLAGS} source/floatobj.cpp -o build/floatobj.o
build/strobj.o: source/strobj.cpp \
	${binon_intobj_hpp_deps} \
	${binon_strobj_hpp_deps}
	${CXX} ${FLAGS} source/strobj.cpp -o build/strobj.o
build/intobj.o: source/intobj.cpp ${binon_intobj_hpp_deps}
	${CXX} ${FLAGS} source/intobj.cpp -o build/intobj.o
build/boolobj.o: source/boolobj.cpp ${binon_boolobj_hpp_deps}
	${CXX} ${FLAGS} source/boolobj.cpp -o build/boolobj.o
build/dicthelpers.o: source/dicthelpers.cpp ${binon_dicthelpers_hpp_deps}
	${CXX} ${FLAGS} source/dicthelpers.cpp -o build/dicthelpers.o
build/objhelpers.o: source/objhelpers.cpp ${binon_objhelpers_hpp_deps}
	${CXX} ${FLAGS} source/objhelpers.cpp -o build/objhelpers.o
build/bufferobj.o: source/bufferobj.cpp ${binon_bufferobj_hpp_deps}
	${CXX} ${FLAGS} source/bufferobj.cpp -o build/bufferobj.o
build/byteutil.o: source/byteutil.cpp ${binon_byteutil_hpp_deps}
	${CXX} ${FLAGS} source/byteutil.cpp -o build/byteutil.o
