
# This file expects that OBJ contains a list of all of the object files.
# The directory portion of each object file is used to locate the source
# and should not contain any ..'s but rather be relative to the top of the
# tree.
#
# So for example, py/map.c would have an object file name py/map.o
# The object files will go into the build directory and mantain the same
# directory structure as the source tree. So the final dependency will look
# like this:
#
# build/py/map.o: py/map.c
#
# We set vpath to point to the top of the tree so that the source files
# can be located. By following this scheme, it allows a single build rule
# to be used to compile all .c files.

#OMV_OBJ_DIRS = $(sort $(dir $(OMV_OBJS)))

#all: | $(OMV_OBJ_DIRS) $(OMV_OBJS)
#$(OMV_OBJ_DIRS):
#	mkdir -p $@

vpath %.c . $(OPENMV_BASE)/src/omv/
vpath %.c . $(OPENMV_BASE)/src/omv/img/
vpath %.c . $(OPENMV_BASE)/src/omv/py/
vpath %.c . $(OPENMV_BASE)/src/omv/nn/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/dsp/CommonTables/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/dsp/FastMathFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/dsp/MatrixFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/ActivationFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/ConvolutionFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/FullyConnectedFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/NNSupportFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/PoolingFunctions/
vpath %.c . $(OPENMV_BASE)/src/cmsis/src/nn/SoftmaxFunctions/
$(OMV_BUILD)/%.o : %.c
	$(ECHO) "CC $< to $@"
	$(Q)$(CC) $(CFLAGS) -c -MD -o $@ $<

#$(warning CFLAGS is $(CFLAGS))
#$(OMV_BUILD)/%.o : %.s
#	$(ECHO) "AS $<"
#	$(AS) $(AFLAGS) $< -o $@

