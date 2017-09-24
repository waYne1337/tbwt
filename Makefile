include external/sg-entropy/Make.helper
include external/divsufsort/Make.helper

OWN_INCS = entropy-coder.hpp mtf-coder.hpp rle0-coder.hpp bwt-run-support.hpp lheap.hpp\
           block-nav-support.hpp tunnel-enc-support.hpp tunneling-support.hpp\
           twobitvector.hpp aux-encoding.hpp bwt-config.hpp block-compressor.hpp
OWN_LIBS = bwt-run-support.cpp block-nav-support.cpp tunnel-enc-support.cpp tunneling-support.cpp

INC_DIRS = external/sg-entropy external/divsufsort include
LIB_DIRS = external/sg-entropy external/divsufsort lib

CC_OPTS = -O3 -DNDEBUG
CC_INCS = $(addprefix external/sg-entropy/,$(SG_ENTROPY_INCS)) \
          $(addprefix external/divsufsort/,$(DIVSUFSORT_INCS)) \
          $(addprefix include/,$(OWN_INCS))
CC_LIBS = $(addprefix external/sg-entropy/,$(SG_ENTROPY_LIBS)) \
          $(addprefix external/divsufsort/,$(DIVSUFSORT_LIBS)) \
          $(addprefix lib/,$(OWN_LIBS))

all:	bwzip.x tbwzip.x

bwzip.x:	lib/ui.cpp include/bwt-compressor.hpp lib/bwt-compressor.cpp $(CC_INCS) $(CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DBWT lib/ui.cpp $(CC_LIBS) lib/bwt-compressor.cpp -o bwzip.x

tbwzip.x:	lib/ui.cpp include/tbwt-compressor.hpp lib/tbwt-compressor.cpp $(CC_INCS) $(CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DTBWT lib/ui.cpp $(CC_LIBS) lib/tbwt-compressor.cpp -o tbwzip.x

clean:
	rm -f bwzip.x
	rm -f tbwzip.x
