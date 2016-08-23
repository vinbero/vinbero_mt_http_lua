function get_content_length(client)
    return client["CONTENT_LENGTH"]
end


local body = ""
function on_body_chunk(client, body_chunk)
    print(body_chunk)
    body = body .. body_chunk
end

function on_body_finish(client)
    print(body)
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