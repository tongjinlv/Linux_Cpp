package json;
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.Calendar;
import java.util.Date;

import org.json.JSONObject;
import org.json.JSONArray;
public class TestJSon {

    public static void main(String[] args) throws Exception{
			Salary say=new Salary();
		  String jsonContent = "{'hello':world,'abc':'xyz'}";
          JSONObject jsonObject = new JSONObject(jsonContent);
 		  Map<String, String> map1 = new HashMap<String, String>();
          map1.put("name", "Alexia");
          map1.put("sex", "female");
    	  map1.put("age", "23");
	      List<Map> list = new ArrayList<Map>();
					        list.add(map1);
	  JSONObject jo = new JSONObject();
	      jo.put("id","333");
		  JSONObject jbAddress=new JSONObject ();
		              jbAddress.append("street","dddd");
					              jbAddress.append("city","a");
								              jbAddress.append("zip","aaa");
			JSONArray array = new JSONArray();
			array.put(jo);
			JSONObject jobject = new JSONObject();
			jobject.put("totalCount",30);
			//       Employee p = new Employee(11,"Biju",new Salary(100));
   //     JSONObject obj = new JSONObject(p,false);
    //    System.out.println(obj.toString(1));
//
  //      Date d = Calendar.getInstance().getTime();
    //    System.out.println(p.getClass().getClassLoader());
    }
}
/*

{
"LMap": {
 "SAL-1": {"basicPay": 3011},
 "SAL-2": {"basicPay": 4012}
},
"age": 11,
"intge": 77,
"l": [
 {"basicPay": 301},
 {"basicPay": 401}
],
"name": "www",
"sal": {"basicPay": 100},
"salArray": [
 {"basicPay": 30},
 {"basicPay": 40}
],
"status": false
}

*/
