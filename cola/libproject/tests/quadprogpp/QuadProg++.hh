
#ifndef _QUADPROGPP
#define _QUADPROGPP

/*
 File QuadProg++.hh
 
 The quadprog_solve() function implements the algorithm of Goldfarb and Idnani 
 for the solution of a (convex) Quadratic Programming problem
by means of a dual method.
	 
The problem is in the form:

min 0.5 * x G x + g0 x
s.t.
    CE^T x + ce0 = 0
    CI^T x + ci0 >= 0
	 
 The matrix and vectors dimensions are as follows:
     G: n * n
		g0: n
				
		CE: n * p
	 ce0: p
				
	  CI: n * m
   ci0: m

     x: n
 
 The function will return the cost of the solution written in the x vector or
 std::numeric_limits::infinity() if the problem is infeasible. In the latter case
 the value of the x vector is not correct.
 
 References: D. Goldfarb, A. Idnani. A numerically stable dual method for solving
             strictly convex quadratic programs. Mathematical Programming 27 (1983) pp. 1-33.

 Notes:
  1. pay attention in setting up the vectors ce0 and ci0. 
	   If the constraints of your problem are specified in the form 
	   A^T x = b and C^T x >= d, then you should set ce0 = -b and ci0 = -d.
  2. The matrices have column dimension equal to MATRIX_DIM, 
     a constant set to 20 in this file (by means of a #define macro). 
     If the matrices are bigger than 20 x 20 the limit could be
		 increased by means of a -DMATRIX_DIM=n on the compiler command line.
  3. The matrix G is modified within the function since it is used to compute
     the G = L^T L cholesky factorization for further computations inside the function. 
     If you need the original matrix G you should make a copy of it and pass the copy
     to the function.
    
 Author: Luca Di Gaspero
  			 DIEGM - University of Udine, Italy
				 l.digaspero@uniud.it
				 http://www.diegm.uniud.it/digaspero/
 
 The author will be grateful if the researchers using this software will
 acknowledge the contribution of this function in their research papers.

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

#ifndef MATRIX_DIM
#define MATRIX_DIM 20
#endif

double solve_quadprog(double G[][MATRIX_DIM], double g0[], int n, 
                      double CE[][MATRIX_DIM], double ce0[], int p, 
                      double CI[][MATRIX_DIM], double ci0[], int m,
                      double x[]);
#endif // #define _QUADPROGPP

