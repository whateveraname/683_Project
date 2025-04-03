## How to train a strategy profile
1. Calculate equities for each hand

	Enter OMPEval folder

	Modify paths in cal_equity.cpp

	Compile:
	``` bash
	mkdir -p build && cd build && cmake .. && make
	```
	
	Calculate and store equities in files:
	``` bash
	./cal_equity
	```
3. Cluster hands into buckets by equity

	Enter abstraction folder

	Modify paths in card_abstraction.py

	Install python dependency:
	``` bash
	conda install pytorch::faiss-cpu
	```
	Calculate and store cluster boundaries in files:
	``` bash
	python card_abstraction.py
	```
	Modify paths in bucket_table.cpp

	Compile:
	``` bash
	g++ -fopenmp -O3 bucket_table.cpp -o bucket_table
	```	
	Calculate and store bucket id for each hand in file:
	``` bash
	./bucket_table
	```
5. Run PureCFR

	Enter open-pure-cfr folder

	Modify paths in bucketing_abstraction.hpp

	Compile:
	``` bash
	mkdir -p build && cd build && cmake .. && make
	```
	Start training (set --threads to as large as possible, for other parameters please check open_pure_cfr's README):

	Also double check the game definition in holdem.limit.2p.reverse_blinds.game is compatible with our project's. I've not done it yet.
	``` bash
	./pure_cfr ../games/holdem.limit.2p.reverse_blinds.game test.holdem.2pl --threads=72
	```

## TODO
Add evaluate function to monitor the training process.

	
	
