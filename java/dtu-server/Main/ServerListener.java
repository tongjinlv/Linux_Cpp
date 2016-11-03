package Main;


import java.io.IOException;  
import java.net.ServerSocket;  
import java.net.Socket;  
  
public class ServerListener extends Thread {  
    public void run() {  
        super.run();  
        System.out.println("主线程等待客户端连接");  
        try {  
            ServerSocket serverSocket = new ServerSocket(60000);  
            while (true) {  
                ManagerChatThread.count++;
                Socket socket = serverSocket.accept();  
                System.out.println("有客户端"+ManagerChatThread.count+"上线["+socket.getRemoteSocketAddress()+"]");  
                ChatThread chat = new ChatThread(socket, ManagerChatThread.count);  
                chat.start();  
                ManagerChatThread.getInstance().add(chat);  
  
            }  
  
        } catch (IOException e1) {  
            e1.printStackTrace();  
        }  
  
    }  
}  