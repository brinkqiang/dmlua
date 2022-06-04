-- lua script
module(..., package.seeall)

function clone(object)
    local lookup_table = {}
    local function copyObj(object)
        if type(object) ~= 'table' then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end

        local new_table = {}
        lookup_table[object] = new_table
        for key, value in pairs(object) do
            new_table[copyObj(key)] = copyObj(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return copyObj(object)
end

function clone(object, deep)
    local copy = {}

    for k, v in pairs(object) do
        if deep and type(v) == 'table' then v = clone(v, deep) end
        copy[k] = v
    end

    return setmetatable(copy, getmetatable(object))
end

function main()
    local map = {id = 1, count = 2}
    local map2 = clone(map)
    print(map)
    print(map2)

    local map3 = {}
    clone(map, map3)
    print(map)
    print(map3)
end
