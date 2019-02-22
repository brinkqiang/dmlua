
#include "protoimporter.h"

#include <stdio.h>
#include "dmutil.h"

class MyMultiFileErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{	
	virtual void AddError(
			const std::string & filename,
			int line,
			int column,
			const std::string & message)
	{
		fprintf(stderr, "%s:%d:%d:%s\n", filename.c_str(), line, column, message.c_str());
	}
};

static MyMultiFileErrorCollector errorCollector;
static google::protobuf::compiler::DiskSourceTree sourceTree;

ProtoImporter::ProtoImporter():
		importer(&sourceTree, &errorCollector)
{
    std::string strRoot = DMGetRootPath();
    std::string strPath = strRoot + PATH_DELIMITER_STR + "proto";
    std::string strPath2 = strRoot + PATH_DELIMITER_STR + ".." + PATH_DELIMITER_STR + "proto";

    sourceTree.MapPath("", strRoot);
    sourceTree.MapPath("", strPath);
    sourceTree.MapPath("", strPath2);

    printf("[ProtoImporter] protopath:%s\n", strRoot.c_str());
	printf("[ProtoImporter] protopath:%s\n", strPath.c_str());
    printf("[ProtoImporter] protopath:%s\n", strPath2.c_str());
}

ProtoImporter::~ProtoImporter()
{
}

bool ProtoImporter::Import(const std::string& filename)
{
	const  google::protobuf::FileDescriptor* filedescriptor = importer.Import(filename);
	if (!filedescriptor)
	{
		fprintf(stderr, "import (%s) file descriptor error\n", filename.c_str());
		return false;
	}
	return true;
}

google::protobuf::Message* ProtoImporter::CreateMessage(const std::string& typeName)
{
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor = importer.pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const google::protobuf::Message* prototype = factory.GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}
