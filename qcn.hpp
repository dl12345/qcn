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

#include <iomanip>
#include <fstream>
#include <iterator>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/classic/iterator/file_iterator.hpp>
#include <boost/unordered_map.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/iterator/iterator_adaptor.hpp>


namespace qcn
{
    namespace spirit = boost::spirit;
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace fusion = boost::fusion;
    namespace range = boost::range_detail;
    namespace phx = boost::phoenix;

    class Dictionary;
    class Qcn;
    
    struct qitem;    
    struct dict_code;
    
    typedef unsigned int uint;
    
    // Skipper type for parser
    typedef ascii::space_type space_type;

    // item grammar types
    typedef struct qitem qcn_item_type;
    typedef std::vector<qcn_item_type> qcn_items_type;  
    typedef std::vector< uint > qcn_item_data_type;    
    typedef boost::unordered_map<uint, qcn_item_type> qcn_map_type;   
        
    // dictionary grammar types
    typedef struct dict_code dict_code_type;
    typedef std::vector<dict_code_type> dict_codes_type; 
    typedef boost::unordered_map<uint, dict_code_type> dict_map_type;
    
    // comparison type pairs
    typedef std::pair<qcn_item_type, qcn_item_type> pair_type;
    typedef std::vector<pair_type> diff_type;
    
    
    struct qitem
    {
        uint code;
        std::string description;
        std::string category;
        std::string status;
        qcn_item_data_type data;

        qitem(): code(0) {}

        qitem(struct qitem const& c)
            : code(c.code), status(c.status), data(c.data) {}

        bool operator==(struct qitem const& rhs) const
        {
            if (code == 0 && status.empty()) // the empty object doesn't match
            {
                return false;
            }
            if (code == rhs.code && status == rhs.status && data == rhs.data)
            {
                return true;
            }
            return false;
        }
        
        bool operator!=(struct qitem const& rhs) const
        {
            if (code == 0 && status.empty())
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
            o << std::dec << q.code;
            
            if (!q.description.empty())
            {
                o << " (" << q.description << ", " << q.category;
            }
            else
            {
                o << " (0x";
                o << std::setw(4) << std::setfill('0');
                o << std::uppercase << std::hex << q.code;
            }
            o << ") - " << (q.status == "" ? "Missing" : q.status) << std::endl;

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

    };  

    template < typename Iterator, typename Skipper = space_type >
    class qcnparser : public qi::grammar< Iterator, qcn_items_type(), Skipper > 
    {        
    public:

        // qcn grammar types
        typedef uint code_type;
        typedef std::vector< uint > leaf_type;
        typedef fusion::vector2< std::string, uint > size_type;
        typedef fusion::vector2< size_type, size_type > info_type;
        
        qcnparser() : qcnparser::base_type(qcndata), size_(0)
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
            
            itemnumber = string("Complete items") >> '-' >> uint_;

            // In the presence of a semantic action the % operator is 
            // necessary to ensure the propagation of the attribute if
            // _val is not assigned explicitly

            itemsize %= (string("Items size") >> '-' >> uint_[ref(size_)=_1]);        

            item = itempresent | itemnotpresent;

            itempresent = itemcode >> statusok >> itemdata;

            itemnotpresent = itemcode                                
                                >> statusother
                                >> attr(qcn_item_data_type());

            itemcode = ulong_ >> omit [ itemdiscard ];

            itemdiscard = '(' >> lit("0x") >> hex >> ')' >> "-";

            statusok = string("OK");

            statusother = (string("Inactive item")
                        | string("Parameter bad")
                        | string("Access denied"));

            // Rules separated to allow for semantic action on data if desired

            // For example
            // omit[eps] >> itemleaf[_val=convert(_1, qcn_item_data_type())];

            itemdata = omit[eps] >> itemleaf;
            itemleaf = repeat(ref(size_))[hex];
    
#ifdef BOOST_SPIRIT_DEBUG

