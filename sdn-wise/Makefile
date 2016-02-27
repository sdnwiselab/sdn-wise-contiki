CONTIKI_PROJECT = sdn-wise
all: $(CONTIKI_PROJECT)

CONTIKI = ../contiki
CONTIKI_WITH_RIME = 1

PROJECT_SOURCEFILES = flowtable.c packet-buffer.c address.c neighbor-table.c packet-handler.c node-conf.c packet-creator.c
TARGETDIRS += ../targets
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
