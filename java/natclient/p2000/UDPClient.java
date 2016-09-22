
import java.io.*;
import java.net.*;

public class UDPClient {

    	public static void main(String args[]) throws IOException{
	int PORT = 12345;
	DatagramSocket dataSocket;
     	DatagramPacket dataPacket;
    	byte sendDataByte[];
	byte[] buf=new byte[100];
	DatagramPacket dp = new DatagramPacket(buf, buf.length);
     	String sendStr;

        dataSocket = new DatagramSocket(2000);
        sendDataByte = new byte[1024];
        sendStr = "UDP方式发送数据";
        sendDataByte = sendStr.getBytes();
        dataPacket = new DatagramPacket(sendDataByte, sendDataByte.length,InetAddress.getByName("139.129.34.143"), PORT);
        dataSocket.send(dataPacket);
	while(true)
	{
		dataSocket.receive(dp);
		String receiveStr = new String(buf, 0, dp.getLength());
        	String rcvd = "Client #" + ",rcvd from " + dp.getAddress() + ", " + dp.getPort() + ":" + receiveStr;
	        System.out.println(rcvd);
	}
    }
}
