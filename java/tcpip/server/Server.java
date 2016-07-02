import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;

public class Server {

	//Size of receive buffer
	private static final int BUFSIZE=32;
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		//Test for correct # of args
		if(args.length!=1){
			throw new IllegalArgumentException("Parameter(s):<Port>");
		}
		int servPort = Integer.parseInt(args[0]);
		//Create a server socket to accept client connection request
		ServerSocket servSocket =null;
		int recvMsgSize=0;
		byte[] receivBuf=new byte[BUFSIZE];
		System.out.println("start listion"+servPort);	
		try {
			servSocket=new ServerSocket(servPort);
			while(true){
				Socket clientSocket=servSocket.accept();
				SocketAddress clientAddress = clientSocket.getRemoteSocketAddress();
				System.out.println("Handling client at "+ clientAddress);
				InputStream in =clientSocket.getInputStream();
				OutputStream out= clientSocket.getOutputStream();
				while((recvMsgSize=in.read(receivBuf))!=-1){
					String receivedData=new String(receivBuf.toString());
					System.out.println(receivedData);
				        System.out.println("dfda");
					out.write(receivBuf, 0, recvMsgSize);
				}
				clientSocket.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		

	}

}

