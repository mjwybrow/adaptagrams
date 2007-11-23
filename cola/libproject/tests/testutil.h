/**
 * \file testutil.h
 * utilities for testing libproject
 */
#include <map>
#include <cassert>
#ifdef NDEBUG
#define __ASSERT_FUNCTION "XXXXXXXXXXXXCompiled with NDEBUG: function info not availableXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#endif // NDEBUG
/**
 * Solve a project problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T G x + g^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param vs variables
 * @param cs constraints
 * @param result the solution vector
 */
void qps(project::Variables &vs, project::Constraints &cs, 
        std::vector<double> &result);

/**
 * Checks that each constraint is satisfied, throwing an assert error otherwise.
 */
bool feasible(project::Constraints &cs);

/**
 * used to identify variable indices from pointers to Variable
 */
typedef std::map<const project::Variable*,unsigned> VMap;

/**
 * Print out the problem instance (as code that can easily be pasted into
 * a test case)
 */
void printProblem(project::Variables &vs, 
		std::vector<double> &XI, 
		project::Constraints &cs, 
		VMap &vmap);

/**
 * float comparisons (i.e. \f$a=b\$f) in our tests are actually \$f|a-b|<epsilon\$f.
 */
const double epsilon = 1e-10;
/**
 * float comparisons (i.e. \f$a=b\$f) in our tests are actually \$f|a-b|<epsilon\$f.
 * Check approximate equality of two doubles.
 * @return true if the above condition is met
 */
bool approxEquals(double a, double b);
/**
 * get random number in the range [0,max]
 */
double getRand(const double max);
/**
 * Attempt to use libproject to solve a given projection problem instance.
 * @param t pointer to a function that will populate a list of variables and a list
 * of constraints.
 */
void test(const char* (*t)(project::Variables&, project::Constraints&),
		bool silentPass = false);
