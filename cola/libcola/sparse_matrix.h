/**
 * \file sparse_matrix.h
 * \author Tim Dwyer
 * \date 2007
 */
#ifndef _SPARSE_MATRIX_H
#define _SPARSE_MATRIX_H
#include <valarray>
#include <map>
#include <iostream>
namespace cola {
using std::valarray;
struct SparseMap {
    SparseMap(unsigned n = 0) : n(n) {};
    unsigned n;
    typedef std::pair<unsigned, unsigned> SparseIndex;
    typedef std::map<SparseIndex,double> SparseLookup;
    typedef SparseLookup::const_iterator ConstIt;
    SparseLookup lookup;
    double& operator[](const SparseIndex& k) {
        return lookup[k];
    }
    double& operator()(const unsigned i, const unsigned j) {
        return lookup[std::make_pair(i,j)]; 
    }
    double getIJ(const unsigned i, const unsigned j) const {
        assert(i<n);
        assert(j<n);
        ConstIt v=lookup.find(std::make_pair(i,j));
        if(v!=lookup.end()) {
            return v->second;
        }
        return 0;
    }
    unsigned nonZeroCount() const {
        return lookup.size();
    }
    void resize(unsigned n) {
        this->n = n;
    }
};
/**
 * Yale Sparse Matrix implementation (from Wikipedia definition).
 * It stores an initial sparse n×n matrix M in row form using three arrays, A,
 * IA, JA. NZ denotes the number of nonzero entries in matrix M. The array A
 * then is of length NZ and holds all nonzero entries of M. The array IA stores
 * at IA(i) the position of the first element of row i in the sparse array A.
 * The length of row i is determined by IA(i+1) - IA(i). Therefore IA needs to
 * be of length N + 1. In array JA, the column index of the element A(j) is
 * stored. JA is of length NZ.
 */
class SparseMatrix {
public:
    SparseMatrix(SparseMap const & m)
            : n(m.n), NZ(m.nonZeroCount()), sparseMap(m), 
              A(valarray<double>(NZ)), IA(valarray<unsigned>(n+1)), JA(valarray<unsigned>(NZ)) {
        unsigned cnt=0;
        int lastrow=-1;
        for(SparseMap::ConstIt i=m.lookup.begin(); i!=m.lookup.end(); i++) {
            SparseMap::SparseIndex p = i->first;
            assert(p.first<n);
            assert(p.second<n);
            A[cnt]=i->second;
            if((int)p.first!=lastrow) {
                for(unsigned r=lastrow+1;r<=p.first;r++) {
                    IA[r]=cnt;
                }
                lastrow=p.first;
            }
            JA[cnt]=p.second;
            cnt++;
        }
        for(unsigned r=lastrow+1;r<=n;r++) {
            IA[r]=NZ;
        }
    }
    void rightMultiply(valarray<double> const & v, valarray<double> & r) const {
        assert(v.size()>=n);
        assert(r.size()>=n);
        for(unsigned i=0;i<n;i++) {
            r[i]=0;
            for(unsigned j=IA[i];j<IA[i+1];j++) {
                r[i]+=A[j]*v[JA[j]];
            }
        }
    }
    double getIJ(const unsigned i, const unsigned j) const {
        return sparseMap.getIJ(i,j);
    }
    void print() const {
        for(unsigned i=0;i<n;i++) {
            for(unsigned j=0;j<n;j++) {
                printf("%f ",getIJ(i,j));
            }
            printf("\n");
        }
    }
    unsigned rowSize() const {
        return n;
    }
private:
    const unsigned n,NZ;
    SparseMap const & sparseMap;
    valarray<double> A;
    valarray<unsigned> IA, JA;
};
} //namespace cola
#endif /* _SPARSE_MATRIX_H */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
