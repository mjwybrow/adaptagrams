#include <map>
#include <valarray>
#include <libcola/sparse_matrix.h>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <time.h>
#include <cmath>

using namespace boost::numeric::ublas;
int sparse_test(const unsigned n, cola::SparseMatrix::SparseMap& cm, mapped_matrix<double>& bm) {
    printf("Sparse test...");
    srand(time(0));
    for (unsigned i = 0; i < n; ++ i) {
        for (unsigned j = 0; j < n; ++ j) {
            double r=(double)rand()/(double)RAND_MAX;
            double s=(double)rand()/(double)RAND_MAX;
            if(r < 0.2) {
                cm[std::make_pair(i,j)] = s;
                bm (i, j) = s;
            }
        }
    }
}
int dense_test(const unsigned n, cola::SparseMatrix::SparseMap& cm, mapped_matrix<double>& bm) {
    printf("Dense test...");
    for (unsigned i = 0; i < n; ++ i) {
        for (unsigned j = 0; j < n; ++ j) {
            bm (i, j) = n * i + j;
            cm[std::make_pair(i,j)]=bm(i,j);
        }
    }
}

void test(int (*generate)(const unsigned, cola::SparseMatrix::SparseMap&, mapped_matrix<double>&)) {
    using std::valarray;
    const unsigned n = 20;
    mapped_matrix<double> bm (n, n, n * n);
    cola::SparseMatrix::SparseMap cm;
    generate(n,cm,bm);
    vector<double> bv (n);
    valarray<double> cv(n);
    for (unsigned i = 0; i < n; ++ i) {
        bv(i) = i;
        cv[i] = i;
    }
    vector<double> br(n);
    valarray<double> cr(n);
    cola::SparseMatrix a(cm,n);
    br=prod(bm,bv);
    a.rightMultiply(cv,cr);
    for (unsigned i = 0; i < n; ++ i) {
        assert(fabs(cr[i]-br(i))<0.000001);
    }
    printf(" passed!\n");
}
int main() {
    test(&dense_test);
    test(&sparse_test);
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
