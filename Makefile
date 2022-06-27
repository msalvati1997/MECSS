

debug:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG

release:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out


clean:
	rm out

%:
	@echo Available targets:
	@echo "  help"
	@echo "  debug"
	@echo "  release"
	@echo "  clean"
	

