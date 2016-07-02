import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;

public class Client {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		if (args.length < 2 || (args.length > 3)) {
			throw new IllegalArgumentException(
			"Parameter(s):<Server> <Word>[<Port>]");
		}

		String server = args[0];// server name or IP address
		// Convert argument string to bytes using the default character encoding
		String sendData=args[1];
		System.out.println("Data to sent:"+ sendData);
		byte[] data = sendData.getBytes();
		int servPort = (args.length == 3) ? Integer.parseInt(args[2]) : 7;
		System.out.println("Server port:"+servPort);
		
		// create socket that is connected to server on specified port
		Socket socket = null;
		try {
			socket = new Socket(server, servPort);
			
			System.out.println("Connected to server ... sending echo string");

			InputStream in = socket.getInputStream();
			OutputStream out = socket.getOutputStream();
			out.write(data);

			// receive
			int totalBytesRcvd = 0;// total bytes received so far
			int bytesRcvd;// Bytes received in last read

			while (totalBytesRcvd < data.length) {
				bytesRcvd = in.read(data, totalBytesRcvd, data.length
						- totalBytesRcvd);
				if (bytesRcvd == -1) {
					throw new SocketException("Connection closed prematurely");
				}
				totalBytesRcvd += bytesRcvd;
			}
			System.out.println("Received:" + new String(data));

		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if (socket != null) {
					socket.close();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

	}

}

