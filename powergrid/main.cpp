#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdio.h>

extern "C" {
  #include "lp_lib.h"
}

#define answer std::pair<int, std::string>

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
  E.resize(G);
  int a, b;
  while (i < nE)
  {
      getline(fin, line);
      fin >> a >> b;
      E[a].push_back(b);
      E[b].push_back(a);
      i++;
  }
}

answer _solve_vertex_cover(const int& G, const std::vector<std::vector<int>>& E) {
  lprec* lp;
  lp = make_lp(0, G);
  set_verbose(lp, SEVERE);

  REAL row[G+1];
  for (int i = 1; i <= G; i++)
  {
    char buffer[100];
    sprintf(buffer, "x-%d", i);
    set_col_name(lp, i, buffer);
    set_binary(lp, i, TRUE);
  }

  for (int i = 0; i <= G; i++)
  {
    row[i] = 0;
  }

  int cnt = 0;
  int colno[G+1];
  for (int i = 0; i < G; i++)
  {
    for (auto& e: E[i])
    {
      row[cnt] = 1;
      colno[cnt++] = e+1;
    }
    row[cnt] = 1; colno[cnt++] = i+1;
    add_constraintex(lp,cnt, row,colno, GE, 1);
    for (int i = 0; i <= G; i++)
    {
      row[i] = 0;
    }
    cnt = 0;
  }

  set_add_rowmode(lp, FALSE);

  for (int i = 1; i <= G; i++)
  {
    row[i] = 1;
  }

  set_obj_fn(lp, row);
  set_minim(lp);

  int ret = solve(lp);
  if (ret == 2) return std::make_pair(G+10000, "");

  REAL ans[G];
  get_variables(lp, ans);

  std::string s = ""; int n;
  for (int i = 0; i < G; i++)
  {
    if (ans[i] == 1)
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

int main(int argc, char** argv)
{
  std::ios_base::sync_with_stdio(false); std::cin.tie(NULL);
	if (argc < 3)
	{
		std::cout << "missing args\n";
		exit(1);
	}
  std::string inputpath(argv[1]);
  std::string outputpath(argv[2]);

  int G = 0;
  std::vector<std::vector<int>> E;
  readfile(inputpath, G, E);

  auto ans = _solve_vertex_cover(G, E);

  std::ofstream fout(outputpath);
  
  fout << ans.first << ":" << ans.second;

  fout.close();
}
