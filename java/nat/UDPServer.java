import java.io.*;
import java.net.*;
import java.util.*;


class UDPServer{
	public static IpPort[] ipool=new IpPort[10];
    public static List<String> list =new ArrayList<String>();   
    public static List<IpPort> iplist =new ArrayList<IpPort>();   
	public static void main(String[] args)throws IOException{
        DatagramSocket  server = new DatagramSocket(12345);
        byte[] recvBuf = new byte[100];
		System.out.println("start");
        DatagramPacket recvPacket=new DatagramPacket(recvBuf , recvBuf.length);
        server.receive(recvPacket);
        String recvStr = new String(recvPacket.getData() , 0 , recvPacket.getLength());
		System.out.println(recvStr);
	//	IpPort ipt=new IpPort();
	//	ipt.IpAddr=InetAddress.getByName("172.25.67.65");
	//	iplist.add(ipt);
		
		ipool[0]=new IpPort(recvPacket.getAddress(),recvPacket.getPort());
		System.out.println("ip:"+ipool[0].IpAddr+" port:"+ipool[0].Port);
        String sendStr = "Hello ! I'm Server";
        byte[] sendBuf;
        sendBuf = sendStr.getBytes();
        DatagramPacket sendPacket=new DatagramPacket(sendBuf , sendBuf.length ,ipool[0].IpAddr,ipool[0].Port);
        server.send(sendPacket);
        server.close();
    }
}


class IpPort{
	     InetAddress IpAddr;//用户公网IP
	     int Port;//用户公网端口
		 public String ID;//网络通信唯一地址
		 public String Key;//用户登录密钥
	public IpPort(InetAddress IpAddr,int Port){
		this.IpAddr=IpAddr;
		this.Port=Port;
	}
}

