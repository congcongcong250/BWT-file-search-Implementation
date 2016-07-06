#include<iostream>
#include<fstream>
#include<algorithm>
#include<cmath>
#include<list>
#include<string>
/*
 *  Comp9319 Data Compression and Search
 *  bwtsearch main.cpp 
 *                         by Liren Wang, Student ID: z5004424
 *
 *  Please find the structure of .idx file in the attached README.txt 
 */
using namespace std;

//@variable small 
//          A boolean value as flag indicating if it is a file smaller than 3KB
//
//@variable blk_size
//          A const global value as block size. For each block of file there is a 
//          snapshot for the occ[char] array which stores the occurrence of the chars
//          before the end of this block.
//
//@variable c[char] 
//          An array stores the C[c] which has the total number of text chars in T
//          which are alphabetically smaller than 'c' 
bool small = false;
const int blk_size = 700;
int *c = new int[128];

//@func  load_bwt
//		load the bwt and saves the snapshots into .idx file
//
//@func  find_occ
//		find the occurrence of a char in a specific position
//
bool load_bwt(ifstream& bwt, char* name, int* c_array);

int find_occ(ifstream& idx, ifstream& bwt, char ch, int pos);

int main(int argc, char* argv[])
{
	//Argument check
	if (argc != 5)
	{
		cout << "We need 4 arguement" << endl;
		return 1;
	}

	//Open the file and keep the ifstream open until program exit
	ifstream bwtfile(argv[2], ios::in);

	//Get the size of bwt file
	streampos begin, end;
	bwtfile.seekg(0, ios::end);
	end = bwtfile.tellg();
	bwtfile.seekg(0, ios::beg);
	begin = bwtfile.tellg();
	const int bwt_size = end - begin;

	small = (bwt_size < 3000);

	fill_n(c, 128, 0);

	if (!small)
	{
		//bwt file more than 3000 bytes (3KB)
		//load the file into .idx
		load_bwt(bwtfile, argv[3], c);
	} else
	{

		//file to small, less than 3KB, load the C[]
		//.idx will not be created and used
		char chr;
		while (bwtfile.get(chr))
		{
			c[(int)chr]++;
		}

		//clear failbit and eofbit
		bwtfile.clear();
		bwtfile.seekg(0, ios::beg);

		int t = 0;
		for (int i = 0; i < 128; ++i)
		{
			if (c[i] == 0)
			{
				c[i] = -1;
			} else
			{
				int tmp = t;
				t = c[i] + t;
				c[i] = tmp;
			}
		}
	}

//	for (int i = 0; i < 128; ++i)
//	{
//		if (c[i] >= 0)
//			cout << (char) i << " :  " << c[i] << endl;
//	}

	string option(argv[1]);
	string pa(argv[4]);

	//Open the ifstream for the .idx file until program exits
	ifstream idx(argv[3], ios::in | ios::binary);

	//Normal implementatino of bwtsearch algorithm
	char ch;
	int j = (int) pa.size() - 1;
	ch = pa[j];

	//Find first and last 
	int first = c[(int)ch];

	int last;
	while (c[ch + 1] == -1)
	{
		ch++;
	}
	if (ch == 127)
	{
		last = bwt_size - 1;
	} else
	{
		last = c[ch + 1] - 1;
	}

	//Loop until reach the final char or find no match
	while (first <= last && j >= 1)
	{
		ch = pa[--j];

		first = c[(int)ch] + find_occ(idx, bwtfile, ch, first - 1);
		last = c[(int)ch] + find_occ(idx, bwtfile, ch, last) - 1;
	}

	if (first > last)
	{
		//Which is not gonna happen
		cout << "No match" << endl;
		bwtfile.close();
		idx.close();
		return 1;
	}

	if (option == "-n")
	{
		//Print the occurrence for Option '-n'
		cout << last - first + 1 << endl;
	} else
	{
		//case -r and -a

		//Create a list to store the offset numbers
		list<int> nbs;

		//Loop all the possibility of the pattern matched, from first to last
		for (int i = first; i <= last; ++i)
		{

			//@variable same_part: indicates if two pattern is in the same offset block
			bool same_part = false;
			char prev;

			//@variable offset:  It is the offset to find the char in bwt file, not the offset in the file.
			int offset = i;
			int nb = 0;
			do
			{

				bwtfile.seekg(offset, ios::beg);
				bwtfile.get(prev);
				bwtfile.clear();
				bwtfile.seekg(0, ios::beg);
				offset = c[(int)prev] + find_occ(idx, bwtfile, prev, offset) - 1;
				//cout << prev;//debug
				
				//If the previous char reaches a char in the previous matched pattern, ignore it 
				if (i > offset && offset >= first)
				{

					same_part = true;
					break;
				}
			} while (prev != ']');

			if (same_part)
				continue;

			int count = 0;

			bwtfile.seekg(offset, ios::beg);
			bwtfile.get(prev);
			bwtfile.clear();
			bwtfile.seekg(0, ios::beg);
			offset = c[(int)prev] + find_occ(idx, bwtfile, prev, offset) - 1;

			//Reached the bracket, find the offset number 
			while (prev != '[')
			{
				nb += (prev - 48) * pow(10, count++);
				//cout << prev; //debug
				bwtfile.seekg(offset, ios::beg);
				bwtfile.get(prev);
				bwtfile.clear();
				bwtfile.seekg(0, ios::beg);
				offset = c[(int)prev] + find_occ(idx, bwtfile, prev, offset) - 1;
			}
			//cout << "  push nb:" << nb; //debug
			//cout << endl;
			nbs.push_back(nb);
		}

		//Sort the list and get rid of the possible duplicates
		nbs.sort();
		nbs.unique();
		if (option == "-r")
		{
			cout << nbs.size() << endl;

			bwtfile.close();
			idx.close();
			return 0;
		}

		int size = nbs.size();
		for (int i = 0; i < size; ++i)
		{
			cout << "[" << nbs.front() << "]" << endl;
			nbs.pop_front();
		}

	}

	bwtfile.close();
	idx.close();
}

