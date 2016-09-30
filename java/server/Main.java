import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

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
				String Tags=vntThreadIS.readLine();
                new GetGpsThreadFun(vntClient,Tags).start();
            }
            //vntClient.close();

        }catch (Exception e) {
            e.printStackTrace();
        }
    }
}

class GetGpsThreadFun extends Thread{
    private Socket vntThreadClient;
	private String ID;
    public GetGpsThreadFun(Socket vntThreadSocket,String Tags){
        vntThreadClient=vntThreadSocket;
		ID=Tags;
    }
    public void run(){
	System.out.println("Client ID="+ID+"\r\n");
	System.out.println(Thread.currentThread().getName()+"\r\n");
        try{
            BufferedReader vntThreadIS=new BufferedReader(new InputStreamReader(vntThreadClient.getInputStream()));
			BufferedWriter vntThreadWrite = new BufferedWriter(new OutputStreamWriter(vntThreadClient.getOutputStream()));
            while(true){
                String vntReceiveString=vntThreadIS.readLine();
                if (vntReceiveString!=null){
                    System.out.println(vntThreadClient.getInetAddress()+":"+vntThreadClient.getPort()+":"+vntReceiveString);
					vntThreadWrite.write("dddddddddddddddddddddd");
                }
                if("shutdown".equals(vntReceiveString)){
                    break;
                }    
            }
            vntThreadIS.close();
            vntThreadClient.close();
        }catch (Exception e) {
            e.printStackTrace();
        }
    }
}
