#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <stdio.h>

extern "C" {
  #include "lp_lib.h"
}

#define answer std::pair<int, std::string>

int N_THREAD;
int N_PREFIX;

void _calc_prefix()
{
  int n = N_THREAD + 1;
  N_PREFIX = 0;
  while (1)
  {
    n >>= 1;
    if (!n) break;
    N_PREFIX++;
  }
}

void readfile(const std::string& filename, int& G, std::vector<std::vector<int>>& E)
{
  int nE;
  std::ifstream fin(filename); std::string line;
  if(!fin)
  {
    std::cout <<  "could not open file" << std::endl;
    std::exit(1);
  } 
  fin >> G;
  getline(fin, line);
  fin >> nE;
  int i = 0;
  E.resize(nE);
  int a, b;
  while (i < nE)
  {
      getline(fin, line);
      fin >> a >> b;
      E[i].resize(2);
      E[i][0] = a; E[i][1] = b;
      i++;
  }
}

answer _solve_vertex_cover(const int& G, const std::vector<std::vector<int>>& E,  const int& tid) {
  lprec* lp;
  lp = make_lp(0, G);
  set_verbose(lp, IMPORTANT);

  double row[G+1]; int col[G+1];
  for (int i = 1; i <= G; i++)
  {
    char buffer[100];
    col[i] = i;
    sprintf(buffer, "x-%d", i);
    set_col_name(lp, i, buffer);
    set_int(lp, i, TRUE);
  }

  for (int i = 1; i <= G; i++)
  {
    row[i] = 0;
  }

  for (int i = 0; i < E.size(); i++)
  {
    row[E[i][0]+1] = 1; row[E[i][1]+1] = 1;
    add_constraint(lp, row, GE, 1);
    row[E[i][0]+1] = 0; row[E[i][1]+1] = 0;
  }

  for (int i = 1; i <= G; i++)
  {
    row[i] = 1;
    add_constraint(lp, row, GE, 0);
    row[i] = -1;
    add_constraint(lp, row, GE, -1);
    row[i] = 0;
  }

  // MORE CONSTRAINT DUE TO THREAD

  // END MORE CONSTRAINT

  set_add_rowmode(lp, FALSE);

  for (int i = 1; i <= G; i++)
  {
    row[i] = 1;
  }

  set_obj_fn(lp, row);
  set_minim(lp);
  
  int ret = solve(lp);

  get_variables(lp, row);

  std::string s = ""; int n;
  for (int i = 0; i < G; i++)
  {
    if (row[i] == 1)
    {
      s += "1";
      n++;
    }
    else
    {
      s += "0";
    }
  }

  delete_lp(lp);
  
  return std::make_pair(n, s);
}

answer* solve_vertex_cover(const int& G, const std::vector<std::vector<int>>& E)
{
  std::vector<answer> ans(N_THREAD);
  #pragma omp parallel for
  for (int t = 0; t < N_THREAD; t++)
  {
    ans[t] = _solve_vertex_cover(std::ref(G), std::ref(E), std::ref(t));
  }

  answer tempbest = { .n = G+1000, .seq = "" };
  answer* best = &tempbest;
  for (int t = 0; t < N_THREAD; t++)
  {
    
  }

  return best;
}

int main(int argc, char** argv)
{
  std::ios_base::sync_with_stdio(false); std::cin.tie(NULL);
	if (argc < 3)
	{
		std::cout << "missing args\n";
		exit(1);
	}
	char* nthreadChar = getenv("NTHREAD");
	if (!nthreadChar)
	{
		nthreadChar = "12";
	}
	N_THREAD = atoi(nthreadChar);
	omp_set_num_threads(N_THREAD);
  
	std::cout << "N THread = " << N_THREAD << std::endl;

  std::string inputpath(argv[1]);
  std::string outputpath(argv[2]);
  std::cout << "I/0 : " << inputpath << "|" << outputpath << std::endl;

  _calc_prefix();

	std::cout << "--- SETUP COMPLETED ---\n";

  int G = 0;
  std::vector<std::vector<int>> E;
  readfile(inputpath, G, E);

  solve_vertex_cover(G, E);
}
