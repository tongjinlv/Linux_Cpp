import java.io.*;
import java.net.*;
import java.util.*;

class UDPServer{
	public static IpPort[] ipool=new IpPort[10];
	public static List<String> list =new ArrayList<String>();   
    public static List<IpPort> iplist =new ArrayList<IpPort>();   
	public static ClientInfor clientinfor=new ClientInfor();
	public static void main(String[] args)throws IOException{
		DatagramSocket  server = new DatagramSocket(12345);
		while(true)
		{
        byte[] recvBuf = new byte[100];
		System.out.println("start");
        DatagramPacket recvPacket=new DatagramPacket(recvBuf , recvBuf.length);
        server.receive(recvPacket);
		IpPort iport=new IpPort(recvPacket.getAddress(),recvPacket.getPort());
        String recvStr = new String(recvPacket.getData() , 0 , recvPacket.getLength());
		System.out.println("ipaddr:"+iport.IpAddr+" port:"+iport.Port);
		System.out.println(recvStr);
		byte[] sendBuf;
        String sendStr="你好";
		sendBuf = sendStr.getBytes();
        DatagramPacket sendPacket=new DatagramPacket(sendBuf , sendBuf.length ,iport.IpAddr,iport.Port);
        server.send(sendPacket);
		}
        //server.close();
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
class ClientInfor{
		public List<IpPort> iplist =new ArrayList<IpPort>();
		public void add(IpPort ip){
			iplist.add(ip);
		}
		public String getJson(String name){
			String jstr="{\""+name+":[";
			for(int i=0;i<iplist.size();i++)
			{
				IpPort iport=iplist.get(i);
				jstr+="{";
				jstr+="\"";
				jstr+=iport.IpAddr.toString();
				jstr+="\"";
				jstr+=":";
				jstr+=iport.Port;
				jstr+="},";
			}
			jstr+="]}";
			return jstr;
		}

}
