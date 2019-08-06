import json
 
obj ="{'bigberg': [7600, {1: [['iPhone', 6300], ['Bike', 800], ['shirt', 300]]}]}"
print obj
a=json.loads(obj)
print a
