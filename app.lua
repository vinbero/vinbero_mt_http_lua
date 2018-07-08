function onRequestFinish(client)
    local content = {}
    table.insert(content, "<h1>Request Info</h1>")
    for k, v in pairs(client.request) do
        table.insert(content, "<div>" .. k .. ": " .. tostring(v) .. "</div>")
    end
    table.insert(content, "<hr/>")
    for k, v in pairs(client.request.headers) do
        table.insert(content, "<div>" .. k .. ": " .. v .. "</div>")
    end
    content = table.concat(content)
    client.response:writeVersion(client.request.versionMajor, client.request.versionMinor)
    client.response:writeStatusCode(200)
    client.response:writeHeader("Content-Type", "html")
    client.response:writeHeader("Content-Length", #content)
    client.response:writeBody(content)
end
