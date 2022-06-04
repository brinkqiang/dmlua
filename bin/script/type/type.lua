
-- lua script
module (..., package.seeall)

function TypeTest(chData, wData, dwData, qwData, sData, nData, llData ,fData, dbData)
    print("==================================")
    print(type(chData), type(wData), type(dwData), type(qwData), type(sData), type(nData), type(llData), type(fData), type(dbData))
    print(chData .. " " .. wData .. " " .. dwData .. " " .. qwData .. " " .. sData .. " " .. nData .. " " .. llData .. " " .. fData .. " " .. tonumber(dbData))
    print("==================================")


    print(tonumber(123456789123456789))
    local num = 123456789123456789
    print(type(num) .. " " .. num)

    local num2 = tonumber(123456789123456789)
    print(type(num2) .. " " .. num2)
    local num3 = 1.23
    print(type(num3) .. " " .. num3)
    
end
