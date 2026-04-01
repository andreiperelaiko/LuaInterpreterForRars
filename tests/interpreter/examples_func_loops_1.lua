function sumto(n)
    s = 0
    for i = 1, n, 1 do
        s = s + i
    end
    return s
end

function multablerow(t, row, k)
    for j = 1, 4, 1 do
        t[row][j] = t[row][j] * k
    end
    return t[row][4]
end

t = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
}

print(sumto(10))
print(multablerow(t, 2, 3))
print(t[2][1], t[2][2], t[2][3], t[2][4])
