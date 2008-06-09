/*
 File main.cc
 
 This file contains just an example on how to set-up the matrices for using with
 the solve_quadprog() function.
 
 The test problem is the following:
 
 Given:
 G =  4 -2   g0^T = [6 0]
     -2  4       
 
 Solve:
 min f(x) = 1/2 x G x + g0 x
 s.t.
   x_1 + x_2 = 3
   x_1 >= 0
   x_2 >= 0
   x_1 + x_2 >= 2
 
 The solution is x^T = [1 2] and f(x) = 12
 
 Author: Luca Di Gaspero
 DIEGM - University of Udine, Italy
 l.digaspero@uniud.it
 http://www.diegm.uniud.it/digaspero/
 
 LICENSE
 
 Copyright 2006 Luca Di Gaspero
 
 This file is part of QuadProg++.
 
 QuadProg++ is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 QuadProg++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with QuadProg++; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <iostream>
#include <sstream>
#include <string>
#include "QuadProg++.h"

int main (int argc, char *const argv[]) {
  int n=2, m=1, p=3;
  double sum = 0.0;
  char ch;
  double **G, *g0, **CE, *ce0, **CI, *ci0, *x;
  
  MatrixStorage _G(G,n,n);
  VectorStorage<double> _g(g0,n);
  MatrixStorage _CE(CE,n,m);
  VectorStorage<double> _ce(ce0,m);
  MatrixStorage _CI(CI,n,p);
  VectorStorage<double> _ci(ci0,p);
  VectorStorage<double> _x(x,n);
  
  std::istringstream Gis("4, -2,"
                         "-2, 4 ");
  std::istringstream g0is("6.0, 0.0 ");
  std::istringstream CEis("1.0, "
                          "1.0 ");
  std::istringstream CIis("1.0, 0.0, 1.0, "
                          "0.0, 1.0, 1.0 ");
  std::istringstream ce0is("-3.0 ");
  std::istringstream ci0is("0.0, 0.0, -2.0 ");

  for (int i = 0; i < n; i++) {
    G[i]=new double[n];
    g0is >> g0[i] >> ch;
    for (int j = 0; j < n; j++)
      Gis >> G[i][j] >> ch;
    CE[i]=new double[m];
    for (int j = 0; j < m; j++)
      CEis >> CE[i][j] >> ch;
    CI[i]=new double[p];
    for (int j = 0; j < p; j++)
      CIis >> CI[i][j] >> ch;
  }
  for (int j = 0; j < m; j++)
    ce0is >> ce0[j] >> ch;
  for (int j = 0; j < p; j++)
    ci0is >> ci0[j] >> ch;

  std::cout << "f: " << solve_quadprog(G, g0, n, CE, ce0, m, CI, ci0, p, x) << std::endl;
  std::cout << "x: ";
  for (int i = 0; i < n; i++)
    std::cout << x[i] << ' ';
  std::cout << std::endl; 

  /* FOR DOUBLE CHECKING COST since in the solve_quadprog routine the matrix G is modified */
  
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      Gis >> G[i][j] >> ch;
  
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      sum += x[i] * G[i][j] * x[j];
  sum *= 0.5;
  
  std::cout << "Double checking cost: ";
  for (int i = 0; i < n; i++)
    sum += g0[i] * x[i];
  std::cout << sum << std::endl;
}
