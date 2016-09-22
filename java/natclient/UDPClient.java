
import java.io.*;
import java.net.*;

public class UDPClient {

    	public static void main(String args[]) throws IOException{
	int PORT = 12345;
	DatagramSocket dataSocket;
     	DatagramPacket dataPacket;
    	byte sendDataByte[];
     	String sendStr;

        dataSocket = new DatagramSocket(2000);
        sendDataByte = new byte[1024];
        sendStr = "UDP方式发送数据";
        sendDataByte = sendStr.getBytes();
        dataPacket = new DatagramPacket(sendDataByte, sendDataByte.length,InetAddress.getByName("139.129.34.143"), PORT);
        dataSocket.send(dataPacket);

    }
}
