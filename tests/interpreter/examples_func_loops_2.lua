function dot(a, b)
    s = 0
    for i = 1, 4, 1 do
        s = s + a[i] * b[i]
    end
    return s
end

function buildrow(base)
    r = {}
    for i = 1, 4, 1 do
        r[i] = base + i
    end
    return r
end

va = buildrow(0)
vb = buildrow(10)

print(dot(va, vb))
print(va[1], va[4], vb[1], vb[4])
