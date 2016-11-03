import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;

public class Main {
    /**
     * @param args
     */
    public static final int SERVER_PORT=12345;
    public static void main(String[] args) {
        System.out.println("Main.main()===TCP SERVER==============");
        try{
            ServerSocket vntServer= new ServerSocket(SERVER_PORT);
            System.out.println("Listening Port is "+vntServer.getLocalPort()+"...");
            while(true){
                Socket vntClient=vntServer.accept();
                System.out.println("have a client entering , the IP is "+vntClient.getInetAddress()+"...");
                System.out.println("have a client entering , the Port is "+vntClient.getPort()+"...");
				BufferedReader vntThreadIS=new BufferedReader(new InputStreamReader(vntClient.getInputStream()));
				String ID=vntThreadIS.readLine();
                new GetGpsThreadFun(vntClient,ID,0).start();
            }
            //vntClient.close();

        }catch (Exception e) {
            e.printStackTrace();
        }
    }
}

class GetGpsThreadFun extends Thread{
    private Socket LoginClient;
	private String LoginID;
	private int Mode;
	private int TimeOut=0;
    public GetGpsThreadFun(Socket vntThreadSocket,String ID,int Mode){
        LoginClient=vntThreadSocket;
        LoginID=ID;
    }
    public void run()
    {
    	try {
    		System.out.println(LoginClient.getInetAddress()+":"+LoginClient.getPort()+"\r\n");
    		System.out.println("Client ID="+LoginID+"\r\n");
    		System.out.println(Thread.currentThread().getName()+"\r\n");
			LoginClient.setSoTimeout(1000);
		} catch (SocketException e) {
			e.printStackTrace();
		}
    	switch(Mode)
    	{
    	case 0:Read();break;
    	case 1:Write();break;
    	}
    }
    public void Read(){
	while(true)
	{
		try
		{
			 BufferedReader vntThreadRead=new BufferedReader(new InputStreamReader(LoginClient.getInputStream()));
			 String vntReceiveString=vntThreadRead.readLine();
             if (vntReceiveString!=null){
                 System.out.println(LoginClient.getInetAddress()+":"+LoginClient.getPort()+":"+vntReceiveString);
                 TimeOut=0;
             }else {TimeOut++;System.out.println("Read Time Out:"+TimeOut);if(TimeOut>10)break;}
		}catch(Exception e)
		{
			System.out.println("Read Time Out:"+TimeOut);
			TimeOut++;
			if(TimeOut>10)break;
		}
	}
        System.out.println(Thread.currentThread().getName()+"Login Out\r\n");
    }
    public void Write(){
    	System.out.println("Client ID="+LoginID+"\r\n");
    	System.out.println(Thread.currentThread().getName()+"\r\n");
            try{
    			BufferedWriter vntThreadWrite = new BufferedWriter(new OutputStreamWriter(LoginClient.getOutputStream()));
                while(true){
                    String vntReceiveString="dfdfdas";
                    if (vntReceiveString!=null){
                        System.out.println(LoginClient.getInetAddress()+":"+LoginClient.getPort()+":"+vntReceiveString);
    					vntThreadWrite.write("dddddddddddddddddddddd");
                    }
                    if("shutdown".equals(vntReceiveString)){
                        break;
                    }    
                }
                vntThreadWrite.close();
                LoginClient.close();
            }catch (Exception e) {
                e.printStackTrace();
            }
            System.out.println("Login Out");
    }
}
