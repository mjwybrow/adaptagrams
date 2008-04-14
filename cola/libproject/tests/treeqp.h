#include <vector>
/**
 * our goal function has terms:
 *   w_i(x_p+o_i-x_c)^2
 * where x_p is the position of the "parent", x_c is the "child" position,
 * w_i is a weight for the ith term and o_i is the offset required between
 * them.
 */
struct Term {
    unsigned p, c;
    double w, o; 
    Term(unsigned p, unsigned c, double w, double o) 
        : p(p), c(c), w(w), o(o) {
    }
};
typedef std::vector<Term> Terms;
/**
 * we solve the goal subject to constraints of the form 
 *   x_l+g<=x_r or x_l+g=x_r
 */
struct Constraint {
    unsigned l, r;
    /// gap required between variables
    double g;
    /// true if this is an equality constraint, otherwise it's an inequality
    bool eq;
    Constraint(unsigned l, unsigned r, double g, bool eq) 
        : l(l), r(r), g(g), eq(eq) {
    }
};
typedef std::vector<Constraint> Constraints;
/**
 * Solve problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T A x + b^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param ts goal terms
 * @param cs constraints
 * @param result the solution vector
 */
void treeQPSolve(Terms &ts, Constraints &cs, std::vector<double> &result);
