all: measure generate

generate: generate.c
	gcc generate.c -o generate

measure: measure.c
	gcc -O0 measure.c -o measure

clean:
	rm -rf measure generate
