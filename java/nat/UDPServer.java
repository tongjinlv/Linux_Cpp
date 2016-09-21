import java.io.*;
import java.net.*;
class UDPServer{
    public static void main(String[] args)throws IOException{
        DatagramSocket  server = new DatagramSocket(12345);
        byte[] recvBuf = new byte[100];
		System.out.println("start");
        DatagramPacket recvPacket=new DatagramPacket(recvBuf , recvBuf.length);
        server.receive(recvPacket);
        String recvStr = new String(recvPacket.getData() , 0 , recvPacket.getLength());
        System.out.println("Hello World!" + recvStr);
        int port = recvPacket.getPort();
        InetAddress addr = recvPacket.getAddress();
		System.out.println("ip:"+addr+" port:"+port);
        String sendStr = "Hello ! I'm Server";
        byte[] sendBuf;
        sendBuf = sendStr.getBytes();
        DatagramPacket sendPacket=new DatagramPacket(sendBuf , sendBuf.length , addr , port );
        server.send(sendPacket);
        server.close();
    }
}
