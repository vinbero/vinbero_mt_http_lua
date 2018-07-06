function onRequestFinish(client)
    local content = {}
    table.insert(content, "<h1>Request Info</h1>")
    for k, v in pairs(client.request) do
        if type(v) ~= 'string' and type(v) ~= 'number' then
            if type(v) == 'boolean' then
                if v == true then
                    v = 'true'
                else
                    v = 'false'
            end
            v = type(v)
        end
        table.insert(content, "<div>" .. k .. ": " .. v .. "</div>")
    end
    table.insert(content, "<hr/>")
    for k, v in pairs(client.request.headers) do
        table.insert(content, "<div>" .. k .. ": " .. v .. "</div>")
    end
    content = table.concat(content)
    client.response:writeVersion(client.request.versionMajor, client.request.versionMinor)
    client.response:writeStatusCode(200)
    client.response:writeStringHeader("Content-Type", "html")
    client.response:writeStringHeader("Content-Length", #content)
    client.response:writeCrLf()
    client.response:writeBytes(content)
end
