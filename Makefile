

all:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -Wall -Wextra -o out

clean:
	rm out

%:
	@echo Available targets:
	@echo "  help"
	@echo "  all"
	@echo "  clean"
	

