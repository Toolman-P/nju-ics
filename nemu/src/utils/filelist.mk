ifdef CONFIG_ITRACE
CXXSRC = src/utils/disasm.cc
CXXFLAGS += $(shell llvm-config --cxxflags) -fPIE
LIBS += $(shell llvm-config --libs)
endif

ifndef CONFIG_FTRACE
SRCS-BLACKLIST-y += src/utils/ftrace.c
endif