// -*- C++ -*-
#ifndef _ParserUtil_h_
#define _ParserUtil_h_

#include "Parser.h"

struct ParamLabel : public boost::spirit::grammar<ParamLabel>
{
    ParamLabel(const std::string& param_name) : m_param_name(param_name) {}
    template <typename ScannerT>
    struct definition
    {
        definition(ParamLabel const& self) {
            r = !(boost::spirit::str_p(self.m_param_name.c_str()) >> '=');
        }
        boost::spirit::rule<ScannerT> r;
        boost::spirit::rule<ScannerT> const& start() const {return r;}
    };
    const std::string m_param_name;
};

struct push_back_impl
{
    template <class Container, class Item>
    struct result {typedef void type;};
    template <class Container, class Item>
    void operator()(Container& c, const Item& item) const {c.push_back(item);}
};
extern const phoenix::function<push_back_impl> push_back_;

struct insert_impl
{
    template <class Container, class Item>
    struct result {typedef std::pair<typename Container::iterator, bool> type;};
    template <class Container, class Item>
    std::pair<typename Container::iterator, bool>
    operator()(Container& c, const Item& item) const {return c.insert(item);}
};
extern const phoenix::function<insert_impl> insert_;

extern boost::spirit::rule<Scanner, NameClosure::context_t> name_p;
extern boost::spirit::rule<Scanner, NameClosure::context_t> file_name_p;

extern boost::spirit::symbols<PlanetSize> planet_size_p;
extern boost::spirit::symbols<PlanetType> planet_type_p;
extern boost::spirit::symbols<PlanetEnvironment> planet_environment_type_p;
extern boost::spirit::symbols<UniverseObjectType> universe_object_type_p;
extern boost::spirit::symbols<StarType> star_type_p;
extern boost::spirit::symbols<FocusType> focus_type_p;
extern boost::spirit::symbols<EmpireAffiliationType> affiliation_type_p;
extern boost::spirit::symbols<UnlockableItemType> unlockable_item_type_p;
extern boost::spirit::symbols<TechType> tech_type_p;

void ReportError(std::ostream& os, const char* input, const boost::spirit::parse_info<const char*>& result);

#endif // _ParserUtil_h_