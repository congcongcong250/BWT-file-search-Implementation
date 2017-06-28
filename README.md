# BWTSearch Data Compression and Search 
by Liren Wang
## Operations:
This program implements the BWT backwards search with three options.

1. "-n": 
Find number of all matches of the pattern in the bwt ehcoded file.
Using basic BWT backwards search algorithm.

2. "-r": 
Find number of all the offset blocks containing matched pattern.
Using FM-index to backwards search for the  previous char until reaching the bracket.
Parsing the number before the bracket and save it into a list. Using std::list::sort()
and std::list::unique() to get rid of the duplicates.

3. "-a": 
Find the offset numbers  of all the offset blocks containing matched pattern.
Same as -r option.

## Index file structure:

.idx file is created as a binary file to save the snapshots of occurrence for various blocks.
In this case, all the data is saved in fixed length. It makes the program easier to find the 
formatted information only using the function  std::ifstream::seekg(OFFSET,POSITION).

The file stores is consisted of several int[128] array blocks. The first block of array is C[c]
array for the whole file. Remains are snapshots for each char's occurrence in various blocks.

||C[c]|Occ[char] for 1st blk|Occ[char] for 2st blk|...|Occ[char] for last blk||
|---|---|---|---|---|---|---|
|HEAD|int[128]|int[128]|int[128]|...|int[128] EOF|

	PS. The Occ[] for each char is accumulated.

	To fetch the C[], use std::ifstream::read( (char*) c, sizeof(int[128])),
	To find the Occ[c] in block x, use  
		std::ifstream::seekg( x * sizeof(int[128]) + c * sizeof(int), std::ios::beg);
		std::ifstream::read( (char*) int, sizeof(int));
	where "x * sizeof(int[128]) + c * sizeof(int)" is the OFFSET to the int.


Functions:

	load_file() load the bwt file and count the occurrence of each char while scanning through it.

	find_occ() use the BW_count algorithm from paper "Opportunistic Data Structure with Application".
				Find the occ in the block, and count the rest to add on.

Approach to reduce the time and memory usage:

	Scanning through the whole file could waste a lot of time, which is mostly used by OCC counting.
	So I want to save as much data as I can. With the memory constraints, the only way to do so is save 
	as much infomation asa possible into .idx file. 

	Using BW-count algorithm with file structure above, the size of the file is 
	
		512B * (1 + number_of_blocks)
		where number_of_blocks is FILE_SIZE/BLOCK_SIZE, 512B is the sizeof(int[128])
	
	The index file size cannot be larger than origin file. Thus, under BLOCK_SIZE of 700B, the minimum size of 
	file to use such approach is around 2612B. I pick 3000B as the minimum size of file to use this method.



