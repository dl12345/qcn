/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2014 dl12345@xda-developers forum

*/

#ifdef DEBUG
#define BOOST_SPIRIT_DEBUG
#endif

#include <string>
#include "qcn.hpp"

namespace qcn
{   
    diff_type const Compare(
        Qcn const& lhs, 
        Qcn const& rhs, 
        Qcn::cmp const cmp,        
        bool const recurse
    )
    {
        // option ::present will perform an inner join of non-matching items
        // option ::missing will perform an outer join of missing items
        // option ::both will perform an outer join of all non-matching items

        diff_type d;
        for (auto l = lhs.begin(); l != lhs.end(); ++l)
        {   
            auto r = rhs.Find(l->code);
            if (r != rhs.end())
            {            
                if (cmp != Qcn::cmp::missing && *l != *r) // inner join
                {
                    pair_type p(*l, *r);
                    d.push_back(p);
                }
            }
            else if (cmp != Qcn::cmp::present) // left outer join missing items
            {
                Qcn::item empty;
                pair_type p(*l, empty);
                d.push_back(p);
            }
        }
        if (cmp != Qcn::cmp::present && recurse) // right outer join missing items
        {
            auto rhs_only = Compare(rhs, lhs, Qcn::cmp::missing, false);
            for (auto s = rhs_only.begin(); s != rhs_only.end(); ++s)
            {
                // swap the order so that we have rhs and lhs correctly ordered

                Qcn::item empty;
                pair_type p(empty, s->first);
                d.push_back(p);
            }
        }
        return d;
    }

}




