
#ifndef PROTOIMPORTER_H_
#define PROTOIMPORTER_H_

#include "gsingleton.h"

#include <string>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

class ProtoImporter : public TSingleton<ProtoImporter>
{
    friend class TSingleton<ProtoImporter>;
public:
	ProtoImporter();
    ~ProtoImporter();
public:
	bool Import(const std::string& filename);
	google::protobuf::Message* CreateMessage(const std::string& typeName);
public:
	google::protobuf::compiler::Importer importer;
	google::protobuf::DynamicMessageFactory factory;
};

#endif
