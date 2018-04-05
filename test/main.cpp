#include "test/role/rolemgr.h"
#include "test/role/role.h"

#include "gtest/gtest.h"

#include "dmlua/dmlua.h"

TEST( luatest, luatest )
{
    CDMLuaEngine oDMLuaEngine;

    if ( !oDMLuaEngine.ReloadScript() )
    {
        return;
    }

    oDMLuaEngine.DoString(
        "function addtest()\n"
        "   local a = 100000000\n"
        "   local b = 100000000\n"
        "   local c = a * b\n"
        "   print(c)\n"
        "end\n" );
    {
        for ( int i = 0; i < 1; ++i )
        {
            int r = oDMLuaEngine.Call( "addtest" );
        }
    }

    if ( !oDMLuaEngine.ReloadScript() )
    {
        return;
    }

    oDMLuaEngine.DoString(
        "function addex(first, second, res)\n"
        "   res.value = first * second\n"
        "end\n" );
    {
        LResultINT64 resAdd( -1 );

        for ( int i = 0; i < 1; ++i )
        {
            int r = oDMLuaEngine.Call( "addex", 100000000LL, 100000000LL, &resAdd );

            if ( r >= 0 )
            {
                std::cout << resAdd.value << std::endl;
            }
        }
    }
    {
        oDMLuaEngine.DoString(
            "function script.task.taskinfo(task)\n"
            "   task.nTaskID = 1002\n"
            "   task.nTaskState = 2\n"
            "   task.nTaskCondition = 2\n"
            "end\n" );
        STaskInfo sInfo;
        int r = oDMLuaEngine.Call( "script.task.taskinfo", &sInfo );

        if ( r >= 0 )
        {
            std::cout << sInfo.nTaskID << std::endl;
        }
    }
    {
        oDMLuaEngine.DoString(
            "function script.task.taskinfo(task)\n"
            "   task.nTaskID = 1003\n"
            "   task.nTaskState = 1\n"
            "   task.nTaskCondition = 0\n"
            "end\n" );
        STaskInfo sInfo;
        int r = oDMLuaEngine.Call( "script.task.taskinfo", &sInfo );

        if ( r >= 0 )
        {
            std::cout << sInfo.nTaskID << std::endl;
        }
    }
    CRole* poRole = CRoleMgr::Instance()->CreateRole();
    poRole->SetName( "andy" );
    poRole->SetHp( 9999 );
    poRole->SetMp( 9999 );
    unsigned int dwTaskID = 100;
    LResultINT oResult( -1 );
    oDMLuaEngine.Call( "script.task.task.AcceptTask", poRole, dwTaskID, &oResult );
    oDMLuaEngine.Call( "script.task.task.FinishTask", poRole, dwTaskID );
    std::vector<std::string> vecData;
    vecData.push_back( "hello" );
    oDMLuaEngine.Call( "script.common.test.main", &vecData );
    oDMLuaEngine.Call( "script.config.loadcsv.main" );
    CRoleMgr::Instance()->ReleaseRole( poRole );
}

TEST( luaperformancetest, luaperformancetest )
{
    CDMLuaEngine oEZLuaEngine;
    oEZLuaEngine.DoString(
        "function performancetest()\n"
        "end\n" );

    for ( int i = 0; i < 1000000; ++i )
    {
        oEZLuaEngine.Call( "performancetest" );
    }
}