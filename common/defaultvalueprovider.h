#ifndef __DEFAULTVALUEPROVIDER__
#define __DEFAULTVALUEPROVIDER__
#include <map>
#include <string>

#define DEFAULT_YANG_MODULE_PATH "/usr/local/yang-models"
#define EMPTY_STR ""

struct ly_ctx;

// Key information
typedef std::pair<std::string, int> KeyInfo;

// Field name to default value mapping
typedef std::map<std::string, std::string> FieldDefaultValueMapping;
typedef std::map<std::string, std::string> FieldValueMapping;
typedef std::shared_ptr<FieldDefaultValueMapping> FieldDefaultValueMappingPtr;

// Key info to default value info mapping
typedef std::map<KeyInfo, FieldDefaultValueMappingPtr> KeyInfoToDefaultValueInfoMapping;

namespace swss {

class TableInfoBase
{
public:
    TableInfoBase();

    void AppendDefaultValues(std::string row, FieldValueMapping& sourceValues, FieldValueMapping& targetValues);

    std::shared_ptr<std::string> GetDefaultValue(std::string row, std::string field);

protected:
    virtual bool FindFieldMappingByKey(std::string row, FieldDefaultValueMapping ** foundedMappingPtr) = 0;
};

class TableInfoDict : public TableInfoBase
{
public:
    TableInfoDict(KeyInfoToDefaultValueInfoMapping &fieldInfoMapping);

private:
    // Mapping: key value -> field -> default 
    std::map<std::string, FieldDefaultValueMappingPtr> m_defaultValueMapping;

    bool FindFieldMappingByKey(std::string row, FieldDefaultValueMapping ** foundedMappingPtr);
};

class TableInfoSingleList : public TableInfoBase
{
public:
    TableInfoSingleList(KeyInfoToDefaultValueInfoMapping &fieldInfoMapping);

private:
    // Mapping: field -> default 
    FieldDefaultValueMappingPtr m_defaultValueMapping;

    bool FindFieldMappingByKey(std::string row, FieldDefaultValueMapping ** foundedMappingPtr);
};

struct TableInfoMultipleList : public TableInfoBase
{
public:
    TableInfoMultipleList(KeyInfoToDefaultValueInfoMapping &fieldInfoMapping);

private:
    // Mapping: key field count -> field -> default 
    std::map<int, FieldDefaultValueMappingPtr> m_defaultValueMapping;

    bool FindFieldMappingByKey(std::string row, FieldDefaultValueMapping ** foundedMappingPtr);
};

class DefaultValueProvider
{
public:
    static DefaultValueProvider& Instance();

    void AppendDefaultValues(std::string table, std::string row, FieldValueMapping& values);

    void AppendDefaultValues(std::string table, std::string row, std::vector<std::pair<std::string, std::string> > &values);

    std::shared_ptr<std::string> GetDefaultValue(std::string table, std::string row, std::string field);
    
#ifdef DEBUG
    bool FeatureEnabledByEnvironmentVariable();
#endif

private:
    DefaultValueProvider();
    ~DefaultValueProvider();

    //  libyang context
    struct ly_ctx *m_context = nullptr;

    // The table name to table default value info mapping
    std::map<std::string, std::shared_ptr<TableInfoBase> > m_defaultValueMapping;

    void Initialize(char* modulePath = DEFAULT_YANG_MODULE_PATH);

    // Load default value info from yang model and append to default value mapping
    void AppendTableInfoToMapping(struct lys_node* table);

    std::shared_ptr<TableInfoBase> FindDefaultValueInfo(std::string table);

    int BuildFieldMappingList(struct lys_node* table, KeyInfoToDefaultValueInfoMapping& fieldMappingList);
    
    std::shared_ptr<KeyInfo> GetKeyInfo(struct lys_node* table_child_node);
    FieldDefaultValueMappingPtr GetDefaultValueInfo(struct lys_node* tableChildNode);
};

}
#endif