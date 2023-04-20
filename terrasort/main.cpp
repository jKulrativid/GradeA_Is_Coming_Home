#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <execution>

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

void _read(std::string filepath, size_t readstart, size_t chunk_size, std::vector<std::pair<std::string, double>>* v)
{
	std::vector<char> rawVals;
	rawVals.reserve(chunk_size + 100);
	std::ifstream fin(filepath.c_str());
	fin.seekg(readstart);
	while (fin.peek() != '\n')
	{
		char nextChar;
		fin.get(nextChar);
		chunk_size--;
	}
	char _nextch; fin.get(_nextch);
	rawVals.resize(chunk_size);
	for (int i = 0; i < chunk_size; i++)
	{
		fin.get(rawVals[i]);
	}

	// handle splittng between the line
	const size_t nChar = rawVals.size();
	if (rawVals[nChar-1] != '\n')
	{
		while (fin.peek() != EOF && fin.peek() != '\n')
		{
			char nextChar;
			fin.get(nextChar);
			rawVals.push_back(nextChar);
		}
	}
	rawVals.push_back('\n');

	size_t cnt = 0;
	char buf[200];
	char keybuf[2000];
	bool passSpace = false;
	for (char& c: rawVals)
	{
		if (passSpace)
		{
			
			if (c == '\n')
			{
				v->push_back(std::make_pair(std::string(keybuf), _parse(buf)));
				cnt = 0;
				passSpace = false;
			}
			else
			{
				buf[cnt] = c;
				cnt++;
			}
		}
		else
		{
			if (c == ' ')
			{
				passSpace = true;
			}
			else if (c == ':')
			{
				cnt = 0;
			}
			else
			{
				keybuf[cnt] = c;
				cnt++;
			}
		}
	}
}

void _merge(std::vector<std::pair<std::string, double>>& v, std::vector<std::pair<std::string, double>>& frag, const size_t start, const size_t end)
{
	auto itr = frag.begin();
	for (size_t i = start; i < end; i++)
	{
		v[i] = *itr;
		itr++;
	}
}

inline void read_file(const std::string& filepath, std::vector<std::pair<std::string, double>>* result)
{
	const size_t fileSize = getFileSize(filepath);
	if (fileSize < (1<<20) && false)
	{ // if size < 1MB just read it
		const size_t readStart = 0;
		_read(filepath, readStart, fileSize, result);
	}
	else
	{
		std::vector<std::vector<std::pair<std::string, double>>> unmergedResult(N_THRAED);
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

inline void write_file(const std::vector<std::pair<std::string, TYPE>>& v, const std::string& outputpath)
{
	std::ofstream fout(outputpath);

	for (auto& x: v)
	{
		fout << x.first << ": " << x.second << "\n";
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
		nthreadChar = "1";
	}
	N_THRAED = atoi(nthreadChar);
	omp_set_num_threads(N_THRAED);

	std::string inputpath(argv[1]), outputpath(argv[2]);
	std::cout << inputpath << " | " << outputpath << std::endl;
	
	std::vector<std::pair<std::string, TYPE>> v;

	read_file(inputpath, &v);

	std::sort(std::execution::par_unseq, v.begin(), v.end(), \
		[ ]( const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs )
	{
		return lhs.second < rhs.second;
	});

	write_file(v, outputpath);

	// END TIME
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

	std::exit(0);
}
