
#ifndef PROTOIMPORTER_H_
#define PROTOIMPORTER_H_

#include "dmsingleton.h"

#include <string>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

class MyMultiFileErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    virtual void AddError(const std::string& filename, int line, int column,
        const std::string& message)
    {
        fprintf(stderr, "%s:%d:%d:%s\n", filename.c_str(), line, column, message.c_str());
    }

    virtual void AddWarning(const std::string& filename, int line, int column,
        const std::string& message)
    {
        fprintf(stdout, "%s:%d:%d:%s\n", filename.c_str(), line, column, message.c_str());
    }
};

class ProtoImporterImpl
{
public:
	ProtoImporterImpl(MyMultiFileErrorCollector& oErrorCollector, google::protobuf::compiler::DiskSourceTree& oSourceTree);
    virtual ~ProtoImporterImpl();
public:
	bool Import(const std::string& strFileName);
	google::protobuf::Message* CreateMessage(const std::string& strTypeName);
public:
	google::protobuf::compiler::Importer m_oImporter;
	google::protobuf::DynamicMessageFactory m_oFactory;


};

class ProtoImporter : public TSingleton<ProtoImporter>
{
    friend class TSingleton<ProtoImporter>;
public:
    ProtoImporter();
    virtual ~ProtoImporter();

    bool Import(const std::string& strFileName);
    google::protobuf::Message* CreateMessage(const std::string& strTypeName);

    ProtoImporterImpl* GetImporter() {
        return m_poProtoImporter;
    }

    void SetImporter(ProtoImporterImpl* poProtoImporter) {
        m_poProtoImporter = poProtoImporter;
    }

    MyMultiFileErrorCollector& GetErrorCollector() { return m_oErrorCollector; }
    google::protobuf::compiler::DiskSourceTree& GetSourceTree() { return m_oSourceTree; }
private:
    ProtoImporterImpl* m_poProtoImporter;

    MyMultiFileErrorCollector m_oErrorCollector;
    google::protobuf::compiler::DiskSourceTree m_oSourceTree;
};


#endif
