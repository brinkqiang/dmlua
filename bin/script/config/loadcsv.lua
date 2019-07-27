module (..., package.seeall)

function dump (t)
    print(t)
    for k,v in pairs(t) do
        print(k,v, type(v))
    end
end

function main()
    local w = csv.writer('config/test.csv')
    w:write {'Name is \"that\"','Product','Date','Age'}
    
    w:write {'steve', 'bonzo \"dog\" catcher','10/10/09',3}
    w:write {'bonzo','dog,cat','23/04/08',10}
    w:write {'john','CowCatcher','20/10/09',4}
    w:close()

    local r,c = csv.reader('config/test.csv',true,true)
    if c then
        print 'headers'
        dump(c)
    end

    --[[
    row = r:read()
    while row do
    dump(row)
    row = r:read()
    end
    --]]

    for row in r:rows() do
        dump(row)  -- can use row:copy() to get unique tables
    end
end

