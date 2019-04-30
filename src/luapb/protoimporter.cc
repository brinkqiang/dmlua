
#include "protoimporter.h"

#include <stdio.h>
#include "dmutil.h"

class ProtobufLibrary
{
public:
    ProtobufLibrary()
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
    };
    virtual ~ProtobufLibrary()
    {
        // fixed memory leak
        google::protobuf::ShutdownProtobufLibrary();
    }
};

static ProtobufLibrary _protobuf_library;

ProtoImporterImpl::ProtoImporterImpl(MyMultiFileErrorCollector& oErrorCollector, google::protobuf::compiler::DiskSourceTree& oSourceTree):
		m_oImporter(&oSourceTree, &oErrorCollector)
{

}

ProtoImporterImpl::~ProtoImporterImpl()
{
}

bool ProtoImporterImpl::Import(const std::string& strFileName)
{
	const  google::protobuf::FileDescriptor* filedescriptor = m_oImporter.Import(strFileName);
	if (!filedescriptor)
	{
		fprintf(stderr, "import (%s) file descriptor error\n", strFileName.c_str());
		return false;
	}
	return true;
}

google::protobuf::Message* ProtoImporterImpl::CreateMessage(const std::string& strTypeName)
{
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor = m_oImporter.pool()->FindMessageTypeByName(strTypeName);
	if (descriptor)
	{
		const google::protobuf::Message* prototype = m_oFactory.GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}

ProtoImporter::ProtoImporter()
{
    std::string strRoot = DMGetRootPath();
    std::string strProtoPath = strRoot + PATH_DELIMITER_STR + "proto";
    std::string strProtoPath2 = strRoot + PATH_DELIMITER_STR + ".." + PATH_DELIMITER_STR + "proto";

    m_oSourceTree.MapPath("", strRoot);
    m_oSourceTree.MapPath("", strProtoPath);
    m_oSourceTree.MapPath("", strProtoPath2);

    printf("[ProtoImporter] protopath:%s\n", strRoot.c_str());
    printf("[ProtoImporter] protopath:%s\n", strProtoPath.c_str());
    printf("[ProtoImporter] protopath:%s\n", strProtoPath2.c_str());
 
    SetImporter(new ProtoImporterImpl(m_oErrorCollector, m_oSourceTree));
}

ProtoImporter::~ProtoImporter()
{

}

bool ProtoImporter::Import(const std::string& strFileName)
{
    ProtoImporterImpl* poProtoImporter = GetImporter();
    if (NULL != poProtoImporter)
    {
        delete poProtoImporter;
        SetImporter(NULL);
    }

    SetImporter(new ProtoImporterImpl(m_oErrorCollector, m_oSourceTree));
    poProtoImporter = GetImporter();
 
    if (NULL == poProtoImporter)
    {
        return false;
    }

    return poProtoImporter->Import(strFileName);
}

google::protobuf::Message* ProtoImporter::CreateMessage(const std::string& strTypeName)
{
    return m_poProtoImporter->CreateMessage(strTypeName);
}
