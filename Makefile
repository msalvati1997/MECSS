
help : 


release_finite_alg1:
	gcc  src/alg1.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out FINITE > ./results/alg1/finite/release_finite_alg1.txt

debug_finite_alg1:
	gcc  src/alg1.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out FINITE > ./results/alg1/finite/debug_finite_alg1.txt

release_infinite_alg1:
	gcc  src/alg1.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out INFINITE > ./results/alg1/infinite/release_infinite_alg1.txt

debug_infinite_alg1:
	gcc  src/alg1.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out INFINITE > ./results/alg1/infinite/debug_infinite_alg1.txt
	
release_finite_alg1_migliorativo:
	gcc  src/alg1_migliorativo.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out FINITE > ./results/alg1_migliorativo/finite/release_finite_alg1_migliorativo.txt

debug_finite_alg1_migliorativo:
	gcc  src/alg1_migliorativo.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out FINITE > ./results/alg1_migliorativo/finite/debug_finite_alg1_migliorativo.txt

release_infinite_alg1_migliorativo:
	gcc  src/alg1_migliorativo.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out INFINITE > ./results/alg1_migliorativo/infinite/release_infinite_alg1_migliorativo.txt

debug_infinite_alg1_migliorativo:
	gcc  src/alg1_migliorativo.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out INFINITE > ./results/alg1_migliorativo/infinite/debug_infinite_alg1_migliorativo.txt

release_finite_alg2:
	gcc  src/alg2.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out FINITE >  ./results/alg2/finite/release_finite_alg2.txt

debug_finite_alg2:
	gcc  src/alg2.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out FINITE >  ./results/alg2/finite/debug_finite_alg2.txt

release_infinite_alg2:
	gcc  src/alg2.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out 
	./out INFINITE > ./results/alg2/infinite/release_infinite_alg2.txt

debug_infinite_alg2:
	gcc  src/alg2.c lib/rvgs.c lib/rvms.c lib/rngs.c lib/utils.c -lm -g -o out -DDEBUG 
	./out INFINITE > ./results/alg2/infinite/debug_infinite_alg2.txt

clean: 
	rm out

clean_infinite_alg1:
	rm results/alg1/infinite/*.csv
	rm results/alg1/infinite/*.txt

clean_infinite_alg1_migliorativo:
	rm results/alg1_migliorativo/infinite/*.csv
	rm results/alg1_migliorativo/infinite/*.txt

clean_infinite_alg2:
	rm results/alg2/infinite/*.csv
	rm results/alg2/infinite/*.txt

clean_finite_alg1:
	rm results/alg1/finite/*.csv
	rm results/alg1/infinite/*.txt

clean_finite_alg1_migliorativo:
	rm results/alg1_migliorativo/finite/*.csv
	rm results/alg1_migliorativo/finite/*.txt

clean_finite_alg2:	
	rm results/alg2/finite/*.csv
	rm results/alg2/infinite/*.txt


%:
	@echo Available targets:
	@echo "  help"
	@echo "  clean"
	@echo "  clean_finite_alg1"
	@echo "  clean_finite_alg1_migliorativo"
	@echo "  clean_finite_alg2"
	@echo "  clean_infinite_alg1"
	@echo "  clean_infinite_alg1_migliorativo"
	@echo "  clean_infinite_alg2"
	@echo "  release_finite_alg1"
	@echo "  release_finite_alg1_migliorativo"
	@echo "  release_finite_alg2"
	@echo "  release_infinite_alg1"
	@echo "  release_infinite_alg1_migliorativo"
	@echo "  release_infinite_alg2"
	@echo "  debug_finite_alg1"
	@echo "  debug_finite_alg1_migliorativo"
	@echo "  debug_finite_alg2"
	@echo "  debug_infinite_alg1"
	@echo "  debug_infinite_alg1_migliorativo"
	@echo "  debug_infinite_alg2"

