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
#include "QuadProg++.hh"

int main (int argc, char *const argv[]) {
	double G[MATRIX_DIM][MATRIX_DIM], g0[MATRIX_DIM], 
		CE[MATRIX_DIM][MATRIX_DIM], ce0[MATRIX_DIM], 
		CI[MATRIX_DIM][MATRIX_DIM], ci0[MATRIX_DIM], 
		x[MATRIX_DIM];
	int n, m, p;
	double sum = 0.0;
	char ch;
  
  n = 2;
  {
		std::istringstream is("4, -2,"
													"-2, 4 ");

		for (int i = 0; i < n; i++)	
			for (int j = 0; j < n; j++)
				is >> G[i][j] >> ch;
	}
	
  {
		std::istringstream is("6.0, 0.0 ");

		for (int i = 0; i < n; i++)
			is >> g0[i] >> ch;
	}
  
  m = 1;
	{
		std::istringstream is("1.0, "
													"1.0 ");

		for (int i = 0; i < n; i++)
			for (int j = 0; j < m; j++)
				is >> CE[i][j] >> ch;
	} 
  
	{
		std::istringstream is("-3.0 ");
		
		for (int j = 0; j < m; j++)
			is >> ce0[j] >> ch;
  }
	
	p = 3;
  {
		std::istringstream is("1.0, 0.0, 1.0, "
													"0.0, 1.0, 1.0 ");
  
		for (int i = 0; i < n; i++)
			for (int j = 0; j < p; j++)
				is >> CI[i][j] >> ch;
	}
  
  {
		std::istringstream is("0.0, 0.0, -2.0 ");

		for (int j = 0; j < p; j++)
			is >> ci0[j] >> ch;
	}

  std::cout << "f: " << solve_quadprog(G, g0, n, CE, ce0, m, CI, ci0, p, x) << std::endl;
	std::cout << "x: ";
  for (int i = 0; i < n; i++)
    std::cout << x[i] << ' ';
	std::cout << std::endl;	

	/* FOR DOUBLE CHECKING COST since in the solve_quadprog routine the matrix G is modified */
	
	{
    std::istringstream is("4, -2,"
													"-2, 4 ");
	
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				is >> G[i][j] >> ch;
	}
	
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			sum += x[i] * G[i][j] * x[j];
	sum *= 0.5;
	
	std::cout << "Double checking cost: ";
	for (int i = 0; i < n; i++)
		sum += g0[i] * x[i];
	std::cout << sum << std::endl;
}
