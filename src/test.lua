function service(client)
    body = function()
        coroutine.yield("<h1>Hello World!</h1>")
        for k, v in pairs(client) do
            coroutine.yield("<h2>" .. k .. ": " .. v .. "</h2>")
        end
    end
    
    return 200, {["Content-Type"] = "text/html; charset=utf8"}, body
end