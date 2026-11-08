lib.name = sensor_receiver

class.sources = sensor_decode.c sensor_calculate.c

datafiles = sensor.pd

PDLIBBUILDER_DIR = pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
