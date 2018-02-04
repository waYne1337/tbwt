include external/sg-entropy/Make.helper
include external/divsufsort/Make.helper
include external/bcm/Make.helper
include external/sdsl/Make.helper

OWN_INCS = \
	aux-encoding.hpp \
	block-compressor.hpp \
	block-nav-support.hpp \
	block-scores-rle-model.hpp \
	bwt-compressor.hpp \
	bwt-config.hpp \
	bwt-run-support.hpp \
	entropy-coder.hpp \
	lheap.hpp \
	mtf-coder.hpp \
	rle0-coder.hpp \
	tbwt-compressor.hpp \
	tunneling-support.hpp \
	twobitvector.hpp
OWN_LIBS = \
	block-nav-support.cpp \
	bwt-run-support.cpp  \
	ui.cpp

INC_DIRS = external/sg-entropy external/divsufsort external/bcm external/sdsl/include include
LIB_DIRS = external/sg-entropy external/divsufsort external/bcm external/sdsl/lib lib

CC_OPTS = -O3 -DNDEBUG
CC_INCS = $(addprefix external/sg-entropy/,$(SG_ENTROPY_INCS)) \
          $(addprefix external/divsufsort/,$(DIVSUFSORT_INCS)) \
          $(addprefix external/bcm/,$(BCM_INCS)) \
          $(addprefix external/sdsl/,$(SDSL_INCS)) \
          $(addprefix include/,$(OWN_INCS))
CC_LIBS = $(addprefix lib/,$(OWN_LIBS)) \
          $(addprefix external/divsufsort/,$(DIVSUFSORT_LIBS))
BW_CC_LIBS  = $(addprefix external/sg-entropy/,$(SG_ENTROPY_LIBS)) $(CC_LIBS)
BCM_CC_LIBS = $(addprefix external/bcm/,$(BCM_LIBS)) $(CC_LIBS)
WT_CC_LIBS  = $(addprefix external/sdsl/,$(SDSL_LIBS)) $(CC_LIBS)

all:	bwzip.x tbwzip.x bcmzip.x tbcmzip.x wtzip.x twtzip.x

bwzip.x:	lib/ui.cpp include/bw94-compressor.hpp $(CC_INCS) $(BW_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DBW94 $(BW_CC_LIBS) -o bwzip.x

tbwzip.x:	lib/ui.cpp include/bw94-compressor.hpp $(CC_INCS) $(BW_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DTBWT $(BW_CC_LIBS) -o tbwzip.x

bcmzip.x:	lib/ui.cpp include/bcm-compressor.hpp $(CC_INCS) $(BCM_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DBCM $(BCM_CC_LIBS) -o bcmzip.x

tbcmzip.x:	lib/ui.cpp include/bcm-compressor.hpp $(CC_INCS) $(BCM_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DTBCM $(BCM_CC_LIBS) -o tbcmzip.x

wtzip.x:	lib/ui.cpp include/wt-compressor.hpp $(CC_INCS) $(WT_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DWT $(WT_CC_LIBS) -o wtzip.x

twtzip.x:	lib/ui.cpp include/wt-compressor.hpp $(CC_INCS) $(WT_CC_LIBS)
	g++ -std=c++11 -Wall -Wextra -g $(addprefix -I,$(INC_DIRS)) $(addprefix -L,$(LIB_DIRS)) $(CC_OPTS) \
		-DTWT $(WT_CC_LIBS) -o twtzip.x

clean:
	rm -f *.x
