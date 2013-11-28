/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):  Tim Dwyer
*/

/*
 * Yale Sparse Matrix implementation (from Wikipedia definition).
 * It stores an initial sparse n×n matrix M in row form using three arrays, A,
 * IA, JA. NZ denotes the number of nonzero entries in matrix M. The array A
 * then is of length NZ and holds all nonzero entries of M. The array IA stores
 * at IA(i) the position of the first element of row i in the sparse array A.
 * The length of row i is determined by IA(i+1) - IA(i). Therefore IA needs to
 * be of length N + 1. In array JA, the column index of the element A(j) is
 * stored. JA is of length NZ.
 */
#ifndef _SPARSE_MATRIX_H
#define _SPARSE_MATRIX_H

#include <valarray>
#include <map>
#include <cstdio>

#include "libvpsc/assertions.h"

namespace cola {
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
        COLA_ASSERT(i<n);
        COLA_ASSERT(j<n);
        ConstIt v=lookup.find(std::make_pair(i,j));
        if(v!=lookup.end()) {
            return v->second;
        }
        return 0;
    }
    size_t nonZeroCount() const {
        return lookup.size();
    }
    void resize(unsigned n) {
        this->n = n;
    }
    void clear() {
        lookup.clear();
    }
};
/*
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
            : n(m.n), NZ((unsigned)m.nonZeroCount()), sparseMap(m), 
              A(std::valarray<double>(NZ)), IA(std::valarray<unsigned>(n+1)), JA(std::valarray<unsigned>(NZ)) {
        unsigned cnt=0;
        int lastrow=-1;
        for(SparseMap::ConstIt i=m.lookup.begin(); i!=m.lookup.end(); i++) {
            SparseMap::SparseIndex p = i->first;
            COLA_ASSERT(p.first<n);
            COLA_ASSERT(p.second<n);
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
    void rightMultiply(std::valarray<double> const & v, std::valarray<double> & r) const {
        COLA_ASSERT(v.size()>=n);
        COLA_ASSERT(r.size()>=n);
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
    std::valarray<double> A;
    std::valarray<unsigned> IA, JA;
};
} //namespace cola
#endif /* _SPARSE_MATRIX_H */
