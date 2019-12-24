all: access generate

generate: generate.c common.h
	gcc generate.c -o generate

access: access.c common.h
	gcc -O0 access.c -o access

clean:
	rm -rf access generate
