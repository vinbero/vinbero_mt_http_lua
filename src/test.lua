function on_request_start(client)
    client["BODY"] = nil
end

function on_headers_finish(client)
    if tonumber(client["CONTENT_LENGTH"]) > 0 then
        client["BODY"] = ""
    end
end

function get_content_length(client)
    return client["CONTENT_LENGTH"]
end

function on_body_chunk(client, body_chunk)
    client["BODY"] = client["BODY"] .. body_chunk
end

function on_body_finish(client)
    print(client["BODY"])
end


function on_request_finish(client)
    body = function()
        coroutine.yield("<h1>Hello World!</h1>")
        for k, v in pairs(client) do
            coroutine.yield("<h2>" .. k .. ": " .. v .. "</h2>")
        end
    end
    
    return 200, {["Content-Type"] = "text/html; charset=utf8"}, body
end