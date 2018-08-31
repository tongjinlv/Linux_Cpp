local f=assert(io.open("test.txt",'a'))  --a是末尾追加模式
f:write("Welcome\n这里增加一个换行\n")
f:write("Welocome\n测试内容")
f:write("\nceshineit")
f:close()
io.write("dddddddddddddd")
