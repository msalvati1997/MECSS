
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
	rm *.csv
	rm ./results/alg1/finite/*.csv
	rm ./results/alg1/infinite/*.csv
	rm ./results/alg2/finite/*.csv
	rm ./results/alg2/infinite/*.csv


%:
	@echo Available targets:
	@echo "  help"
	@echo "  clean"
	@echo "  release_finite_alg1"
	@echo "  release_finite_alg2"
	@echo "  release_infinite_alg1"
	@echo "  release_infinite_alg2"
	@echo "  debug_finite_alg1"
	@echo "  debug_finite_alg2"
	@echo "  debug_infinite_alg1"
	@echo "  debug_infinite_alg2"

