all: access generate

generate: generate.c
	gcc generate.c -o generate

access: access.c
	gcc -O0 access.c -o access

clean:
	rm -rf measure generate
