host="api.douban.com"
port="80"
package_name = "fetchtag"
package_version = "0.1"
start="0"
count="10"

function generateRequest(field)
    local words = "&q="
    local k
    local v
    for k,v in pairs(field) do
        words = words..v.."+"
    end
    words = words:sub(1,-2)
    return string.format("GET /v2/music/search?%s&start=%d&count=%d HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: %s %s\r\nConnection: close\r\n\r\n", words:gsub("%s+", "+"), start, count, host, port, package_name, package_version)
end

function discardHeader(file)
    local f = assert(io.open(file, "r"))
    local content = f:read("*a")
    local pos = string.find(content, "\r\n\r\n",startPos)
    f:close()
    assert((pos ~= nil and pos+4<content:len()), "no empty line after header")
    return content:sub(pos+4)
end

function addAttrs(s) 
    if s and s:len() > 4 then
        info = info..s:sub(3,-3).."\n"
    else
        info = info.."".."\n"
    end
end

function addAttrsTracks(s)
    local count
    local v = string.gsub(s:sub(3,-3), "\\t", " ")
    v = string.gsub(v, "\\n", "\t")
    v = string.gsub(v, "\t[^0-9]+[^\t]*\t", "\t")
    v = string.gsub(v, "[0-9]+[%.%s]+", "")
    v, count = string.gsub(v, "\t+", "\t")
    info = info..tostring(count+1).."\t"..v.."\n"
end


function parseAttrs(s)
    local v1
    local v2
    v1,v2 = s:gsub("\"publisher\":(%b[])", addAttrs)
    if v2==0 then info=info.."".."\n" end

    v1,v2 = s:gsub("\"singer\":(%b[])", addAttrs)
    if v2==0 then info=info.."".."\n" end

    --pubdate has the form yyyy-mm-dd
    v1,v2 = s:gsub(".*\"pubdate\":%[\"(%d%d%d%d)[^\"]*\"%].*", "%1")
    if v2==0 then 
        info=info.."".."\n" 
    else 
        info = info..v1.."\n"
    end

    v1,v2 = s:gsub("\"title\":(%b[])", addAttrs)
    if v2==0 then info=info.."".."\n" end
    
    v1,v2 = s:gsub("\"tracks\":(%b[])", addAttrsTracks)
    if v2==0 then info=info.."".."\n" end
end

function addHref(s)
    info = info..string.format("music.douban.com/subject/%s", s).."\n"
end

function parseResult(s)
    local s = discardHeader(s)
    local results = {}
    table.insert(results, 0)
    local musicPos = s:find(".*\"musics\":%[") + 10
    --strlen of "musics":[ is 10
    assert(musicPos <= string.len(s), "failed at getting musics")
    local c = 0
    for musics in string.gmatch(s:sub(musicPos), "%b{}") do
        info = ""
        musics:gsub("\"attrs\":(%b{})", parseAttrs)
        musics:gsub("\"id\":\"(%d+)\"", addHref)
        table.insert(results, info)
        c = c + 1
    end
    results[1] = c
    return results
end