            BOOST_SPIRIT_DEBUG_NODE(qcndata);
            BOOST_SPIRIT_DEBUG_NODE(header);
            BOOST_SPIRIT_DEBUG_NODE(item);

#endif

        }
    private:
        qi::rule<Iterator, qcn_items_type(), Skipper> qcndata; 

        qi::rule<Iterator, info_type(), Skipper>  header; 
        qi::rule<Iterator, info_type(), Skipper> description;

        qi::rule<Iterator, Skipper> nvitemslabel;
        qi::rule<Iterator, info_type(), Skipper> iteminfo;

        qi::rule<Iterator, size_type(), Skipper> itemnumber;
        qi::rule<Iterator, size_type(), Skipper> itemsize; 

        qi::rule<Iterator, qcn_item_type(), Skipper> item;
        qi::rule<Iterator, qcn_item_type(), Skipper> itempresent;
        qi::rule<Iterator, qcn_item_type(), Skipper> itemnotpresent; 

        qi::rule<Iterator, std::string(), Skipper> statusok;
        qi::rule<Iterator, std::string(), Skipper> statusother;

        qi::rule<Iterator, code_type(), Skipper> itemcode;
        qi::rule<Iterator, Skipper> itemdiscard;
        qi::rule<Iterator, qcn_item_data_type(), Skipper> itemdata;
        qi::rule<Iterator, leaf_type(), Skipper> itemleaf;
        
        unsigned int size_;
    
    }; 
    
    struct dict_code
    {

        uint code;
        std::string description;
        std::string category;
          
        friend std::ostream& operator<<(
                std::ostream& o, 
                struct dict_code const& c)
        {
            o << c.description << " - " << c.category;
            return o;
        }

    };

    template < typename Iterator, typename Skipper = space_type >
    class codeparser : public qi::grammar<Iterator, dict_codes_type(), Skipper>
    {        
    public:
                    
        codeparser() : codeparser::base_type(codes)
        {

            using qi::uint_;
            using qi::char_;
            using qi::string;
            using qi::lit;
            using qi::eol;
            using qi::omit;
            using qi::no_skip;
            using qi::_a;
           
            codes = *(uint_ >> sep >> description >> sep >> category);
                
            sep = omit[ char_("^") ];
                
            description =  
                   omit[char_("\"") ]             
                >> no_skip[+(char_ - char_("\""))]
                >> omit[char_("\"")]           
            ;

            category = 
                   omit[ char_("'\"") ]             
                >> no_skip[+(char_ - char_("*\""))]    
                >> no_skip[ omit[ *(char_ - eol) ] >> eol]
            ;
                
#ifdef BOOST_SPIRIT_DEBUG
                
            BOOST_SPIRIT_DEBUG_NODE(codes);
            BOOST_SPIRIT_DEBUG_NODE(description);
            BOOST_SPIRIT_DEBUG_NODE(category);
            BOOST_SPIRIT_DEBUG_NODE(sep);
                
#endif
                
        }
    private:
        qi::rule<Iterator, dict_codes_type(), Skipper> codes; 
        qi::rule<Iterator, Skipper> sep;
        qi::rule<Iterator, std::string(), Skipper> description;
        qi::rule<Iterator, std::string(), Skipper> category;
    };            
    
    template <
                typename T_data_type, 
                typename T_item_type,
                typename T_map_type, 
                typename T_parser_type
            >
    class DataFile
    {
    public:
    
        typedef typename T_data_type::iterator d_iterator;
        typedef typename T_map_type::iterator m_iterator;
        typedef typename T_map_type::const_iterator m_const_iterator;
        
        DataFile(std::string const& filename)
            :   filename_(filename), 
                err_(""),
                success_(false)
        {
        }

        DataFile(DataFile const& rhs)
            :   filename_(rhs.filename_), 
                err_(rhs.err_),
                success_(rhs.success_),
                map_(rhs.map_)  
        {
        }
 
        ~DataFile() {}
         
