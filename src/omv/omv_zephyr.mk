# where py object files go (they have a name prefix to prevent filename clashes)
OMV_BUILD = $(BUILD)/omv

# py object files tmp remove main.c
CMSIS_SRC_C  = $(wildcard $(OPENMV_BASE)/src/cmsis/src/dsp/CommonTables/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/dsp/FastMathFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/dsp/MatrixFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/ActivationFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/ConvolutionFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/FullyConnectedFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/NNSupportFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/PoolingFunctions/*.c)
CMSIS_SRC_C  += $(wildcard $(OPENMV_BASE)/src/cmsis/src/nn/SoftmaxFunctions/*.c)

CMSIS_SRC_BASE_C = $(notdir $(CMSIS_SRC_C))

OMV_MAIN_BASE_SRC = \
	omv_zephyr_main.c   \
	xalloc.c            \
        fb_alloc.c          \
        umm_malloc.c        \
        ff_wrapper.c        \
        framebuffer.c       \
        array.c             \
        usbdbg_zephyr.c     \
        mt9m114.c           \
        sensor_zephyr.c     \
	cambus_zephyr.c     \
        mutex.c             \
        trace.c             \

#OMV_MAIN_SRC = $(addprefix ../../../omv/, $(OMV_MAIN_BASE_SRC))

OMV_IMG_BASE_SRC = \
	binary.c                \
	blob.c                  \
	clahe.c                 \
	draw.c                  \
	qrcode.c                \
	apriltag.c              \
	dmtx.c                  \
	zbar.c                  \
	fmath.c                 \
	fsort.c                 \
	qsort.c                 \
	fft.c                   \
	filter.c                \
	haar.c                  \
	imlib.c                 \
	collections.c           \
	stats.c                 \
	integral.c              \
	integral_mw.c           \
	kmeans.c                \
	lab_tab.c               \
	xyz_tab.c               \
	yuv_tab.c               \
	rainbow_tab.c           \
	rgb2rgb_tab.c           \
	invariant_tab.c         \
	mathop.c                \
	pool.c                  \
	point.c                 \
	rectangle.c             \
	bmp.c                   \
	ppm.c                   \
	gif.c                   \
	mjpeg.c                 \
	fast.c                  \
	agast.c                 \
	orb.c                   \
	template.c              \
	phasecorrelation.c      \
	shadow_removal.c        \
	font.c                  \
	jpeg.c                  \
	lbp.c                   \
	eye.c                   \
	hough.c                 \
	line.c                  \
	lsd.c                   \
	sincos_tab.c            \
	edge.c                  \
	hog.c                   \
	selective_search.c      \

#OMV_IMG_SRC = $(addprefix ../../../omv/img/, $(OMV_IMG_BASE_SRC))

OMV_NN_BASE_SRC = nn.c

#OMV_NN_SRC = $(addprefix ../../../omv/nn/,$(OMV_NN_BASE_SRC))

OMV_PY_BASE_SRC = \
	py_helper.c             \
	py_omv.c                \
	py_sensor.c             \
	py_image.c              \
	py_time.c               \
	py_lcd_zephyr.c         \
	py_gif.c                \
	py_mjpeg.c              \
	py_cpufreq.c            \
	py_nn.c                 \

#OMV_PY_SRC = $(addprefix ../../../omv/py/, $(OMV_PY_BASE_SRC))

OMV_BASE_SRCS = $(OMV_MAIN_BASE_SRC) $(OMV_IMG_BASE_SRC) $(OMV_NN_BASE_SRC) $(OMV_PY_BASE_SRC) $(CMSIS_SRC_BASE_C)

#OMV_SRCS = $(OMV_MAIN_SRC) $(OMV_IMG_SRC) $(OMV_NN_SRC) $(OMV_PY_SRC) $(CMSIS_SRC_C)

OMV_OBJS = $(addprefix $(OMV_BUILD)/, $(OMV_BASE_SRCS:.c=.o))

# Anything that depends on FORCE will be considered out-of-date
FORCE:
.PHONY: FORCE


