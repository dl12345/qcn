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

#include <iostream>
#include <fstream>
#include <string>
#include "qcn.hpp"

namespace qcn
{
    Qcn::Qcn(std::string const& filename)
        :   filename_(filename), 
            items_(0), 
            size_(0), 
            err_(""),
            success_(false)
    {
    }

    Qcn::Qcn(Qcn const& rhs)
        :   filename_(rhs.filename_), 
            items_(rhs.items_), 
            size_(rhs.size_), 
            err_(rhs.err_),
            success_(rhs.success_),
            map_(rhs.map_)
    {
    }

    bool const Qcn::Open()
    {
        using boost::spirit::ascii::space;    
        using boost::spirit::qi::eoi;
        using boost::spirit::qi::blank;

        typedef boost::spirit::istream_iterator iterator_type;    
        typedef qcnparser< iterator_type > parser;
   
        std::ifstream in(filename_);  
        if (!in.is_open()) 
        {
            err_ = "Could not open input file";
            success_ = false;
            return success_;
        }
        in.unsetf(std::ios::skipws);

        iterator_type begin(in);
        iterator_type end;

        if (begin == end)
        {
            err_ = "Empty input file";
            success_ = false;
            return success_;
        }

        unsigned n = 0, s = 0;
        parser qcn(n, s); 
    
        bool r = phrase_parse(begin, end, qcn >> eoi, space, data_);   

        if (r && begin == end)
        {
            MakeHashTable();
            success_ = true;            
            return success_;
        }
        else
        {
            err_ = "Invalid format input file";
            success_ = false;
            return success_;
        } 
    }

    void Qcn::MakeHashTable()
    {
        for (auto i = data_.begin(); i != data_.end(); ++i)
        {
            map_[(*i).code] = *i;
        }
        data_.clear();
    }

    std::ostream& operator<<(std::ostream& o, Qcn const& q)
    {
        for (auto i = q.map_.begin(); i != q.map_.end(); ++i)
        {
            o << i->second << " \n";
        }
        return o;
    }

    uint const Qcn::Size() const
    {
        return map_.size();
    }

    Qcn::iterator Qcn::Find(uint const& key)
    {
        auto i = map_.find(key);
        return i;
    }

    Qcn::const_iterator Qcn::Find(uint const& key) const
    {
        auto i = map_.find(key);
        return i;
    }

    item_type& Qcn::operator[](uint const& key)
    {
        return map_[key];
    }

    Qcn::~Qcn()
    {
    }

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




