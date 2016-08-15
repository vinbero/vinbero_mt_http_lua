function service(client_id)
    print("client_id: " .. client_id)
    for k, v in pairs(client_table[client_id]) do
        print(k .. ": " .. v)
    end
end

