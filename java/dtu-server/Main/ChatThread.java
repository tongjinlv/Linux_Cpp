package Main;

import java.io.BufferedReader;  
import java.io.BufferedWriter;  
import java.io.IOException;  
import java.io.InputStreamReader;  
import java.io.OutputStreamWriter;  
import java.io.UnsupportedEncodingException;  
import java.net.Socket;  
  
public class ChatThread extends Thread {  
  
    int count;  
    String ID;
    Socket socket;  
    StringBuffer sb;  
    public ChatThread(Socket socket, int count) {  
        // TODO Auto-generated constructor stub  
        this.socket = socket;  
        this.count = count;  
    }  
  
    /** 
     * 向客户端发送消息 
     *  
     * @param msg 
     */  
    public void out(String loginbag,String msg) {  
        try {  
            if(this.ID.equals(loginbag))socket.getOutputStream().write((msg+"\n").getBytes("utf-8")); 
            //"xxxxx.bbbb.xxxx"
        } catch (IOException e) {  
            
            e.printStackTrace();  
            System.out.println("发送失败断开"+count+"["+this.socket.getRemoteSocketAddress()+"]");
        }  
    }  
  
    @Override  
    public void run() {   
        super.run();  
        boolean firstlogin=true;
        BufferedReader br = null;  
        try {  
            br = new BufferedReader(new InputStreamReader(socket.getInputStream(), "utf-8"));  
            String line = null;  
            while (((line = br.readLine()) != null)) {  
            	if(firstlogin)
            	{
            		ID=line;
            		System.out.println("客户端登陆"+"IP:["+this.socket.getRemoteSocketAddress()+"]ID:["+ID+"]"); 
            		firstlogin=false;
            	}else
            	{
            		System.out.println("客户端"+count+"["+this.socket.getRemoteSocketAddress()+"]：" + line);  
            		ManagerChatThread.getInstance().out(this, line);
            	}
            }  
        } catch (UnsupportedEncodingException e) {  
            e.printStackTrace();  
        } catch (IOException e) {  
            e.printStackTrace();  
        }  
        ManagerChatThread.count--;
        ManagerChatThread.getInstance().del(this);
        System.out.println("读取失败断开"+count+"["+this.socket.getRemoteSocketAddress()+"]");
    }  
}  
