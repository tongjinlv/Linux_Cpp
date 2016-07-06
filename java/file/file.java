import java.io.File;
import java.io.File;
import java.io.*;
public class File{
	public File(){
	}
	public static void main(String[] args){
		FileOutputStream out=null;
		FileOutputStream outStr=null;
		BufferedOutputStream Buff=null;
		FileWriter fw=null;
		int count=1000;
		try{
			out =new FileOutputStream(new File("1.txt"));
			long begin=System.currentTimeMillis();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
}