bool load_bwt(ifstream& bwt, char* name, int* c_array)
{
	bool load = false;
	if (!ifstream(name))
	{
		//the argv[3] file does not exist, create it and load the data into file

		ofstream oidx(name, ios::out | ios::binary);
		int tmp[128];
		oidx.write((char*) tmp, sizeof(int[128]));

		char chr;
		int *tmp_occ = new int[128];
		fill_n(tmp_occ, 128, 0);
		int count = 0;

		while (bwt.get(chr))
		{
			tmp_occ[(int)chr]++;
			//clock size 700
			////position in file should be 699, 1399, 2099, 2799 and so.
			if (++count == blk_size)
			{
				//write the current occ into idx file
				//zero count
				oidx.write((char*) tmp_occ, sizeof(int[128]));
				count = 0;
			}
		}
		bwt.clear();
		bwt.seekg(0, ios::beg);

		//Convert the final occ[] into C[], and write it into the beginning of the file
		int t = 0;
		for (int i = 0; i < 128; ++i)
		{
			if (tmp_occ[i] == 0)
			{
				c_array[i] = -1;
			} else
			{
				int tmp = t;
				t = tmp_occ[i] + t;
				c_array[i] = tmp;
			}

		}

		load = true;
		oidx.seekp(0, ios::beg);
		oidx.write((char*) c_array, sizeof(int[128]));
		oidx.close();

		delete[] tmp_occ;

	}

	//if the idx is exist, load the first 512 bytes into C[]
	if (!load)
	{
		ifstream idx(name, ios::in | ios::binary);
		idx.read((char*) c_array, sizeof(int[128]));
		idx.close();
	}

	return true;
}

int find_occ(ifstream& idx, ifstream& bwt, char ch, int pos)
{
	int res = 0;
	if (small)
	{
		//If it is a small file, simply count it through the .bwt file
		char chr;
		for (int i = 0; i <= pos; ++i)
		{
			bwt.get(chr);
			if (chr == ch)
			{
				res++;
			}
		}
		bwt.clear();
		bwt.seekg(0, ios::beg);
	} else
	{
		//If there is occ[] snapshots in the .idx file, find the corresponding one 
		//and count the rest occurrence in the next block
		int blk_nb = (pos + 1) / blk_size;
		int offset = pos + 1;
		if (blk_nb != 0)
		{

			idx.seekg(blk_nb * sizeof(int[128]) + ch * sizeof(int), ios::beg);
			idx.read((char*) &res, sizeof(int));

			bwt.seekg(blk_nb * blk_size, ios::beg);
			offset = (pos + 1) % blk_size;
			idx.clear();
			idx.seekg(0, ios::beg);
		}

		char chr;
		for (int i = 0; i < offset; ++i)
		{
			bwt.get(chr);
			if (chr == ch)
			{
				res++;
			}
		}
		bwt.clear();
		bwt.seekg(0, ios::beg);

	}

	return res;
}
