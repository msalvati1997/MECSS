

release:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 

debug:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG


clean:
	rm out

%:
	@echo Available targets:
	@echo "  help"
	@echo "  debug"
	@echo "  release"
	@echo "  clean"
	

