all: dat2json
	echo "make all ran successfully"

############################################################################
CFLAGS=-Wno-format-contains-nul
LDFLAGS=

############################################################################
dat2json_o = dat2json.o get_gal.o get_star.o get_plan.o utils.o
dat2json: $(dat2json_o)
	cc -o ../bin/dat2json $(CFLAGS) $(dat2json_o)

############################################################################
dat2json.o: dat2json.c fh.h
	cc $(CFLAGS) -DTHIS_IS_MAIN -c dat2json.c

get_gal.o: get_gal.c fh.h
	cc $(CFLAGS) -c get_gal.c

get_plan.o: get_plan.c fh.h
	cc $(CFLAGS) -c get_plan.c

get_star.o: get_star.c fh.h
	cc $(CFLAGS) -c get_star.c

utils.o: utils.c fh.h
	cc $(CFLAGS) -c utils.c

############################################################################
.PHONY: clean check
clean:
	rm -f $(dat2json_o)
# check is not yet implemented
check:
	/bin/false
