svn st | grep "? \+" | sed "s/? \+//" | xargs svn add  
# 删除所有本地缺失的文件  
svn st | grep "! \+" | sed "s/! \+//" | xargs svn delete  
# 提交  
svn ci -m '' 