        bool const Open()
        {
            using spirit::ascii::space;    
            using qi::eoi;
            using qi::blank;
   
            std::ifstream in(filename_);  
            if (!in.is_open()) 
            {
                err_ = "Could not open input file";
                success_ = false;
                return success_;
            }
            in.unsetf(std::ios::skipws);

            typedef spirit::istream_iterator iterator_type;
            
            iterator_type begin(in);
            iterator_type end;

            if (begin == end)
            {
                err_ = "Empty input file";
                success_ = false;
                return success_;
            }

            T_parser_type p;   
            bool r = phrase_parse(begin, end, p >> eoi, space, data_);    

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
            return false;
        }
    
        bool const IsOpen() const { return success_; }
        uint const Size() const { return map_.size(); }
        std::string const& ErrorMessage() { return err_; }
        
    public:

        class iterator 
            : public boost::iterator_adaptor<
                                                iterator, 
                                                m_iterator,
                                                T_item_type&,
                                                boost::forward_traversal_tag
                                            >
        {
        public:
            iterator(): iterator::iterator_adaptor_() {}

            iterator(const m_iterator& i) : iterator::iterator_adaptor_(i){}

            T_item_type& dereference() const
            {
                return this->base()->second;
            }
            
            T_item_type* reference() const
            {
                return &(this->base()->second);
            }
       
        private:
            friend class boost::iterator_core_access;
        };    

        class const_iterator 
            : public boost::iterator_adaptor<
                                                const_iterator,
                                                m_const_iterator,
                                                T_item_type const&,
                                                boost::forward_traversal_tag
                                            >
        {
        public:

            const_iterator(): const_iterator::iterator_adaptor_() {}

            const_iterator(const m_const_iterator& i)
                : const_iterator::iterator_adaptor_(i) {}

            const_iterator(const iterator& i)
                : const_iterator::iterator_adaptor_(i.base()) {}

            T_item_type const& dereference() const
            {
                return (T_item_type const&) this->base()->second;
            }

            T_item_type const* reference() const
            {
                return (T_item_type const *) &(this->base()->second);
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

        iterator Find(uint const& key) { return map_.find(key); }
        const_iterator Find(uint const& key) const { return map_.find(key); }


    private:
    
        virtual typename T_map_type::key_type const Key(d_iterator const& i)=0;
          
        void MakeHashTable()
        {
            for (auto i = data_.begin(); i != data_.end(); ++i)
            {
                map_[Key(i)] = *i;
            }
            data_.clear();
        }
            
        std::string filename_;
        std::string err_;
        bool success_;
        T_data_type data_;
        T_map_type map_;
    };
                      
    class Dictionary: public DataFile   < 
                                            dict_codes_type, 
                                            dict_code_type,
                                            dict_map_type, 
                                            codeparser<spirit::istream_iterator> 
                                        > 
    {
    public:
    
        Dictionary(std::string const& filename) : DataFile(filename) {}
        Dictionary(Dictionary const& rhs) : DataFile(rhs) {}
          
          
    private:
    
        typename dict_map_type::key_type const Key(d_iterator const& i)
        {
            return i->code;
        }
        
    };
    
    class Qcn: public DataFile  < 
                                    qcn_items_type, 
                                    qcn_item_type,
                                    qcn_map_type, 
                                    qcnparser< spirit::istream_iterator > 
                                > 
    {
    public:
    
        typedef qcn_item_type item;     
        typedef enum {present, missing, both} cmp;
            
        Qcn(std::string const& filename) : DataFile(filename) {}
        Qcn(Qcn const& rhs) : DataFile(rhs) {}
                  
    private:     
     
        typename qcn_map_type::key_type const Key(d_iterator const& i)
        {
            return i->code;
        }

    };
    
    diff_type const Compare (
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

BOOST_FUSION_ADAPT_STRUCT(
    qcn::dict_code,
    (qcn::uint, code)
    (std::string, description)
    (std::string, category)
)

#endif
