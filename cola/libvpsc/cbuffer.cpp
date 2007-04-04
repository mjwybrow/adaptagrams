/**
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * \brief 
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#include "cbuffer.h"
#include "constraint.h"
#include <cassert>

namespace vpsc {
    static const double ZERO_UPPERBOUND=-0.0000001;
	void CBuffer::load() {
        size=0; 
        double buffMaxSlack=-DBL_MAX;
        unsigned buffMaxSlackPos=0;
        for(Constraints::iterator i=master_list.begin();
                i!=master_list.end();++i) {
            Constraint *c=*i;
            double slack = c->slack();
            if(c->equality||slack<ZERO_UPPERBOUND) {
                if(size<maxsize) {
                    // make sure buffer is full
                    buffer[size]=c;
                    if(slack>buffMaxSlack) {
                        buffMaxSlack=slack;
                        buffMaxSlackPos=size;
                    }
                    size++;
                } else {
                    // if c is more violated than the least violated
                    // constraint in the buffer then replace it
                    buffer[buffMaxSlackPos]=c;
                    // need to search the buffer for the new least
                    // violated constraint
                    buffMaxSlack=-DBL_MAX;
                    for(unsigned i=0;i<size;i++) {
                        c=buffer[i];
                        if(!c->equality&&buffMaxSlack < c->slack()) {
                            buffMaxSlack = slack;
                            buffMaxSlackPos = i;
                        }
                    }
                }
            }
        }
    }
    Constraint* CBuffer::mostViolated() {
        Constraint* v=NULL;
        while(true) {
            if(size==0) {
                load();
                if(size==0) break;
            }
            double minSlack=DBL_MAX;
            int i,deletePos=-1;
            for(i=0;i<(int)size;i++) {
                Constraint *c=buffer[i];
                double slack = c->slack();
                if(!(c->equality||slack < ZERO_UPPERBOUND)) {
                    assert(size>0);
                    buffer[i--]=buffer[--size];
                } else if(c->equality||slack < minSlack) {
                    v=c;
                    deletePos=i;	
                    minSlack=slack;
                }
            }
            if(deletePos>=0) {
                assert(size>0);
                buffer[deletePos]=buffer[--size];
                break;
            }
        }
        return v;
    }
}
