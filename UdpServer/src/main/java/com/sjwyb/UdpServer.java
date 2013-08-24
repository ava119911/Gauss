package com.sjwyb;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class UdpServer {
	private static final Log accessLogger = LogFactory.getLog("access");
	private static final Log consoleLogger = LogFactory.getLog("");
	private static final int PORT = 10033;
	private static final int MAX_PACKET_SIZE = 512;
	
	private DatagramSocket socket;
	private DatagramPacket packet;
	
	public UdpServer() throws SocketException {
		socket = new DatagramSocket(PORT);
		packet = new DatagramPacket(new byte[MAX_PACKET_SIZE], MAX_PACKET_SIZE);
	}
	
	public void start() throws Exception {
		while (true) {
			try {
				socket.receive(packet);
			} catch (IOException ex) {
				consoleLogger.error("", ex);
			}
			ArrayList<String> strings = decodeCString(packet);
			accessLogger.info(String.format(
					"%s\t%s", 
					packet.getAddress().getHostAddress(),
					joinString(strings, "\t")));
			packet.setLength(MAX_PACKET_SIZE);
		}
	}
	
	private String joinString(Collection<?> c, String sep) {
		StringBuilder sb = new StringBuilder();
		Iterator<?> iter = c.iterator();
		while (iter.hasNext()) {
			String s = iter.next().toString();
			sb.append(s);
			if (iter.hasNext())
				sb.append(sep);
		}
		return sb.toString();
	}
	
	private ArrayList<String> decodeCString(DatagramPacket packet) throws UnsupportedEncodingException {
		byte[] data = packet.getData();
		int i = packet.getOffset();
		int j = i;
		int end = packet.getOffset() + packet.getLength();
		ArrayList<String> strings = new ArrayList<String>();
		
		while (i < end) {
			if (data[i] == 0) {
				if ((i - j) > 0) {
					strings.add(new String(data, j, i - j, "ASCII"));
				}
				i++;
				j=i;
				continue;
			}
			i++;
		}
		return strings;
	}
	
	public static void main(String[] args) throws Exception {
		new UdpServer().start();
	}
}