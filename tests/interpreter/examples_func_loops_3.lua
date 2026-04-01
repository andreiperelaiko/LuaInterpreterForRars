function walkvalues(t)
    total = 0
    for x in t do
        total = total + x
    end
    return total
end

function scaleandsum(t, k)
    for i = 1, 5, 1 do
        t[i] = t[i] * k
    end
    return walkvalues(t)
end

a = {1, 2, 3, 4, 5}
print(walkvalues(a))
print(scaleandsum(a, 2))
print(a[1], a[3], a[5])
