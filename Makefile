
help : 


release_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out finite

debug_finite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out finite

release_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out infinite

debug_infinite:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out infinite

clean:
	rm out

%:
	@echo Available targets:
	@echo "  help"
	@echo "  release_finite"
	@echo "  release_infinite"
	@echo "  debug_finite"
	@echo "  debug_infinite"


