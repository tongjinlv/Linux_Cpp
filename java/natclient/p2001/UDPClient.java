
import java.io.*;
import java.net.*;

public class UDPClient {

    	public static void main(String args[]) throws IOException{
	int PORT = 12345;
	DatagramSocket dataSocket;
	DatagramSocket dataSocket1;
     	DatagramPacket dataPacket;
     	DatagramPacket dataPacket1;
    	byte sendDataByte[];
     	String sendStr;

        dataSocket = new DatagramSocket(2001);
        dataSocket1 = new DatagramSocket(2002);
        sendDataByte = new byte[1024];
        sendStr = "aaaaaaa";
        sendDataByte = sendStr.getBytes();
        dataPacket = new DatagramPacket(sendDataByte, sendDataByte.length,InetAddress.getByName("139.129.34.143"), PORT);
 	dataSocket.send(dataPacket);
	sendStr="ddddddddddddddd";
	sendDataByte=sendStr.getBytes();
	dataPacket1 = new DatagramPacket(sendDataByte, sendDataByte.length,InetAddress.getByName("119.136.112.123"),2000 );
	dataSocket1.send(dataPacket1);	
    }
}
