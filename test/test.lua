function onRequestFinish(client)
    for k, v in pairs(client.request) do
        print(k, v)
    end
    for k, v in pairs(client.request.headers) do
        print(k, v)
    end

    client.response:writeVersion(client.request.versionMajor, versionMinor)
    client.response:writeStatusCode(200)
    client.response:writeStringHeader("Content-Length", 5)
    client.response:writeCrLf()
    client.response:writeBytes("HELLO")
end
