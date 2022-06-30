
help : 


release_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out FINITE > finite.txt

debug_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out FINITE > finite.txt

release_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out INFINITE > infinite.txt

debug_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out INFINITE > infinite.txt

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


