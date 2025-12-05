function on_update(dt, time)
    -- Simple logic: Rotate objects around origin
    local r = 0.6
    local speed = 2.0
    
    -- Left Cube: Rotate CW
    local x1 = math.sin(time * speed) * r - 0.5
    local z1 = math.cos(time * speed) * r
    
    -- Right Cube: Rotate CCW
    local x2 = math.sin(-time * speed) * r + 0.5
    local z2 = math.cos(-time * speed) * r
    
    -- Dispatch commands to Sim via IR
    if EID_LEFT then
        dispatch({
            op = "set_pos",
            id = EID_LEFT,
            x = x1, y = 0.0, z = z1
        })
    end
    
    if EID_RIGHT then
        dispatch({
            op = "set_pos",
            id = EID_RIGHT,
            x = x2, y = 0.0, z = z2
        })
    end
end
