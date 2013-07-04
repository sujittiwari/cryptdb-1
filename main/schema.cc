#include "schema.hh"

using namespace std;


ostream&
operator<<(ostream &out, const OnionLevelFieldPair &p)
{
    out << "(onion " << p.first
        << ", level " << levelnames[(int)p.second.first]
        << ", field `" << (p.second.second == NULL ? "*" : p.second.second->fname) << "`"
        << ")";
    return out;
}


std::ostream&
operator<<(std::ostream &out, const OLK &olk)
{
    out << "( onion " << olk.o << " level " << levelnames[(uint)olk.l] << " fieldmeta ";
    if (olk.key == NULL) {
	out << " NULL ";
    } else {
	out << olk.key->fname;
    }
    out << ")";
    return out;
}


string FieldMeta::stringify() {
    string res = " [FieldMeta " + fname + "]";
    return res;
}

FieldMeta::FieldMeta()
{
    fname = "";
    sql_field = NULL;
    salt_name = "";
    has_salt = true;

}

FieldMeta::~FieldMeta()
{
    for (auto onion_it : onions) {
        delete onion_it.second;
    }
}

bool TableMeta::destroyFieldMeta(std::string field)
{
    fieldNames.remove(field);
    return 1 == fieldMetaMap.erase(field);
}

TableMeta::TableMeta() {
    anonTableName = "";
    tableNo = 0;
}

TableMeta::~TableMeta()
{
    for (auto i = fieldMetaMap.begin(); i != fieldMetaMap.end(); i++)
        delete i->second;

}

// table_no: defaults to NULL indicating we are to generate it ourselves.
TableMeta *
SchemaInfo::createTableMeta(std::string table_name,
                            std::string anon_table_name, bool has_sensitive,
                            bool has_salt, std::string salt_name,
                            const unsigned int *table_no)
{
    // Make sure a table with this name does not already exist.
    std::map<std::string, TableMeta *>::iterator it =
        tableMetaMap.find(table_name);
    if (tableMetaMap.end() != it) {
        return NULL;
    }

    unsigned int table_number;
    if (NULL == table_no) {
        table_number = ++totalTables;
    } else {
        // TODO: Make sure no other tables with this number exist.
        ++totalTables;
        table_number = *table_no;
    }
    TableMeta *tm = new TableMeta(table_number, anon_table_name,
                                  has_sensitive, has_salt, salt_name);
    tableMetaMap[table_name] = tm;
    return tm;
}

TableMeta *
SchemaInfo::getTableMeta(const string & table) {
    auto it = tableMetaMap.find(table);
    assert_s(it != tableMetaMap.end(), "could not find table " + table);
    if (tableMetaMap.end() == it) {
        return NULL;
    } else {
        return it->second;
    }
}

FieldMeta *
SchemaInfo::getFieldMeta(const string & table, const string & field) {
    TableMeta * tm = getTableMeta(table);
    auto it = tm->fieldMetaMap.find(field);
    if (tm->fieldMetaMap.end() == it) {
        return NULL;
    } else {
        return it->second;
    }
}

bool
SchemaInfo::destroyTableMeta(std::string table)
{
    if (totalTables <= 0) {
        throw CryptDBError("SchemaInfo::totalTables can't be less than zero");
    }

    --totalTables;
    return 1 == tableMetaMap.erase(table);
}
