
TEMPLATE = subdirs 

SUBDIRS = main \
          contrib \
          playground

include(../compilationtype.pri)
DESTDIR = ../../bin
