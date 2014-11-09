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

#ifndef QCN_H
#define QCN_H

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/classic/iterator/file_iterator.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <iomanip>


namespace qcn
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace fusion = boost::fusion;
    namespace range = boost::range_detail;
    namespace phx = boost::phoenix;

    typedef unsigned int uint;

    // Skipper type for parser
    typedef ascii::space_type space_type;

    // Synthesized attribute types

    // Vector for the parsed NV Item - itemdata attribute
    typedef std::vector< uint > data_type;

    // attribute of item, itempresent, itemnotpresent: code, status, item data
    typedef struct qitem
    {
        uint code;
        std::string status;
        data_type data;

        qitem(): code(0), status("") {}

        qitem(struct qitem const& c)
            : code(c.code), status(c.status), data(c.data) {}

        inline bool operator==(struct qitem const& rhs) const
        {
            if (code == 0 && status == "") // the empty object doesn't match
            {
                return false;
            }
            if (code == rhs.code && status == rhs.status && data == rhs.data)
            {
                return true;
            }
            return false;
        }
        inline bool operator!=(struct qitem const& rhs) const
        {
            if (code == 0 && status == "")
            {
                return true;
            }
            if (code != rhs.code || status != rhs.status || data != rhs.data)
            {
                return true;
            }
            return false;
        }
    

        friend std::ostream& operator<<(std::ostream& o, struct qitem const& q)
        {
            o << std::setw(4) << std::setfill('0');
            o << std::dec << q.code << " (0x";
            o << std::setw(4) << std::setfill('0');
            o << std::uppercase << std::hex << q.code;
            o << ") - " << q.status << std::endl;

            for (auto i = 0; i < q.data.size(); i++)
            {
                if (i && i % 16 == 0) 
                {
                    std::cout << std::endl;
                }
                else if (i) o << " ";

                o << std::setw(2) << std::setfill('0');
                o << std::uppercase << std::hex << q.data[i];
            }
            if(q.data.size() > 0) o << std::endl;
            return o;
        }

    } item_type;

    // Parser output - a vector of items: code, status, item data
    typedef std::vector<item_type> qcndata_type;    

    template < typename Iterator, typename Skipper = space_type >
    class qcnparser : public qi::grammar< Iterator, qcndata_type(), Skipper > 
    {        
    public:

        // itemcode attribute
        typedef uint code_type;

        // Raw vector of byte values for the NV Item - itemleaf attribute
        typedef std::vector< uint > leaf_type;

        // itemnumber and itemsize attributes
        typedef fusion::vector2<std::string, uint> size_type;

        // iteminfo attribute. Composite of itemnumber and itemsize
        typedef fusion::vector2< size_type, size_type > info_type;
        
        qcnparser(uint& n, uint& s) : qcnparser::base_type(qcndata)
        {

            using qi::uint_;
            using qi::ulong_;
            using qi::hex;
            using qi::char_;
            using qi::string;
            using qi::lit;
            using qi::eol;
            using qi::uint_parser;
            using qi::repeat;
            using qi::_val;
            using qi::omit;
            using qi::_1;
            using qi::attr;
            using qi::eps;
            using phx::ref;
           
            qcndata = omit[+(header)] >> *(item);

            header = '[' >> description >> ']';

            description = nvitemslabel | iteminfo; 

            nvitemslabel = lit("NV items") ;

            iteminfo = itemnumber >> ',' >> itemsize;

            // In the presence of a semantic action the % operator is 
            // necessary to ensure the propagation of the attribute if
            // _val is not assigned explicitly

            itemnumber %= (string("Complete items") >> '-' >> uint_[ref(n)=_1]);

            itemsize %= (string("Items size") >> '-' >> uint_[ref(s)=_1]);        

            item = itempresent | itemnotpresent;

            itempresent = itemcode >> statusok >> itemdata;

            itemnotpresent = itemcode >> statusother >> attr(data_type());

            itemcode = ulong_ >> omit [ itemdiscard ];

            itemdiscard = '(' >> lit("0x") >> hex >> ')' >> "-";

            statusok = string("OK");

            statusother = (string("Inactive item")
                        | string("Parameter bad")
                        | string("Access denied"));

            // Rules separated to allow for semantic action on data if desired

            // For example
            // itemdata = omit[eps] >> itemleaf[_val=convert(_1, data_type())];

            itemdata = omit[eps] >> itemleaf;
            itemleaf = repeat(ref(s))[hex];
     
#ifdef BOOST_SPIRIT_DEBUG

            BOOST_SPIRIT_DEBUG_NODE(qcndata);
            BOOST_SPIRIT_DEBUG_NODE(header);
            BOOST_SPIRIT_DEBUG_NODE(item);

#endif

        }
    private:
        qi::rule<Iterator, qcndata_type(), Skipper> qcndata; 

        qi::rule<Iterator, info_type(), Skipper>  header; 
        qi::rule<Iterator, info_type(), Skipper> description;

        qi::rule<Iterator, Skipper> nvitemslabel;
        qi::rule<Iterator, info_type(), Skipper> iteminfo;

        qi::rule<Iterator, size_type(), Skipper> itemnumber;
        qi::rule<Iterator, size_type(), Skipper> itemsize; 

        qi::rule<Iterator, item_type(), Skipper> item;
        qi::rule<Iterator, item_type(), Skipper> itempresent;
        qi::rule<Iterator, item_type(), Skipper> itemnotpresent; 

        qi::rule<Iterator, std::string(), Skipper> statusok;
        qi::rule<Iterator, std::string(), Skipper> statusother;

        qi::rule<Iterator, code_type(), Skipper> itemcode;
        qi::rule<Iterator, Skipper> itemdiscard;
        qi::rule<Iterator, data_type(), Skipper> itemdata;
        qi::rule<Iterator, leaf_type(), Skipper> itemleaf;
    
    };

    class Qcn
    {
    public:

        // hash table of items
        typedef boost::unordered_map<uint, item_type> map_type;
        typedef item_type item;        

        typedef map_type::iterator map_iterator; 
        typedef map_type::const_iterator map_const_iterator;

        typedef enum {present, missing, both} cmp;

        Qcn(std::string const& filename);
        Qcn(Qcn const& rhs);
        ~Qcn();

        bool const Open();
        bool const IsOpen() const { return success_; }
        uint const Size() const;
        std::string const& ErrorMessage() { return err_; }

        item_type& operator[](uint const& key); 
        friend std::ostream& operator<<(std::ostream& o, Qcn const& q);

    public:

        class iterator 
            : public boost::iterator_adaptor<
                iterator, 
                map_iterator,
                item_type&,
                boost::forward_traversal_tag
              >
        {
        public:
            iterator(): iterator::iterator_adaptor_() {}

            iterator(const map_iterator& i) : iterator::iterator_adaptor_(i){}

            item_type& dereference() const
            {
                return base()->second;
            }
            item_type* reference() const
            {
                return &(base()->second);
            }
       
        private:
            friend class boost::iterator_core_access;
        };

        class const_iterator 
            : public boost::iterator_adaptor<
                const_iterator,
                map_const_iterator,
                item_type const&,
                boost::forward_traversal_tag
            >
        {
        public:

            const_iterator(): const_iterator::iterator_adaptor_() {}

            const_iterator(const map_const_iterator& i)
                : const_iterator::iterator_adaptor_(i) {}

            const_iterator(const iterator& i)
                : const_iterator::iterator_adaptor_(i.base()) {}

            item_type const& dereference() const
            {
                return (item_type const&) base()->second;
            }

            item_type const* reference() const
            {
                return (item_type const *) &(base()->second);
            }

        private:
            friend class boost::iterator_core_access;
        };


        iterator begin()
        {
            return iterator(map_.begin());
        }

        iterator end()
        {
            return iterator(map_.end());
        }

        const_iterator begin() const
        {
            return const_iterator(map_.begin());
        }

        const_iterator end() const
        {
            return const_iterator(map_.end());
        }

        iterator Find(uint const& key);        
        const_iterator Find(uint const& key) const;
     
    private:
        void MakeHashTable();

        std::string filename_;
        std::string err_;
        qcndata_type data_;
        map_type map_;
        uint items_, size_;
        bool success_;
    };

    typedef std::pair<Qcn::item, Qcn::item> pair_type;
    typedef std::vector<pair_type> diff_type;

    diff_type const Compare(
        Qcn const& lhs, 
        Qcn const& rhs, 
        Qcn::cmp const cmp = Qcn::cmp::both,
        bool const recurse = true
    );
}

BOOST_FUSION_ADAPT_STRUCT(
    qcn::qitem,
    (qcn::uint, code)
    (std::string, status)
    (std::vector<qcn::uint>, data)
)

#endif
