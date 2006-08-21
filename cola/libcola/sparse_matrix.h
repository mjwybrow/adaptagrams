#ifndef _SPARSE_MATRIX_H
#define _SPARSE_MATRIX_H
#include <valarray>
namespace cola {
    using std::valarray;
class SparseMatrix {
public:
    typedef std::pair<unsigned, unsigned> SparseIndex;
    typedef std::map<SparseIndex,double> SparseMap;
    SparseMatrix(SparseMap const & m, const unsigned n)
            : n(n), NZ(m.size()), sparseMap(m), 
              A(valarray<double>(NZ)), IA(valarray<unsigned>(n+1)), JA(valarray<unsigned>(NZ)) {
        unsigned cnt=0;
        int lastrow=-1;
        for(SparseMap::const_iterator i=m.begin(); i!=m.end(); i++) {
            SparseIndex p = i->first;
            A[cnt]=i->second;
            if(p.first!=lastrow) {
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
    void rightMultiply(valarray<double> const & v, valarray<double> & r) {
        for(unsigned i=0;i<n;i++) {
            r[i]=0;
            for(unsigned j=IA[i];j<IA[i+1];j++) {
                r[i]+=A[j]*v[JA[j]];
            }
        }
    }
    double getIJ(const unsigned i, const unsigned j) {
        return sparseMap.find(std::make_pair(i,j))->second;
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
