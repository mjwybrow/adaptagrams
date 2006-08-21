#ifndef _SPARSE_MATRIX_H
#define _SPARSE_MATRIX_H
namespace cola {
class SparseMatrix {
public:
    typedef std::pair<unsigned, unsigned> SparseIndex;
    typedef std::map<SparseIndex,double> SparseMap;
    SparseMatrix(SparseMap& m, const unsigned n)
            : n(n), NZ(m.size()), sparseMap(m), 
              A(new double[NZ]), IA(new unsigned[n+1]), JA(new unsigned[NZ]) {
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
    ~SparseMatrix() {
        delete [] A;
        delete [] IA;
        delete [] JA;
    }
    void rightMultiply(const double* v, double* r) {
        for(unsigned i=0;i<n;i++) {
            r[i]=0;
            for(unsigned j=IA[i];j<IA[i+1];j++) {
                r[i]+=A[j]*v[JA[j]];
            }
        }
    }
    double getIJ(const unsigned i, const unsigned j) {
        return sparseMap[std::make_pair(i,j)];
    }

private:
    const unsigned n,NZ;
    SparseMap& sparseMap;
    double* A;
    unsigned *IA, *JA;
};
} //namespace cola
#endif /* _SPARSE_MATRIX_H */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
