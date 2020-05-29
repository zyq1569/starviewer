
# Cal afegir el directori de l'extensió a la variable SUBDIRS tant si es compila com si no.

SUBDIRS =

include(../../extensions.pri)

TEMPLATE = subdirs

for(dir, SUBDIRS) {
    !exists($$dir) {
        SUBDIRS -= $$dir
    }
    !contains(CONTRIB_EXTENSIONS, $$dir) {
        SUBDIRS -= $$dir
    }
}

include(../../compilationtype.pri)
