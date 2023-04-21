#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <execution>
#include <stdio.h>

// number of pre-allocated vector slots = (file_size / READ_MAGIC_DIVISOR)/ N_THREAD
#define READ_MAGIC_DIVISOR 6
#define TYPE double

int N_THRAED; // assign in runtime (main func)

inline size_t getFileSize(const std::string& filepath)
{
	std::filesystem::path path(filepath);
	std::filesystem::file_status status = std::filesystem::status(path);
	if (std::filesystem::is_regular_file(status)) {
		return static_cast<size_t>(std::filesystem::file_size(path));
	}
	std::cout << "!!! not a file !!!" << std::endl;
	return 0;
}

// this algorithm is volatile to change due to optimization
TYPE _parse(char* char_ptr)
{
	if (typeid(TYPE) == typeid(double))
	{
		TYPE x =  std::strtof(char_ptr, nullptr);
		return x;
	}
	return atoi(char_ptr);
}

void _read(std::string filepath, size_t readstart, size_t chunk_size, std::vector<std::pair<int, double>>* v)
{
	std::ifstream fin(filepath.c_str());
	fin.seekg(readstart);
	while (fin.peek() != 's' )
	{
		char nextChar;
		fin.get(nextChar);
		chunk_size--;
	}

	size_t cnt = 0;
	char c;
	char buf[200];
	char keybuf[200];
	bool passSpace = false, passdash = false;
	for (size_t i = 0; i < chunk_size; i++)
	{
		fin.get(c);
		if (passSpace)
		{
			if (c == '\n')
			{
				v->push_back(std::make_pair(atoi(keybuf), _parse(buf)));
				cnt = 0;
				passSpace = false;
			}
			else
			{
				buf[cnt] = c;
				cnt++;
			}
		}
		else if (passdash)
		{
			if (c == ' ')
			{
				passdash = false;
				passSpace = true;
				cnt = 0;
			}
			else if (c != ':')
			{
				keybuf[cnt] = c;
				cnt++;
			}
		}
		else if (c == '-')
		{
			passdash = true;
		}
	}
	
	if (c != '\n' && fin.peek() != EOF)
	{
		while (fin.peek() != EOF && fin.peek() != 's')
		{
			fin.get(c);
			if (passSpace)
			{
				if (c == '\n')
				{
					v->push_back(std::make_pair(atoi(keybuf), _parse(buf)));
					cnt = 0;
					passSpace = false;
				}
				else
				{
					buf[cnt] = c;
					cnt++;
				}
			}
			else if (passdash)
			{
				if (c == ' ')
				{
					passdash = false;
					passSpace = true;
					cnt = 0;
				}
				else if (c != ':')
				{
					keybuf[cnt] = c;
					cnt++;
				}
			}
			else if (c == '-')
			{
				passdash = true;
			}
		}
	}

	fin.close();
}

void _merge(std::vector<std::pair<int, double>>& v, std::vector<std::pair<int, double>>& frag, const size_t start, const size_t end)
{
	auto itr = frag.begin();
	for (size_t i = start; i < end; i++)
	{
		v[i] = std::move(*itr);
		itr++;
	}
}

inline void read_file(const std::string& filepath, std::vector<std::pair<int, double>>* result)
{
	const size_t fileSize = getFileSize(filepath);
	if (fileSize < (1<<20) && false)
	{ // if size < 1MB just read it
		const size_t readStart = 0;
		_read(filepath, readStart, fileSize, result);
	}
	else
	{
		std::vector<std::vector<std::pair<int, double>>> unmergedResult(N_THRAED);
		size_t chunkSize = fileSize / (size_t) N_THRAED;
		size_t readStart = 0;
		#pragma omp parallel for
		for (int t = 0; t < N_THRAED; t++)
		{
			unmergedResult[t].reserve(chunkSize / READ_MAGIC_DIVISOR);
			_read(filepath, readStart + t*chunkSize, chunkSize, &unmergedResult[t]);
		}
		
		size_t n_total = 0;
		std::vector<size_t> n_eachs_cumulative(N_THRAED);
		for (int t = 0; t < N_THRAED; t++)
		{
			n_total += unmergedResult[t].size();
			if (t == 0) 
			{
				n_eachs_cumulative[t] = unmergedResult[t].size();
			}
			else
			{
				n_eachs_cumulative[t] = n_eachs_cumulative[t-1] + unmergedResult[t].size();
			}
		}

		result->resize(n_total);

		#pragma omp parallel for
		for (int t = 0; t < N_THRAED; t++) {
			_merge(std::ref(*result), std::ref(unmergedResult[t]), n_eachs_cumulative[t] - unmergedResult[t].size(),  n_eachs_cumulative[t]); 
		}
	}
}

