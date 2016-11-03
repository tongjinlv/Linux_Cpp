package Main;

import java.util.ArrayList;  
  
/** 
 * @author Administrator 管理客户端线程对象 
 */  
public class ManagerChatThread { 
	public static int count=0;
    private ArrayList<ChatThread> al = new ArrayList<ChatThread>();  
  
    public static ManagerChatThread instance = new ManagerChatThread();  
  
    private ManagerChatThread() {  
    };  
  
    public static ManagerChatThread getInstance() {  
        return instance;  
    }  
      
    public void add(ChatThread chat){  
        al.add(chat);  
    }  
    public void del(ChatThread chart){
    	al.remove(chart);
    }
    public void out(ChatThread chat, String line){  
        for (int i = 0; i < al.size(); i++) {  
            if(!al.get(i).equals(chat))//就不发给自己了
            {  
                al.get(i).out(chat.ID,line);  
            }  
        }  
    }  
}  