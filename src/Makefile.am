bin_PROGRAMS = fetchtag

fetchtag_SOURCES = connect.c utils.c search.c tags.c fetchtag.c assign.c
noinst_HEADERS = connect.h common.h utils.h search.h tags.h assign.h

fetchtag_CFLAGS = @TAGLIB_C_CFLAGS@ @LUA_CFLAGS@
fetchtag_LDADD = @TAGLIB_C_LIBS@ @LUA_LIBS@