size_t _count_char(const std::vector<std::pair<int, TYPE>>& v, const size_t start, const size_t chunk_size)
{
	size_t result = 0;
	char buffer[250];
	for (size_t i = start; i < chunk_size + start; i++)
	{
		result += static_cast<size_t>(sprintf(buffer, "std-%d: %.15f\n", v[i].first, v[i].second));
	}
	return result;
}

void _write(const std::vector<std::pair<int, TYPE>>& v, const std::string& outputpath, const size_t offset, const size_t start, const size_t end)
{
	std::fstream fout(outputpath);

	fout.seekg(offset);

	char buffer[40];
	for (size_t i = start; i < end; i++)
	{
		const size_t n = sprintf(buffer, "std-%d: %.15f\n", v[i].first, v[i].second);
		fout.write(buffer, n);
	}

	fout.close();
}

inline void write_file(const std::vector<std::pair<int, TYPE>>& v, const std::string& outputpath)
{
	std::ofstream fout(outputpath); fout.close();

	std::vector<size_t> chunk_sizes(N_THRAED), prefix_chunk_size(N_THRAED);
	size_t n_chunk = (v.size()/N_THRAED);

	#pragma omp parallel for
	for (int t = 0; t < N_THRAED; t++)
	{
		size_t frag = (t+1 == N_THRAED) ? (v.size() % N_THRAED) : 0;
		size_t start = t*n_chunk, end = start + n_chunk + frag;
		chunk_sizes[t] = _count_char(v, start, n_chunk);
	}

	std::cout << "--- COUNT CHAR COMPLETED ---" << std::endl;

	prefix_chunk_size[0] = 0;
	for (int t = 1; t < N_THRAED; t++)
	{
		prefix_chunk_size[t] = prefix_chunk_size[t-1] + chunk_sizes[t-1];
	}

	#pragma omp parallel for
	for (int t = 0; t < N_THRAED; t++)
	{
		size_t frag = (t+1 == N_THRAED) ? (v.size() % N_THRAED) : 0;
		size_t start = t*n_chunk, end = start + n_chunk + frag;
		_write(v, outputpath, prefix_chunk_size[t], start, end);
	}
}

int main(int argc, char** argv)
{
	// START TIME
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if (argc < 3)
	{
		std::cout << "missing arg" << std::endl;
		std::exit(1);
	}
	std::ios_base::sync_with_stdio(false); std::cin.tie(NULL);
	auto nthreadChar = std::getenv("NTHREAD");
	if (!nthreadChar)
	{
		nthreadChar = "12";
	}
	N_THRAED = atoi(nthreadChar);
	omp_set_num_threads(N_THRAED);
	std::cout << "N THread =" << N_THRAED << std::endl;

	std::string inputpath(argv[1]), outputpath(argv[2]);
	std::cout << inputpath << " | " << outputpath << std::endl;
	
	std::vector<std::pair<int, TYPE>> v;

	std::cout << "--- SETUP COMPLETED ---" << std::endl;

	read_file(inputpath, &v);

	std::cout << "--- READ COMPLETED ---" << std::endl;

	std::sort(std::execution::par_unseq, v.begin(), v.end(), \
		[ ]( const std::pair<int, double>& lhs, const std::pair<int, double>& rhs )
	{
		return lhs.second < rhs.second;
	});

	std::cout << "--- SORT COMPLETED ---" << std::endl;

	write_file(v, outputpath);

	std::cout << "--- WRITE COMPLETED ---" << std::endl;

	// END TIME
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

	std::exit(0);
}
