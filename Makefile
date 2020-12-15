CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -O0 -lm -ldl -std=gnu99
CFLAGS += $(shell pkg-config --cflags json-c) # json-c flags
CFLAGS_GTK = $(shell pkg-config --cflags gtk+-3.0)


CLIBS = `sdl2-config --cflags --libs` -lSDL2_image -lpthread
CLIBS += $(shell pkg-config --libs json-c) #json-c libs
CLIBS_GTK = $(shell pkg-config --libs gtk+-3.0 -export-dynamic)

#SRC = main.c
SRC =  src/matrix/matrix.c 
SRC += src/image/image.c 
SRC += src/image/binarization.c 
SRC += src/ml/struct/network.c
SRC += src/ml/struct/layer.c 
SRC += src/ml/struct/neurone.c
SRC += src/math/sortalgo.c
SRC += src/rotate/rotate.c
SRC += src/math/analysis.c
SRC += src/math/random.c
SRC += src/ml/train/backpropagation.c
SRC += src/segmentation/segmentation.c
SRC += src/ml/struct/networks.c
SRC += src/ml/train/lib/backpropagMISC.c
SRC += src/miscellaneous/CHARS.c
#SRC += OCR_project.c
#SRC += OCR_project.glade


OUT = ocr
DEBUG_OUT = a.out


all: ${SRC} ${SRC}
	${CC} main.c ${SRC} ${CLIBS} ${CFLAGS} -g -o main

xor : ${SRC} ${SRC}
	${CC} mainXOR.c ${SRC} ${CLIBS} ${CFLAGS} -g -o mainXOR

binarization : ${SRC} ${SRC}
	${CC} mainBINARIZATION.c ${SRC} ${CLIBS} ${CFLAGS} -g -o mainBINARIZATION

rotation : ${SRC} ${SRC}
	${CC} mainROTATION.c ${SRC} ${CLIBS} ${CFLAGS} -g -o mainROTATION

segmentation : ${SRC} ${SRC}
	${CC} mainSEGMENTATION.c ${SRC} ${CLIBS} ${CFLAGS} -g -o mainSEGMENTATION

test : ${SRC} ${SRC}
	${CC} maintest.c ${SRC} ${CLIBS} ${CFLAGS} -g -o maintest

train : ${SRC} ${SRC}
	${CC} mainTRAIN.c ${SRC} ${CLIBS} ${CFLAGS} -g -o mainTRAIN
