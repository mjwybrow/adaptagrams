#ifndef SEEN_LIBVPSC_EXCEPTIONS_H
#define SEEN_LIBVPSC_EXCEPTIONS_H
#include <vector>
namespace vpsc {
class Constraint;
struct UnsatisfiableException {
	std::vector<Constraint*> path;
};
struct UnsatisfiedConstraint {
	UnsatisfiedConstraint(Constraint& c):c(c) {}
	Constraint& c;
};
} // namespace vpsc
#endif //SEEN_LIBVPSC_EXCEPTIONS_H
