all: wc_multi

wc_multi: wc_multi.c wc_core.c wc.h
	gcc wc_multi.c wc_core.c -g -o wc_multi

clean:
	rm -f wc_multi