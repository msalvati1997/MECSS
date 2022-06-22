
all:
	gcc  src/main.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -w -Wall


%:
	@echo Available targets:
	@echo "  all"
	