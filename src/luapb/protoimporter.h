
#ifndef PROTOIMPORTER_H_
#define PROTOIMPORTER_H_

#include "dmsingleton.h"

#include <string>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

class ProtoImporter : public TSingleton<ProtoImporter>
{
    friend class TSingleton<ProtoImporter>;
public:
	ProtoImporter();
    virtual ~ProtoImporter();
public:
	bool Import(const std::string& strFileName);
	google::protobuf::Message* CreateMessage(const std::string& strTypeName);
public:
	google::protobuf::compiler::Importer m_oImporter;
	google::protobuf::DynamicMessageFactory m_oFactory;
};

#endif
