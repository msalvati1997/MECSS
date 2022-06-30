
help : 


release_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out FINITE

debug_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out FINITE

release_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out INFINITE

debug_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out INFINITE

clean:
	rm out
	rm *.csv

%:
	@echo Available targets:
	@echo "  help"
	@echo "  clean"
	@echo "  release_finite"
	@echo "  release_infinite"
	@echo "  debug_finite"
	@echo "  debug_infinite"


